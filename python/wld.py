import sys
import os

def include_files(input_file, output_file):
    try:
        with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
            for line in infile:
                stripped_line = line.strip()
                if stripped_line.startswith('#include'):
                    # Extract the file path from the include directive
                    start = stripped_line.find('"') + 1
                    end = stripped_line.rfind('"')
                    if start == 0 or end == -1:
                        print(f"Error: Malformed include directive: {line.strip()}")
                        continue
                    
                    include_path = stripped_line[start:end]
                    
                    # Check if the file exists
                    if not os.path.exists(include_path):
                        print(f"Error: Included file not found: {include_path}")
                        continue
                    
                    # Read and write the content of the included file
                    with open(include_path, 'r') as included_file:
                        outfile.write(f"# Start of included file: {include_path}\n")
                        outfile.writelines(included_file.readlines())
                        outfile.write(f"# End of included file: {include_path}\n")
                else:
                    # Write the line as is
                    outfile.write(line)
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python include_files.py <input_file> <output_file>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    include_files(input_file, output_file)
