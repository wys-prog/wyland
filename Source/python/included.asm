#define load    db(0x01)
#define store   db(0x14)
#define lea     db(0x15)
#define mov     db(0x02)
#define add     db(0x03)
#define sub     db(0x04)
#define mul     db(0x05)
#define div     db(0x06)
#define mod     db(0x07)
#define jmp     db(0x08)
#define je      db(0x09)
#define jne     db(0x0A)
#define jg      db(0x0B)
#define jl      db(0x0C)
#define cmp     db(0x0D)
#define call    db(0x0E)
#define ret     db(0x0F)
#define xor     db(0x10)
#define or      db(0x11)
#define and     db(0x12)
#define syscall db(0x13)
#define halt    db(0xFF)

#define r0      db(0x00)
#define r1      db(0x01)
#define r2      db(0x02)
#define r3      db(0x03)
#define r4      db(0x04)
#define r5      db(0x05)
#define r6      db(0x06)
#define r7      db(0x07)
#define r8      db(0x08)
#define r9      db(0x09)
#define r10     db(0x0A)
#define r11     db(0x0B)
#define r12     db(0x0C)
#define r13     db(0x0D)
#define r14     db(0x0E)
#define r15     db(0x0F)
#define r16     db(0x10)
#define r17     db(0x11)
#define r18     db(0x12)
#define r19     db(0x13)
#define r20     db(0x14)
#define r21     db(0x15)
#define r22     db(0x16)
#define r23     db(0x17)
#define r24     db(0x18)
#define r25     db(0x19)
#define r26     db(0x1A)
#define r27     db(0x1B)
#define r28     db(0x1C)
#define r29     db(0x1D)
#define r30     db(0x1E)
#define r31     db(0x1F)

#define resb(x) times x db(0x00)

#define label   
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
