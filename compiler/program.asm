main:
  .lea %qmm0, qword(@system.data.string)
  .lbyte %qmm1, byte(13)
  .lea %qmm15, qword(@system.program.end)
  .jmp qword(@system.print)

@system.print:
  ; char *_Str (qmm0), size_t len (qmm1)
  ; size_t i (qmm2)
  .mov %qmm4, %qmm0
  .lbyte %bmm0, byte(0x01) ; for incrementation

  ; def HANDLE
  .call-c dword(0)

  @system.print.write:
    .movad %dmm0, %qmm4

    .add %qmm4, %bmm0
    .add %qmm2, %bmm0
    .cmp %qmm2, %qmm1
    .jne qword(@system.print.write)
  
    ; clear registers
    .lbyte %bmm0, byte(0x00)
    .mov %qmm4, %bmm0
    .mov %qmm0, %bmm0
    .mov %qmm2, %bmm0
    .mov %dmm0, %bmm0

    .ret

@system.program.end:
  .data [byte(0xFF)]

@system.data:
  @system.data.string:
    .data [string("Hello, world"), byte(0x0A), byte(0x00)] ; len = 13


@lib_start:
  ; external functions
  .data [string("io/0:wyland_get_stdout,1:wyland_get_stdin,2:wyland_get_stderr,") ]
  .data [string("3:wyland_putc,4:wyland_readc,"), byte(0)]
  ; putc -> dmm0 char, qmm0 -> handle;
  ; readc -> (return)dmm0(char)
  ; (gets) -> (return)qmm0

@disk_start:
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]
  .data [qword(0x00), qword(0x00), qword(0x00), qword(0x00)]