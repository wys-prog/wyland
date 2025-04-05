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

@wyland.max: 0xFE nop
  # qmm0 __A
  # qmm1 __B
  jmp @calc: .here + 9
  @wyland.max.b:
    mov qmm0 qmm1

  cmp qmm0 qmm1
  jle %wyland.max.b
  ret 

@wyland.strcmp: 0xFE nop # debug symbol (again)
  # qmm0 -> (char *)str1    = ARG 
  # qmm1 -> (char *)str2    = ARG 
  # qmm2 -> (size_t)strlen1 = strlen(str1)
  # qmm3 -> (size_t)strlen2 = strlen(str2)
  # qmm4 -> (size_t)pos     = 0
  # qmm5 -> (size_t)max     = wyland.max(strlen1, strlen2)

  load 8 bmm1 0x01

  # "save" pointers into qmm6 and qmm7
  mov qmm6 qmm0
  mov qmm7 qmm1

  # wyland.max(strlen1, strlen2)
  mov qmm0 qmm2
  mov qmm1 qmm3

  jmp %wyland.max # call

  mov qmm0 qmm5

  # for loop
  @wyland.strcmp.loop:

    add qmm2 bmm1   # Increment pointers
    add qmm3 bmm1   # Increment pointers

    movad bmm0 qmm2
    movad bmm2 qmm3 # bmm2 since bmm1 is already used for incrementing

    add qmm4 bmm1   # Increment index

    cmp bmm0 bmm2   # str1[i] == str2[i]
    jne  @calc: .here + 21

    cmp qmm4 qmm5
    jne %wyland.strcmp.loop
    jmp @calc: .here + 5

    @wyland.strcmp.not_equal:
      load 8 qmm0 0x01
      ret
    
    # Or, it's equal, so we load 1.
    load 8 qmm0 0x00
    ret