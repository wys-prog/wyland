; Certificat
0x77 0x6C 0x66
; target, wtarg64
0xA64
; version, 1
0x0 0x0 0x0 0x1 
; code section starts at 0x00 + 33
0 0 0 0 0 0 0 33

; There isn't any data section
0 0 0 0 0 0 0 0 

; Lib section starts at (code_section) + (code_size)
; wich is 33 + 5 + 1= 39 (wow)
0 0 0 0 0 0 0 48
load 8 dmm0 0x63
clfn 0 0 0 1
clfn 0 0 0 0
0xFF

; libs
.string "io/0:wyland_putc,1:wyland_get_stdout,"