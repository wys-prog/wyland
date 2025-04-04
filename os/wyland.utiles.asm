@wyland.print: 0xFE nop # debug symbol
  # qmm0 -> char pointer
  # qmm1 -> size of string
  # qmm2 -> index
  load 8 qmm2 0x00  ; Make sure qmm2 is null.
  load 8 bmm1 0x01

  ; Now we need to write this char.
  @wyland.print.write:
    ; Writting the char
    movad bmm0 qmm0 # get the char
    syscall 0       # write it to STDOUT
    ; Increment pointer, and current index.
    add qmm2 bmm1 # Increments our index
    add qmm0 bmm1 # Increments our pointer
    cmp qmm2 qmm1 
    jne %wyland.print.write 

    ret ; Return

@wyland.strlen: 0xFE nop # debug symbol
  # qmm0 -> char pointer
  # qmm1 -> our local index 
  # bmm1 -> (char)END = '\0'
  # bmm2 -> (int)INC = 1

  load 8 qmm1 0 # Ensure that our index is null !
  @wyland.strlen.loop:
    movad bmm0 qmm0
    
    add qmm1 bmm2
    add qmm0 bmm2
    cmp bmm0 bmm1
    jne %wyland.strlen.loop

  # return register: qmm0
  mov qmm0 qmm1
  ret 