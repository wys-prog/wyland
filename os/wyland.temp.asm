; --- file: "wyland.asm" ---

	# First of all, include the header.
	; from "wyland.asm"
; --- file: "wyland.header.asm" ---

	; Certificat
	; from "wyland.header.asm"
	0x77 0x6C 0x66
	; from "wyland.header.asm"
	; target, wtarg64
	; from "wyland.header.asm"
	0xA64
	; from "wyland.header.asm"
	; version, 1
	; from "wyland.header.asm"
	0x0 0x0 0x0 0x1
	; from "wyland.header.asm"
	; Code section start (8 bytes)
	; from "wyland.header.asm"
	0 0 0 0 0 0 0 33
	; from "wyland.header.asm"
	; Data section start (8 bytes)
	; from "wyland.header.asm"
	0 0 0 0 0 0 0 0
	; from "wyland.header.asm"
	; Libs. section start (8 bytes)
	; from "wyland.header.asm"
	# Add utile functions
	; from "wyland.asm"
; --- file: "wyland.utiles.asm" ---

	@wyland.print: 0xFE nop # debug symbol
	; from "wyland.utiles.asm"
	# qmm0 -> char pointer
	; from "wyland.utiles.asm"
	# qmm1 -> size of string
	; from "wyland.utiles.asm"
	# qmm2 -> index
	; from "wyland.utiles.asm"
	load 8 qmm2 0x00  ; Make sure qmm2 is null.
	; from "wyland.utiles.asm"
	load 8 bmm1 0x01
	; from "wyland.utiles.asm"
	; Now we need to write this char.
	; from "wyland.utiles.asm"
	@wyland.print.write:
	; from "wyland.utiles.asm"
	; Writting the char
	; from "wyland.utiles.asm"
	movad bmm0 qmm0 # get the char
	; from "wyland.utiles.asm"
	syscall 0       # write it to STDOUT
	; from "wyland.utiles.asm"
	; Increment pointer, and current index.
	; from "wyland.utiles.asm"
	add qmm2 bmm1 # Increments our index
	; from "wyland.utiles.asm"
	add qmm0 bmm1 # Increments our pointer
	; from "wyland.utiles.asm"
	cmp qmm2 qmm1
	; from "wyland.utiles.asm"
	jne %wyland.print.write
	; from "wyland.utiles.asm"
	ret ; Return
	; from "wyland.utiles.asm"
	@wyland.strlen: 0xFE nop # debug symbol
	; from "wyland.utiles.asm"
	# qmm0 -> char pointer
	; from "wyland.utiles.asm"
	# qmm1 -> our local index
	; from "wyland.utiles.asm"
	# bmm1 -> (char)END = '\0'
	; from "wyland.utiles.asm"
	# bmm2 -> (int)INC = 1
	; from "wyland.utiles.asm"
	load 8 qmm1 0 # Ensure that our index is null !
	; from "wyland.utiles.asm"
	@wyland.strlen.loop:
	; from "wyland.utiles.asm"
	movad bmm0 qmm0
	; from "wyland.utiles.asm"
	add qmm1 bmm2
	; from "wyland.utiles.asm"
	add qmm0 bmm2
	; from "wyland.utiles.asm"
	cmp bmm0 bmm1
	; from "wyland.utiles.asm"
	jne %wyland.strlen.loop
	; from "wyland.utiles.asm"
	# return register: qmm0
	; from "wyland.utiles.asm"
	mov qmm0 qmm1
	; from "wyland.utiles.asm"
	ret
	; from "wyland.utiles.asm"
	@wyland.max: 0xFE nop
	; from "wyland.utiles.asm"
	# qmm0 __A
	; from "wyland.utiles.asm"
	# qmm1 __B
	; from "wyland.utiles.asm"
	jmp @calc: .here + 9
	; from "wyland.utiles.asm"
	@wyland.max.b:
	; from "wyland.utiles.asm"
	mov qmm0 qmm1
	; from "wyland.utiles.asm"
	cmp qmm0 qmm1
	; from "wyland.utiles.asm"
	jle %wyland.max.b
	; from "wyland.utiles.asm"
	ret
	; from "wyland.utiles.asm"
	@wyland.strcmp: 0xFE nop # debug symbol (again)
	; from "wyland.utiles.asm"
	# qmm0 -> (char *)str1    = ARG
	; from "wyland.utiles.asm"
	# qmm1 -> (char *)str2    = ARG
	; from "wyland.utiles.asm"
	# qmm2 -> (size_t)strlen1 = strlen(str1)
	; from "wyland.utiles.asm"
	# qmm3 -> (size_t)strlen2 = strlen(str2)
	; from "wyland.utiles.asm"
	# qmm4 -> (size_t)pos     = 0
	; from "wyland.utiles.asm"
	# qmm5 -> (size_t)max     = wyland.max(strlen1, strlen2)
	; from "wyland.utiles.asm"
	load 8 bmm1 0x01
	; from "wyland.utiles.asm"
	# "save" pointers into qmm6 and qmm7
	; from "wyland.utiles.asm"
	mov qmm6 qmm0
	; from "wyland.utiles.asm"
	mov qmm7 qmm1
	; from "wyland.utiles.asm"
	# wyland.max(strlen1, strlen2)
	; from "wyland.utiles.asm"
	mov qmm0 qmm2
	; from "wyland.utiles.asm"
	mov qmm1 qmm3
	; from "wyland.utiles.asm"
	jmp %wyland.max # call
	; from "wyland.utiles.asm"
	mov qmm0 qmm5
	; from "wyland.utiles.asm"
	# for loop
	; from "wyland.utiles.asm"
	todo  @wyland.strcmp.loop:
	; from "wyland.utiles.asm"
	todo
	; from "wyland.utiles.asm"
	todo    add qmm2 bmm1   # Increment pointers
	; from "wyland.utiles.asm"
	todo    add qmm3 bmm1   # Increment pointers
	; from "wyland.utiles.asm"
	todo
	; from "wyland.utiles.asm"
	todo    movad bmm0 qmm2
	; from "wyland.utiles.asm"
	todo    movad bmm2 qmm3 # bmm2 since bmm1 is already used for incrementing
	; from "wyland.utiles.asm"
	todo
	; from "wyland.utiles.asm"
	todo    add qmm4 bmm1   # Increment index
	; from "wyland.utiles.asm"
	todo
	; from "wyland.utiles.asm"
	todo    cmp bmm0 bmm2   # str1[i] == str2[i]
	; from "wyland.utiles.asm"
	todo    je  @calc: .here +
	; from "wyland.utiles.asm"
	todo
	; from "wyland.utiles.asm"
	todo    cmp qmm4 qmm5
	; from "wyland.utiles.asm"
	# Finally, add OS's core
	; from "wyland.asm"
; --- file: "wyland.main.asm" ---

	@wyland.string.hello: .ascii "wyland" 0x0A 0x00
	; from "wyland.main.asm"
	@wyland_start:
	; from "wyland.main.asm"
	# prepare call for "strlen"
	; from "wyland.main.asm"
	# wyland.strlen(&string.hello)
	; from "wyland.main.asm"
	lea qmm0 %wyland.string.hello
	; from "wyland.main.asm"
	lea qmm15 @calc: .here + 10
	; from "wyland.main.asm"
	jmp %wyland.strlen
	; from "wyland.main.asm"
