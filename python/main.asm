.worg
@main: 
  ; sign the label (not necessary)
  0xFE 0x00

  ; char *str
  lea qmm0 @calc: .code + 2
  load 8 qmm1 12
  lea qmm4 @calc: .code
  lea qmm15 @calc: .here + 10
  jmp @calc: .code + 14
  0xFF
.mend

.wcorg 
@.pstop: 0xFF
@.lstring_msg: 0xFE .string "Hello, world"

; will write our string, with a '\n' at the end.
@print:
  nop 0xFE nop
  ; qmm0 -> (char*)str
  ; qmm1 -> (size_t)size
  ; qmm2 -> (size_t)i (our counter)
  ; qmm4 -> (void*)return_address
  load 8 qmm2 0x00  ; Make sure qmm2 is null.
  load 8 bmm1 0x01

  ; Now we need to write this char.
  @print.write:
    ; Writting the char
    movad bmm0 qmm0
    syscall 0
    ; Increment pointer, and current index.
    add qmm2 bmm1 
    add qmm0 bmm1
    cmp qmm2 qmm1
    jne %print.write 

    load 8 bmm0 0x0A
    syscall 0

    ret ; Return

