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
    'clfn': 25,
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

symbols = {
    'main': 524288000,
    'end_main': 524288128, 
    '.code': 524288128, 
    '.here': 0
}

def error(what: str, line: str, line_count: int, word: str):
    word = word.strip()
    line_clean = line.rstrip('\n')  # pour éviter les sauts de ligne parasites
    emsg = f"Error {what.strip()}\n  {line_count}: {line_clean}"

    # Essayer de localiser le mot exact
    try:
        posw = line_clean.index(word)
    except ValueError:
        posw = -1

    if posw != -1:
        pointer_line = ' ' * (len(str(line_count)) + 2 + posw) + '~' * len(word)
        emsg += "\n  " + pointer_line

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

def evaluate_expression(expression: str, line: str, line_count: int):
    try:
        tokens = expression.split()
        result = 0
        operator = '+'
        for token in tokens:
            if token in symbols:
                value = symbols[token]
            elif token.lower().startswith(('0x', '0o')) or token.isdigit():
                value = get_int(token, line, line_count)
                if value is None:
                    return None
            elif token in ('+', '-'):
                operator = token
                continue
            else:
                error(f"Unknown token in expression: '{token}'", line, line_count, token)
                return None

            if operator == '+':
                result += value
            elif operator == '-':
                result -= value

        return result
    except Exception as e:
        error(f"Failed to evaluate expression: {expression}", line, line_count, expression)
        return None

def assemble_file(input_file, output_file):
    global current_address
    error_register = 0
    current_address = 0
    line_count = 0
    in_main = False
    
    with open(input_file, 'r') as infile, open(output_file, 'wb') as outfile:
        for line in infile:
            line_count += 1
            words = line.split(' ')
            for i in range(0, len(words)):
                word = words[i].strip()
                if (word.startswith(';') or word.startswith('#')): break
                
                if word == '.string':
                    wpos = line.find(word)
                    beg = line.find('"', wpos + len(word))
                    end = line.find('"', beg + 1)
                    if beg != -1 and end != -1:
                        string_content = line[beg + 1:end] 
                        outfile.write(string_content.encode('utf-8')) 
                        current_address += len(string_content.encode('utf-8'))
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
                        current_address += len(string_content.encode('ascii'))
                    else:
                        error(f"Malformed string", line.strip(), line_count, line.strip())
                        error_register += 1
                    break

                binary_instruction = assemble_word(word)
                
                if binary_instruction:
                    byte = int(binary_instruction, 2).to_bytes(1, byteorder='big')
                    outfile.write(byte)
                    current_address += 1
                elif word == '': 
                    continue
                elif word == '@at:':
                    at = words[i + 1]
                    byte = get_int(at, line, line_count)
                    if not byte: continue
                    byte = int(byte)
                    outfile.write(byte.to_bytes(8, 'big'))
                    current_address += 8
                    break
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
                elif word.lower() == '@calc:':
                    if i + 1 < len(words):  
                        expression = ' '.join(words[i + 1:]).strip()
                        value = evaluate_expression(expression, line, line_count)
                        if value is not None:
                            outfile.write(value.to_bytes(8, byteorder='big'))
                            current_address += 8
                        else:
                            error(f"Invalid expression for '@calc:': {expression}", line, line_count, expression)
                            error_register += 1
                        break
                    else:
                        error("Missing expression for '@calc:'", line, line_count, '')
                        error_register += 1
                        break
                elif word.lower() == "@%here":
                    outfile.write(current_address.to_bytes(8, byteorder='big'))
                    current_address += 8
                elif word.startswith('@'):
                    if word.endswith(':'):
                        symbols[word[1:len(word)-1]] = current_address
                        if word[1:len(word)-1] == 'main': in_main = True
                    else:
                        symbols[word[1:]] = current_address
                        if word[1:] == 'main': in_main = True
                elif word.startswith('%'):
                    if not word[1:] in symbols:
                        error(f"Unknown referenced symbol (could not resovle): '{word.strip()}'", line, line_count, word.strip())
                        error_register += 1
                        continue
                    outfile.write(symbols[word[1:]].to_bytes(8, byteorder='big'))
                    current_address += 8
                elif word.lower() == '.worg':
                    current_address = 524288000
                elif word.lower() == '.wcorg':
                    current_address = 524288128
                elif word.lower() == '.mend':
                    if in_main:
                        outfile.write(b'\x00' * (symbols['end_main'] - current_address))
                        current_address += symbols['end_main'] - current_address
                        current_address = symbols['.code']
                        in_main = False
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
                    break
                elif word.lower() == '.wyland_system_org':
                    current_address = 524288000
                else:
                    rawdata, size = encode_value(word, 10)
                    if rawdata:
                        outfile.write(int(rawdata, 2).to_bytes(size, byteorder='big'))
                        current_address += size
                    else:
                        error(f'Unknown instruction "{word.strip()}"', line, line_count, word.strip())
                        error_register += 1
            symbols['@here:'] = current_address
            
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