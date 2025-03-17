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
  march_nop, 
  march_lea,
  march_load, 
  march_store, 
  march_mov, 
  march_add, 
  march_sub, 
  march_mul, 
  march_odiv, 
  march_mod, 
  march_jmp, 
  march_je, 
  march_jne, 
  march_jl, 
  march_jg, 
  march_jle, 
  march_jge,
  march_cmp,
  march_xint,
  march_loadat, 
  march_ret, 
  march_movad, 
  /* Specific instructions */
  march_loadfloat126, 
  march_storefloat128, 
  march_movfloat128, 
  march_addfloat128, 
  march_subfloat128, 
  march_mulfloat128, 
  march_divfloat128, 
  march_modfloat128, 
  march_loadrfloat128, /* Load a register in a 128 float register. */
};

#endif //___WYLAND_TARGETS___