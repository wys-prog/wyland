import sys

reserved_tokens = [
  'function', 'if', 'elseif', 'else', 'while', 'for', 
  'then', 'end', 'do', 'return', 'not', 'and', 'or', 
  'repeat', 'until', 'nil', 'local', 'in', 'goto', 'break', 
  'true', 'false'
]

functions = {}

should_stop: bool = False

# first step: "parse"
# what we'll gonna do ?
# read all the code, and define variables, functions
# and resovle functions"s addresses

class Token:
  def __init__(self, type: str, value: bytearray) -> None:
    self.type: str = type
    self.value: bytearray = value
    
  def get_type(self) -> str:
    return self.type
  
  def get_value(self) -> bytearray:
    return self.value
  
def error(what: str, line: str, line_count: int, word: str) -> None:
  word = word.strip()
  line_clean = line.rstrip('\n')  #
  emsg = f"Error {what.strip()}\n  {line_count}: {line_clean}"

  try:
    posw = line_clean.index(word)
  except ValueError:
    posw = -1
  if posw != -1:
    pointer_line = ' ' * (len(str(line_count)) + 2 + posw) + '~' * len(word)
    emsg += "\n  " + pointer_line
      
  print(emsg)  

def get_words(line: str) -> list:
  words = []
  i = 0
  n = len(line)
  
  while i < n:
    c = line[i]
    
    if c.isspace():
      i += 1
      continue

    if c == '-' and i + 1 < n and line[i+1] == '-':
      break 

    if c == '"':
      i += 1
      start = i
      string_value = ''
      while i < n:
        if line[i] == '"' and line[i-1] != '\\':
          break
        string_value += line[i]
        i += 1
      words.append('"' + string_value + '"')
      i += 1  
      continue

    if i + 1 < n and line[i:i+2] in ('==', '~=', '<=', '>='):
      words.append(line[i:i+2])
      i += 2
      continue

    if c in '(){}[],;=+-*/<>':
      words.append(c)
      i += 1
      continue

    start = i
    while i < n and (line[i].isalnum() or line[i] == '_'):
      i += 1
    words.append(line[start:i])
  
  return words

def parse(input_file) -> None:
  with open(input_file, 'r') as file:
    lnb: int = 1
    relative_address: int = 0
    
    for line in file:
      words: list = get_words(line)
      for i in range(0, len(words)):
        if words[i] == 'function':
          name: str = words[i + 1]
          if name in functions:
            error(f'redefinition of {name}', line, lnb, line)
            should_stop = True
          
    
    lnb += 1