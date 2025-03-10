import re
import struct
import argparse

# Global symbol table, address counter, defines, and macros
symbol_table = {}
current_address = 0
defines = {}
macros = {}
verbose = False  # Global verbose flag

# Regex to match optional label, directive, and values
pattern = re.compile(r'^(?:(?P<label>[\w.:@]+):)?\s*(?P<directive>\w+)?\s*(?P<values>.*)$')
define_pattern = re.compile(r'^\s*%define\s+(?P<name>\w+)\s+(?P<value>.+)$')
macro_pattern = re.compile(r'^\s*%macro\s+(?P<name>\w+)\s*(?P<params>.*)$')
endmacro_pattern = re.compile(r'^\s*%endmacro\s*$')

def log(message):
    """Print message if verbose mode is enabled."""
    if verbose:
        print(message)

def parse_line(line):
    """Parse an assembly line into (label, directive, values)."""
    line = line.split(';')[0].strip()  # Remove comments
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


def process_macro(lines, index):
    """Process a macro definition with the correct syntax."""
    match = macro_pattern.match(lines[index])
    if not match:
        return index

    name = match.group('name')
    params = [p.strip() for p in match.group('params').split(',') if p.strip()]
    macro_body = []

    index += 1
    while index < len(lines):
        line = lines[index].strip()
        if endmacro_pattern.match(line):
            break
        macro_body.append(line)
        index += 1

    macros[name] = (params, macro_body)
    return index


def expand_macro(line):
    """Expand a macro call by replacing parameters properly."""
    parts = line.split()
    name = parts[0]
    
    if name not in macros:
        return [line]  # If not a macro, return as is.

    params, macro_body = macros[name]
    args = [p.strip() for p in ' '.join(parts[1:]).split(',')]

    if len(args) != len(params):
        raise ValueError(f"Macro {name} expects {len(params)} arguments, got {len(args)}")

    # Create a dictionary of replacements
    replacements = {f"${param}": arg for param, arg in zip(params, args)}

    expanded_lines = []
    for body_line in macro_body:
        for param, arg in replacements.items():
            body_line = body_line.replace(param, arg)  # Replace variables correctly
        expanded_lines.append(body_line)

    return expanded_lines  # Now correctly expanded!



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

def assemble(source_code):
    """Assemble source code into binary."""
    global current_address, symbol_table
    binary_data = bytearray()
    lines = source_code.splitlines()
    
    # First pass: process macros
    index = 0
    while index < len(lines):
        line = lines[index].strip()
        if line.startswith('%macro'):
            index = process_macro(lines, index)
        index += 1

    # Second pass: assemble code
    label = None
    index = 0
    while index < len(lines):
        line = lines[index].strip()
        if not line or line.startswith('%'):
            index += 1
            continue

        # Expand macros
        expanded_lines = expand_macro(line)
        for expanded_line in expanded_lines:
            if expanded_line.startswith('%define'):
                process_define(expanded_line)
                continue

            parsed = parse_line(expanded_line)
            if parsed is None:
                continue

            current_label, directive, values = parsed
            if current_label:
                label = current_label
                symbol_table[label] = current_address
                log(f"{current_address:#018X}: {label.ljust(10)}")

            if directive == 'org':
                try:
                    new_address = int(values, 0)
                    log(f"{new_address:#018X}: New origin")
                    current_address = new_address
                except ValueError:
                    print(f"Error: Invalid address for ORG: {values}")
                continue

            if directive in ['db', 'dw', 'dd', 'dq']:
                data = process_directive(directive, values)
                binary_data.extend(data)
            elif directive in symbol_table:
                binary_data.extend(struct.pack('>Q', symbol_table.get(directive)))
                current_address += 8
            elif directive == "$":
                continue

            # Reset label after processing the directive
            if label and directive:
                label = None

        index += 1

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

    with open(args.source_file, 'r') as usrin:
        with open(args.output, 'wb') as out:
            for line in usrin:
                try:
                    binary = assemble(line)
                    out.write(binary)
                except Exception as e:
                    print(e, "\nline: ")
                    return -1
            
    print(f"Binary saved to {args.output}")
    

if __name__ == '__main__':
    main()


