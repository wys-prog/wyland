main:
  .jmp qword(.loop)
  main.end:
    .db byte(0xFF)


.loop:
  .lbyte %qmm0, byte(0xFF)
  .lbyte %qmm1, byte(0x00)
  .lbyte %qmm2, byte(0x01)

  looping:
    .add %qmm1, %qmm2
    .cmp %qmm0, %qmm1
    .jne qword(looping)
    .jmp qword(main.end)