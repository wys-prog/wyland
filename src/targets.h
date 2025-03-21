#ifndef ___WYLAND_TARGETS___
#define ___WYLAND_TARGETS___

#include <stdint.h>
#include <string.h>

typedef int16_t __wtarget;

const __wtarget wtarg64    = 0xA64;
const __wtarget wtarg32    = 0xA32; /* Not implemented */
const __wtarget wtargmarch = 0xA +'m'+'a'+'t'+'h';
const __wtarget wtargfast  = 0xAFA; /* Deprecated, not implemented */

const char *nameof(__wtarget tar) {
  switch (tar) {
    case wtarg64: return "wtarg64"; break;
    case wtarg32: return "wtarg32"; break;
    case wtargmarch: return "wtargmarch"; break;
    case wtargfast: return "wtargfast"; break;
    default: return "unknown"; break;
  }
}

__wtarget ofname(const char *name) {
  if (strcmp(name, nameof(wtarg64)) == 0) return wtarg64;
  else if (strcmp(name, nameof(wtarg32)) == 0) return wtarg32;
  else if (strcmp(name, nameof(wtargmarch)) == 0) return wtargmarch;
  else if (strcmp(name, nameof(wtargfast)) == 0) return wtargfast;
  else return wtarg64;
}

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