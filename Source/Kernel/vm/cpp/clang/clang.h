#ifndef ___CLANG_H___
#define ___CLANG_H___

#define NOP 0x00

#define LOAD 0x01 /* Load next 8bytes into a register */
#define STORE 0x14 /* Save @r to next 8bytes (as dst) */

#define MOV 0x02
#define ADD 0x03
#define SUB 0x04
#define MUL 0x05
#define DIV 0x06
#define MOD 0x07

#define JMP 0x08
#define JE 0x09
#define JNE 0x0A
#define JG 0x0B
#define JL 0x0C

#define CMP 0x0D
#define CALL 0x0E
#define RET 0x0F

#define XOR 0x10 
#define OR 0x11
#define AND 0x12

#define SYSCALL 0x13

#define HALT 0xFF

#endif