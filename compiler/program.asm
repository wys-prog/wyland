.header word(0xA64), dword(14), qword(@_code_start), qword(@_libs_start), qword(@_disk_start)
; we can put additionial informations here !
; for example, table of symbol (instead of putting at the end of the file)
; or in the case we use a custom disk (avaible soon), we can put informations about disk here.

@_code_start:

main:     ; in memory: 0x00, in filedisk: _code_start + 0x00

  .lbyte %dmm0, byte(0x77)
  .int dword(0x9FEA)
  .lbyte %dmm0, byte(0x6C)
  .int dword(0x00)
  .lbyte %dmm0, byte(0x66)
  .int dword(0x00)

  .jmp qword(exit)
  
exit:
  .dq qword(0x00)
  .db byte(0xFF)

@_libs_start:
@_disk_start: