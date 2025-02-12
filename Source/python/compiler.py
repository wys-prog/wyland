import re
import struct
import sys

def parse_macros(file_path):
  macros = {}
  pattern = re.compile(r"#define\s+(\w+)\((.*?)\)\s+(.+)")

  with open(file_path, "r") as file:
    for line in file:
      match = pattern.match(line)
      if match:
        name, params, body = match.groups()
        param_list = [p.strip() for p in params.split(',')]
        macros[name] = (param_list, body)
  
  return macros

def expand_macros(input_file, output_file, macros):
  macro_call_pattern = re.compile(r"(\w+)\(([^)]*)\)")

  def replace_macro(match):
    name = match.group(1)
    if name not in macros:
      return match.group(0)
    
    args = [arg.strip() for arg in match.group(2).split(',')]
    param_list, body = macros[name]

    if len(args) != len(param_list):
      return match.group(0)
    
    expanded_body = body
    for param, value in zip(param_list, args):
      expanded_body = expanded_body.replace(param, value)
    
    return expanded_body

  with open(input_file, "r") as infile, open(output_file, "w") as outfile:
    for line in infile:
      if not line.startswith("#define"):
        expanded_line = macro_call_pattern.sub(replace_macro, line)
        outfile.write(expanded_line)

def parse_assembly(input_file, output_file):
  pattern = re.compile(r"(\w+)\((.*?)\)")

  with open(input_file, "r") as infile, open(output_file, "wb") as outfile:
    for line in infile:
      binary_data = bytearray()
      match = pattern.match(line.strip())

      if not match:
        continue
      
      directive, args = match.groups()
      args = args.split(',')

      if directive == "db":
        binary_data.append(int(args[0], 0))
      elif directive == "dw":
        binary_data.extend(struct.pack('>H', int(args[0], 0)))
      elif directive == "dd":
        binary_data.extend(struct.pack('>I', int(args[0], 0)))
      elif directive == "dq":
        binary_data.extend(struct.pack('>Q', int(args[0], 0)))
      elif directive == "da":
        binary_data.extend(int(x.strip(), 0) for x in args)
      elif directive == "times":
        times = int(args[0], 0)
        nested_instruction = f"{args[1].strip()})"

        for _ in range(times):
          binary_data.extend(parse_assembly_line(nested_instruction))

      outfile.write(binary_data)

def parse_assembly_line(line):
  binary_data = bytearray()
  pattern = re.compile(r"(\w+)\((.*?)\)")
  match = pattern.match(line.strip())

  if not match:
    return binary_data
  
  directive, args = match.groups()
  args = args.split(',')

  if directive == "db":
    binary_data.append(int(args[0], 0))
  elif directive == "dw":
    binary_data.extend(struct.pack('>H', int(args[0], 0)))
  elif directive == "dd":
    binary_data.extend(struct.pack('>I', int(args[0], 0)))
  elif directive == "dq":
    binary_data.extend(struct.pack('>Q', int(args[0], 0)))
  elif directive == "da":
    binary_data.extend(int(x.strip(), 0) for x in args)

  return binary_data

def compile_source(input_file, expanded_file, output_file):
  print("Loading...")
  macros = parse_macros(input_file)

  print("Expanding...")
  expand_macros(input_file, expanded_file, macros)

  print("Compiling...")
  parse_assembly(expanded_file, output_file)

  print("Compiled")

if __name__ == "__main__":
  output = "out.bin"
  if len(sys.argv) < 2:
    print('Fatal: Expected input file.')
    sys.exit(1)
  if (len(sys.argv) > 2):
    output = sys.argv[2]
    
  compile_source(sys.argv[1], "tmp.asm", output)