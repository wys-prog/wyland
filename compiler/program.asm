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
  .string "/Users/wys/Documents/wyland/src/libs/bin/io%0:wyland_close,1:wyland_get_stdout,2:wyland_get_stdin,3:wyland_get_stderr"
  .db byte(0)
  .string "/Users/wys/Documents/wyland/src/libs/bin/io2%0:wyland_close,1:wyland_get_stdout,2:wyland_get_stdin,3:wyland_get_stderr"
  .db byte(0)

@_disk_start: