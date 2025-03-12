import re
import struct
import argparse

# Global symbol table, address counter, and defines
symbol_table = {}
current_address = 0
defines = {}
verbose = False  # Global verbose flag

# Regex to match optional label, directive, and values
pattern = re.compile(r'^(?:(?P<label>[\w.:@]+):)?\s*(?P<directive>\w+)?\s*(?P<values>.*)$')
define_pattern = re.compile(r'^\s*%define\s+(?P<name>\w+)\s+(?P<value>.+)$')
macro_pattern = re.compile(r'%define\s+(\w+)\(([^)]+)\)\s+(.+)')

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
        try:
            value = int(value, 0)  # Convert to int (supports hex, decimal)
            defines[name] = value
            log(f"{name} = {value:#X}")
            return True
        except ValueError:
            print(f"Error: Invalid value for %define {name}: {value}")
    return False

def process_macro(line):
    """Process macros with parameters (e.g. %define Something(X, Y) dq X ; db Y)."""
    match = macro_pattern.match(line)
    if match:
        macro_name = match.group(1)
        params = match.group(2)
        body = match.group(3)
        defines[macro_name] = (params, body)
        log(f"Macro {macro_name} defined with parameters: {params}")
        return True
    return False

def process_directive(directive, values):
    """Process data directives (db, dw, dd, dq) and handle strings, chars, and ints."""
    global current_address
    data = bytearray()
    values = resolve_defines(values)
    
    # Split values while preserving quoted strings
    parts = re.findall(r'"[^"]*"|\'[^\']*\'|\S+', values)
    
    for part in parts:
        if part.startswith('"') and part.endswith('"'):
            # Handle string
            string_data = part.strip('"').encode('utf-8')
            if directive == 'db':
                data.extend(string_data)
                current_address += len(string_data)
            elif directive == 'dw':
                for char in string_data:
                    data.extend(struct.pack('>H', char))
                    current_address += 2
            elif directive == 'dd':
                for char in string_data:
                    data.extend(struct.pack('>I', char))
                    current_address += 4
            elif directive == 'dq':
                for char in string_data:
                    data.extend(struct.pack('>Q', char))
                    current_address += 8
        elif part.startswith("'") and part.endswith("'") and len(part) == 3:
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
    """Replace %define constants in values."""
    for name, value in defines.items():
        values = values.replace(name, str(value))
    return values

def replace_macros(values):
    """Replace macros with parameters in the given values."""
    for macro_name, (params, body) in defines.items():
        # Replace the macro parameters in the body with the values
        macro_values = {param.strip(): f'{{{param.strip()}}}' for param in params.split(',')}
        for param, replacement in macro_values.items():
            body = body.replace(param, replacement)
        values = values.replace(f'%{macro_name}({params})', body)
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
            if not process_define(line) and not process_macro(line):
                print(f"Error: Failed to process %define: {line}")
    
    # Second pass: assemble code
    for line in lines:
        line = line.strip()
        if not line or line.startswith('%'):
            continue

        parsed = parse_line(line)
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
            values = replace_macros(values)  # Replace macros in values
            data = process_directive(directive, values)
            binary_data.extend(data)
        elif directive in symbol_table:
            binary_data.extend(struct.pack('>Q', symbol_table[directive]))
            current_address += 8
        elif directive == "$":
            continue

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
