.header word(0xA64), dword(14), qword(@_code_start), qword(@_libs_start), qword(@_disk_start)

@_code_start:

main:     ; in memory: 0x00, in filedisk: _code_start + 0x00
  .jmp qword(exit)

exit:
  .dq qword(0x00)
  .db byte(0xFF)

@_libs_start:
@_disk_start: