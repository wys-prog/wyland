#include "asmlib.asm"
; Include assembly file.

#define main: dq(0x00)
#define loop: dq(28)
#define end:  dq(41)

label main: 
  load r10, dq(0x00) ; dq defines a 64-bit value.
  load r11, dq(0xA7) ; max 
  load r20, dq(0x01) 

  jmp loop
  ret

label loop:
  add r10, r20
  cmp r10, r11
  jne loop
  jmp end
  ret

label end:
  halt
  ret
