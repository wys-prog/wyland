

; section .code
boot:
  .lbyte %qmm1, byte(0x0F)
  .lea %qmm0, qword(system.data.strb1)
  .lea byte(0x3F), qword(system.stop)
  .jmp qword(print)

; libs

; simple system print function. That function will print 8 bits chars.
print:
  ; print(wstrptr str, wulong len);
  ; qmm0 -> (wstrptr)str
  ; qmm1 -> (wulong)len
  ; qmm2 -> (wulong)i
  ; bmm0 -> (const wint)(1)
  ; bmm1 -> (char)c

  .lbyte %bmm0, byte(0x01) ; (const wint)i = (1);
  .lbyte %qmm2, byte(0x00) ; (wulong)i = 0;

  print.write:
    .movad %bmm1, %qmm0
    .pushmmio byte(0x00), %bmm1 
    
    .add %qmm2, %bmm0
    .add %qmm0, %bmm0
    .cmp %qmm2, %qmm1
    .jne qword(print.write)
    .jmp qword(print.end)

  print.end:
    .ret

system:  
  system.stop:
    .db byte(0xFF)

; system reserved disks
system.data:
  system.data.strb1:
    .string "Hello, world !"
    .db byte(0x0A)
    ; len: 14

system.disk:
  .resb(512)

user.disk:
  .resb(2048)