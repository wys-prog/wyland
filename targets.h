#ifndef ___WYLAND_TARGETS___
#define ___WYLAND_TARGETS___

#include <stdint.h>

typedef int __wtarget;

__wtarget wtarg64    = 0xA64;
__wtarget wtarg32    = 0xA32; /* Not implemented */
__wtarget wtargmarch = 0xAC;
__wtarget wtargfast  = 0xAFA; /* Deprecated, not implemented */

enum set_wtarg64 {
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

enum set_wtargmarch {
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
  /* Specific instructions */
  loadfloat126, 
  storefloat128, 
  movfloat128, 
  addfloat128, 
  subfloat128, 
  mulfloat128, 
  divfloat128, 
  modfloat128, 
  loadrfloat128, /* Load a register in a 128 float register. */
};

#endif //___WYLAND_TARGETS___