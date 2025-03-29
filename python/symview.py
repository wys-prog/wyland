import os
import argparse

# Set up argument parser
parser = argparse.ArgumentParser(description="Symbol viewer for binary files.")
parser.add_argument("file", help="Path to the binary file to analyze.")
parser.add_argument("origin", help="Origin address (e.g., 0x1000).")
parser.add_argument("-o", "--output", help="Path to the output file (optional).", default=None)
args = parser.parse_args()

# Parse the origin
try:
    if args.origin.lower().startswith('0x'):
        org = int(args.origin, 16)
    elif args.origin.lower().startswith('0o'):
        org = int(args.origin, 8)
    else:
        org = int(args.origin)
except ValueError:
    print("Invalid origin format. Please enter a valid number (e.g., 0x1000).")
    exit(1)

# Check if the file exists
if not os.path.isfile(args.file):
    print(f"File '{args.file}' not found.")
    exit(1)

# Open the output file if specified
output = open(args.output, 'w') if args.output else None

# Read and display the file content
with open(args.file, 'rb') as usrin:
    i = 0
    while True:
        byte = usrin.read(1)  # Read one byte at a time
        if not byte:  # End of file
            break

        byte_value = byte[0]  # Convert byte to integer
        if byte_value == 0xFE:  # Check for potential label or string
            next_byte = usrin.read(1)
            if not next_byte:  # End of file
                line = f'0x{org + i:08X}: 0xFE\n'
                if output:
                    output.write(line)
                else:
                    print(line, end='')
                break

            next_value = next_byte[0]
            if next_value == 0x00:  # Label detected
                line = f'0x{org + i:08X}: 0xFE 0x00 (label)\n'
                if output:
                    output.write(line)
                else:
                    print(line, end='')
                i += 2
                continue
            else:  # Possible string
                string_bytes = bytearray()
                string_bytes.append(next_value)
                while True:
                    char = usrin.read(1)
                    if not char or char[0] == 0x00:  # End of string
                        break
                    string_bytes.append(char[0])
                try:
                    decoded_string = string_bytes.decode('utf-8')
                    line = f'0x{org + i:08X}: 0xFE ("{decoded_string}", {len(decoded_string)})\n'
                except UnicodeDecodeError:
                    line = f'0x{org + i:08X}: 0xFE (invalid string)\n'
                if output:
                    output.write(line)
                else:
                    print(line, end='')
                i += len(string_bytes) + 2
                continue

        # Default case: print the byte
        line = f'0x{org + i:08X}: 0x{byte.hex().upper()}\n'
        if output:
            output.write(line)
        else:
            print(line, end='')
        i += 1

# Close the output file if it was opened
if output:
    output.close()