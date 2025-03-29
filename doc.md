# WORKING ON !
I didnt finished so idk 

i'll put some programs that i made, with some explications.
However, i dont think that's the best to learn how to use my VM, so 
you have multiple choice:
  1 - You read the code, and learn by reading it 
  2 - You test something, look if it works, why, and how
  3 - You test, read the code, and read this "documentation" (there's nothing but dont tell it!)
  4 - Ask me questions ?


## Examples of code 

### A program that throws an error if you press 'ESC' key
```asm
.worg
@main:
  load 8 bmm5 27    ; bmm5 is a "random" register, of 8 bits. You can also use 
                    ; another register, but in this example we'll use this one.
  nop               ; A place-holder before nesteded label. Ignore it can make 
                    ; issues that i don't know for the moment.
  @main.loop:       ; Nesteded label, a while loop.
    cmp dmm0 bmm5   ; Comparing the dmm0 register, which contains the last-pressed key in ASCII code.
    jne %main.loop  ; jump at the nesteded label if the last-pressed key isn't 'ESC'
  
  nop               ; Idk why.. Just for fun ?

  0xFF              ; Halting the core.
.mend               ; Defines the end of the 'main' section.
                    ; 'main' section is a specific section, to initialize
                    ; custom values for your program.
                    ; After the 'main' section, you can code like you want, 
                    ; there isn't any limitation (exepted 12MB for the system-segment, and around 400MB for the code segment)

.wcorg              ; Defines the origin of the non-main section.
                    ; 'main' section is limited to 128 bytes.
```