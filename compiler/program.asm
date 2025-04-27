; header
header:
  .string "wlf"
  .dw word(0xA64)
  .dd dword(14)
  .dq qword(@_code_start)
  .dq qword(@_disk_start)
  .dq qword(@_libs_start)

@_code_start:

main:
  .jmp qword(main.code)
  main.debug:
    .string "main"
    .db byte(0)

  main.code:
    .pushmmio byte(0x90), byte(0x3F)

exit:
  .db byte(0xFF)

@_libs_start:
@_disk_start: