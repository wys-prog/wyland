org 524288065
print: ; @beg*:48, @len:49, (@ret:63)
  ; local(counter:50)
  load, 8, 1, 1
  
  @print.write:
    movad, 0, 48
    ; call the interuption
    swritec

  add 50, 1 
  add 48, 1
  cmp 50, 49
  jne & dq @print.write

  ret

string:
  ; db "Hello, world !", 0x0A
  db 'H', 'e', 'l', 'o', 0x0A

org SYSTEM_SEGMENT_START

main:
  lea 63 & dq main
  lea 48 & dq string
  lea 49 & dq 5
  jmp & dq print
  ; sizeof(main): 0x27 (39)
  dq 0 
  dq 0 
  dq 0 
  db 0xFF

