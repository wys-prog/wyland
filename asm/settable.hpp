#pragma once

#include <cstdint>

namespace wylma {
  enum eins : uint8_t {
    nop, 
    lea,
    load, 
    store, 
    mov, 
    add, 
    sub, 
    mul, 
    odiv, 
    mod, 
    jmp, 
    je, 
    jne,
    jl, 
    jg, 
    jle, 
    jge,
    cmp,
    xint,
    loadat, 
    ret, 
    movad, 
  };
}