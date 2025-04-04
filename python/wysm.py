import os
import argparse
import subprocess
from datetime import datetime

def is_up_to_date(source_file, output_file):
    """
    Check if the output file is up-to-date with the source file.
    """
    if not os.path.exists(output_file):
        return False
    source_mtime = os.path.getmtime(source_file)
    output_mtime = os.path.getmtime(output_file)
    return source_mtime <= output_mtime

def compile_file(source_file, output_file, skip_if_up_to_date):
    """
    Compile a single file.
    """
    if skip_if_up_to_date and is_up_to_date(source_file, output_file):
        print(f"Skipping {source_file}: already up-to-date.")
        return True

    try:
        print(f"Compiling {source_file} -> {output_file}...")
        # Replace this with the actual compilation command
        subprocess.run(["python", "/Users/wys/Documents/wyland/python/asm.py", source_file, output_file], check=True)
        print(f"Compiled {source_file} successfully.")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error compiling {source_file}: {e}")
        return False

def compile_chain(files, skip_if_up_to_date):
    """
    Compile multiple files in a chain.
    """
    for i, source_file in enumerate(files):
        output_file = f"{os.path.splitext(source_file)[0]}.bin"
        if not compile_file(source_file, output_file, skip_if_up_to_date):
            print(f"Stopping chain compilation due to error in {source_file}.")
            return False
    return True

def main():
    parser = argparse.ArgumentParser(description="Multi-file compiler with options.")
    parser.add_argument("files", nargs="+", help="Source files to compile.")
    parser.add_argument("-o", "--output", help="Output file for a single source file.")
    parser.add_argument("-s", "--skip", action="store_true", help="Skip compilation if output is up-to-date.")
    parser.add_argument("-c", "--chain", action="store_true", help="Compile files in a chain.")
    args = parser.parse_args()

    if args.chain:
        # Compile files in a chain
        if not compile_chain(args.files, args.skip):
            print("Chain compilation failed.")
            exit(1)
    else:
        # Compile individual files
        for source_file in args.files:
            output_file = args.output if args.output else f"{os.path.splitext(source_file)[0]}.bin"
            if not compile_file(source_file, output_file, args.skip):
                print(f"Compilation failed for {source_file}.")
                exit(1)

if __name__ == "__main__":
    main()
