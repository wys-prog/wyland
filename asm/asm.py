import re
import struct
import argparse

# Global symbol table, address counter, and defines
symbol_table = {}
current_address = 0
defines = {
    "nop": "db 0", 
    "lea": "db 1,",
    "load": "db 2",
    "store": "db 3",
    "mov": "db 4,",
    "add": "db 5,",
    "sub": "db 6,",
    "mul": "db 7,",
    "div": "db 8,",
    "mod": "db 9,",
    "jmp": "db 10",
    "je": "db 11",
    "jne": "db 12",
    "jl": "db 13",
    "jg": "db 14",
    "jle": "db 15",
    "jge": "db 16",
    "cmp": "db 17,",
    "int": "db 19",
    "loadat": "db 20",
    "ret": "db 21",
    "movad": "db 22",
    "swritec": "db 19, 0", 
    "swritecerr": "db 19, 1", 
    "sreadc": "db 19, 2", 
    "scsystem": "db 19, 3", 
    "sldlib": "db 19, 4", 
    "sstartt": "db 19, 5", 
    "spseg": "db 19, 6", 
    "sreads": "db 19, 7", 
    "sldlcfun": "db 19, 8", 
    "suldlib": "db 19, 9", 
    "scfun": "db 19, 10", 
    "CODE_SEGMENT_SIZE": "419430400",
    "HARDWARE_SEGMENT_SIZE": "104857600", 
    "SYSTEM_SEGMENT_SIZE": "12582912", 
    "CODE_SEGMENT_START": "0", 
    "HARDWARE_SEGMENT_START": "419430400", 
    "SYSTEM_SEGMENT_START": "524288000", 
    "KEYBOARD_SEGMENT_START": "419430400",
    "KEYBOARD_SEGMENT_END": "421527552",
}
verbose = False  # Global verbose flag

# Regex to match optional label, directive, and values
pattern = re.compile(r'^(?:(?P<label>[\w.:@]+):)?\s*(?P<directive>\w+)?\s*(?P<values>.*)$')
define_pattern = re.compile(r'^\s*%define\s+(?P<name>\w+)\s+(?P<value>.+)$')
ex_pattern = re.compile(r'%ex\s+(\w+)\(([^)]+)\)\s+(.+)')

def log(message):
    """Print message if verbose mode is enabled."""
    if verbose:
        print(message)

def parse_line(line):
    """Parse an assembly line into (label, directive, values)."""
    line = line.split('#')[0].strip()  # Remove comments (using # instead of ;)
    if not line:
        return None
    match = pattern.match(line)
    if match:
        label = match.group('label')
        directive = match.group('directive').lower() if match.group('directive') else None
        values = match.group('values').strip() if match.group('values') else None
        return label, directive, values
    return None

def process_define(line):
    """Process %define and store in dictionary."""
    match = define_pattern.match(line)
    if match:
        name = match.group('name')
        value = match.group('value').strip()
        
        defines[name] = value
        

def process_directive(directive, values):
    """Process data directives (db, dw, dd, dq) and handle strings, chars, and ints."""
    global current_address
    data = bytearray()
    values = resolve_defines(values)
    
    # Split values while preserving quoted strings
    # parts = re.findall(r'"[^"]*"|\'[^\']*\'|\S+', values)
    parts = values.split(',')
    
    for part in parts:
        if part.startswith("'") and part.endswith("'") and len(part) == 3:
            # Handle char
            char_data = ord(part[1])
            if directive == 'db':
                data.extend(struct.pack('>B', char_data))
                current_address += 1
            elif directive == 'dw':
                data.extend(struct.pack('>H', char_data))
                current_address += 2
            elif directive == 'dd':
                data.extend(struct.pack('>I', char_data))
                current_address += 4
            elif directive == 'dq':
                data.extend(struct.pack('>Q', char_data))
                current_address += 8
        else:
            # Handle int
            num = int(part, 0)
            if directive == 'db':
                data.extend(struct.pack('>B', num & 0xFF))
                current_address += 1
            elif directive == 'dw':
                data.extend(struct.pack('>H', num & 0xFFFF))
                current_address += 2
            elif directive == 'dd':
                data.extend(struct.pack('>I', num & 0xFFFFFFFF))
                current_address += 4
            elif directive == 'dq':
                data.extend(struct.pack('>Q', num))
                current_address += 8
    return data

def resolve_defines(values):
    previous_values = None
    while previous_values != values:
        previous_values = values
        for name, value in defines.items():
            values = re.sub(rf'\b{name}\b', str(value), values)
    return values


def assemble(source_code):
    """Assemble source code into binary."""
    global current_address, symbol_table
    binary_data = bytearray()
    lines = re.split(r'[&\n]', source_code)

    # First pass: process defines and macros
    for line in lines:
        line = line.strip()
        if line.startswith('%define'):
            process_define(line)
    
    # Second pass: assemble code
    for line in lines:
        line = line.strip()
        if not line or line.startswith('%'):
            continue

        parsed = parse_line(resolve_defines(line))
        if parsed is None:
            continue

        current_label, directive, values = parsed
        if current_label:
            symbol_table[current_label] = current_address
            log(f"{current_address:#018X}: {current_label.ljust(10)}")

        if directive in [';', '//', '#']: continue

        if directive == 'org':
            try:
                current_address = int(values, 0)
                log(f"{current_address:#018X}: New origin")
            except ValueError:
                print(f"Error: Invalid address for ORG: {values}")
        elif directive in ['db', 'dw', 'dd', 'dq']:
            values = resolve_defines(values)  # Replace macros in values
            unlabeledValues = values.split(' ')
            labeled = []

            for unlabeled in unlabeledValues: 
                if unlabeled in symbol_table:
                    labeled.append(str(symbol_table[unlabeled]))
                else: labeled.append(unlabeled)

            data = process_directive(directive, ''.join(labeled))
            binary_data.extend(data)
        elif directive in symbol_table:
            binary_data.extend(struct.pack('>Q', symbol_table[directive]))
            current_address += 8        

    return binary_data

def parse_args():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description="Simple assembler to binary")
    parser.add_argument('source_file', type=str, help="Path to input source file")
    parser.add_argument('-o', '--output', type=str, default='output.bin', help="Output binary file")
    parser.add_argument('-v', '--verbose', action='store_true', help="Enable verbose mode")
    return parser.parse_args()

def main():
    """Main function to assemble the file."""
    global verbose
    args = parse_args()
    verbose = args.verbose
    
    count = 0
    
    with open(args.source_file, 'r') as usrin:
        with open(args.output, 'wb') as out:
            for line in usrin:
                try:
                    out.write(assemble(line))
                except Exception as e:
                    print(e, f'\nline: [{line.strip()}]:{count}:{args.source_file}')
                    return -1
                
                count += 1
                    
    print(f"Binary saved to {args.output}")

if __name__ == '__main__':
    main()
