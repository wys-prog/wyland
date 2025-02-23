#include <iostream>
#include <bits/stdc++.h>

enum instructions : uint8_t
{
    i_nop,   // OK
    i_load,  // OK
    i_store, // OK
    i_lea,   // OK
    i_mov,   // OK
    i_add,   // OK
    i_sub,   // OK
    i_mul,   // OK
    i_div,   // OK
    i_cmp,   // OK
    i_jmp,   // OK
    i_je,    // OK
    i_jne,   // OK
    i_jg,    // OK
    i_jl,    // OK
    i_jge,   // OK
    i_jle,   // OK
    i_jz,    // OK
    i_jnz,   // OK
    i_xor,   // OK
    i_or,    // OK
    i_and,   // OK
    i_int,   // OK?
    i_call,  // OK
    i_ret,   // OK
    i_halt,  // OK
};

class Instruction {
public:
    uint8_t base[8];

    Instruction(uint8_t ins = 0, uint8_t size = 0, uint8_t rt = 0, uint8_t vt = 0, uint16_t rv = 0, uint16_t vv = 0) {
        base[0] = ins;
        base[1] = size;
        base[2] = rt;
        base[3] = vt;
        base[4] = rv >> 8;
        base[5] = rv & 0xFF;
        base[6] = vv >> 8;
        base[7] = vv & 0xFF;
    }
};

int main()
{
    int Register = 0;
    int IMM = 1;
    int Counter = 0;
    int Max = 1;
    std::vector<Instruction> Instructions = {
        Instruction(i_load, 64, Register, IMM, Max, 0xDEAD),
        Instruction(i_add, 64, Register, IMM, Counter, 1), 
        Instruction(i_cmp, 64, Register, Register, Max, Counter), 
        Instruction(i_jne), Instruction(), 
        Instruction(0xFF)
    };

    std::ofstream file("out.bin");
    for (const auto &i : Instructions) {
        file.write((char*)i.base, 8);
    }
    file.close();

    return 0;
}