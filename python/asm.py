instruction_set = {
    'nop': 0,
    'lea': 1,
    'load': 2,
    'store': 3,
    'mov': 4,
    'add': 5,
    'sub': 6,
    'mul': 7,
    'div': 8,
    'mod': 9,
    'jmp': 10,
    'je': 11,
    'jne': 12,
    'jl': 13,
    'jg': 14,
    'jle': 15,
    'jge': 16,
    'cmp': 17,
    'int': 18,
    'syscall': 18,
    'loadat': 19,
    'ret': 20,
    'movad': 21,
    'sal': 22,
    'sar': 23,
    'wthrow': 24,
}



op_type = {
    'byte': 8,
    'word': 16, 
    'dword': 32, 
    'qword': 64,
}


# Registers
# byte:  bmm0, ..., bmm15
# word:  wmm0, ..., wmm15
# dword: dmm0, ..., dmm15
# qword: qmm0, ..., qmm31

register = {
    # Byte registers
    **{f'bmm{i}': i for i in range(16)},
    # Word registers
    **{f'wmm{i}': i + 16 for i in range(16)},
    # Dword registers
    **{f'dmm{i}': i + 32 for i in range(16)},
    # Qword registers
    **{f'qmm{i}': i + 48 for i in range(32)},
}

symbols = {}

def error(what: str, line: str, line_count: int, word: str):
    word = word.strip()
    emsg = f"Error {what.strip()}\n  {line_count}: {line.strip()}\n  "
    posw = line.find(word)
    if posw != -1:
        emsg += (' ' * (posw + len(str(line_count)))) + ('~' * len(word))
    print(emsg)

def get_int(word: str, line: str, line_count):
    base = 10
    if word.lower().startswith('0x'): base = 16
    elif word.lower().startswith('0o'): base = 8

    try:
        return int(word, base)
    except ValueError:
        error(f"Not an integer: '{word.strip()}'", line, line_count, word.strip())
        return None

def assemble_word(word):
    word = word.strip()
    if word in instruction_set:
        return bin(instruction_set[word])[2:].zfill(8)
    elif word in register:
        return bin(register[word])[2:].zfill(8)
    
    else:
        return None
    
def encode_value(word, base): # Idk what my IDE is sayin.. But OK.
    try:
        value = int(word, base)
        
        if value <= 0xFF:  # 8 bits
            return bin(value)[2:].zfill(8), 1
        elif value <= 0xFFFF:  # 16 bits
            return bin(value)[2:].zfill(16), 2
        elif value <= 0xFFFFFFFF:  # 32 bits
            return bin(value)[2:].zfill(32), 4
        elif value <= 0xFFFFFFFFFFFFFFFF:  # 64 bits
            return bin(value)[2:].zfill(64), 8
        else:
            return None, 0  
    except ValueError:
        return None, 0  

def assemble_file(input_file, output_file):
    global current_address
    error_register = 0
    current_address = 0
    line_count = 0
    
    with open(input_file, 'r') as infile, open(output_file, 'wb') as outfile:
        for line in infile:
            line_count += 1
            words = line.split(' ')
            for i in range(0, len(words)):
                word = words[i].strip()
                
                if word == '.string':
                    wpos = line.find(word)
                    beg = line.find('"', wpos + len(word))
                    end = line.find('"', beg + 1)
                    if beg != -1 and end != -1:
                        string_content = line[beg + 1:end] 
                        outfile.write(string_content.encode('utf-8')) 
                        outfile.write(b'\x00')
                        current_address += len(string_content.encode('utf-8')) + 1
                    else:
                        error(f"Malformed string", line.strip(), line_count, line.strip())
                        error_register += 1
                    break
                elif word == '.ascii':
                    wpos = line.find(word)
                    beg = line.find('"', wpos + len(word))
                    end = line.find('"', beg + 1)
                    if beg != -1 and end != -1:
                        string_content = line[beg + 1:end] 
                        outfile.write(string_content.encode('ascii')) 
                        outfile.write(b'\x00')
                        current_address += len(string_content.encode('ascii')) + 1
                    else:
                        error(f"Malformed string", line.strip(), line_count, line.strip())
                        error_register += 1
                    break

                binary_instruction = assemble_word(word)
                
                if binary_instruction:
                    byte = int(binary_instruction, 2).to_bytes(1, byteorder='big')
                    outfile.write(byte)
                elif word == '': 
                    continue
                elif word.lower().startswith('0x'):
                    rawdata, size = encode_value(word[2:], 16)
                    if not rawdata:
                        error(f"Too large or bad format: '{word.strip()}'", line, line_count, word.strip())
                        error_register += 1
                        continue
                    outfile.write(int(rawdata, 2).to_bytes(size, byteorder='big'))
                    current_address += size
                elif word.lower().startswith('0o'):
                    rawdata, size = encode_value(word[2:], 8)
                    if not rawdata:
                        error(f"Too large or bad format: '{word.strip()}'", line, line_count, word.strip())
                        error_register += 1
                        continue
                    outfile.write(int(rawdata, 2).to_bytes(size, byteorder='big'))
                    current_address += size
                elif word.lower() == "@here":
                    outfile.write(current_address.to_bytes(8, byteorder='big'))
                    current_address += 8
                elif word.startswith('@'):
                    if word.endswith(':'):
                        symbols[word[1:len(word)-1]] = current_address
                    else:
                        symbols[word[1:]] = current_address
                elif word.startswith('%'):
                    if not word[1:] in symbols:
                        error(f"Unknown referenced symbol (could not resovle): '{word.strip()}'", line, line_count, word.strip())
                        error_register += 1
                        continue
                
                    outfile.write(symbols[word[1:]].to_bytes(8, byteorder='big'))
                    current_address += 8
                elif word.lower() == 'org':
                    if i + 1 < len(words):  
                        value = get_int(words[i + 1], line, line_count)
                        i += 1  
                        if value is not None:  
                            current_address = value
                        else:
                            error(f"Invalid 'org' argument", line, line_count, words[i])
                            
                            error_register += 1
                    else:
                        error('Missing argument for \'org\'', line, line_count, '')
                        error_register += 1
                    continue
                elif word.lower() in op_type:

                    continue
                else:
                    rawdata, size = encode_value(word, 10)
                    if rawdata:
                        outfile.write(int(rawdata, 2).to_bytes(size, byteorder='big'))
                        current_address += size
                    else:
                        error(f'Unknown instruction "{word.strip()}"', line, line_count, word.strip())
                        error_register += 1
        
    
        if error_register != 0:
            print(f'{error_register} generated.')
            outfile.truncate(0)
            return error_register
                    
    
    return 0

if __name__ == '__main__':
    import sys
    if len(sys.argv) == 3:
        sys.exit(assemble_file(sys.argv[1], sys.argv[2]))
    elif len(sys.argv) == 2:
        output_file = 'wyland.section.disk.bin'
        sys.exit(assemble_file(sys.argv[1], output_file))
    elif len(sys.argv) == 1:
        try:
            sys.exit(assemble_file('wyland.section.main.asm', 'wyland.section.disk.bin'))
        except FileNotFoundError:
            error("Input file not found", "", 0, "wyland.section.main.asm")
            sys.exit(-1)
    else:
        error("Invalid arguments", "", 0, " ".join(sys.argv[1:]))
        sys.exit(-1)