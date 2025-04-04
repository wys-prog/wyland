# a horible Python code.

string = input('➜ Write something (i\'ll convert it into Wyland Machine code): ')

with open('wyland.haha.asm', "w") as file:
  for c in string:
    #file.write(int(18).to_bytes(1, "big"))
    #file.write(int(0).to_bytes(1, "big"))
    #file.write(c.encode('utf-8'))
    file.write(f'load 8 00 {ord(c)} syscall 0x00 ; letter "{c}"\n')