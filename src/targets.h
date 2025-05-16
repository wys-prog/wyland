#ifndef ___WYLAND_TARGETS___
#define ___WYLAND_TARGETS___

#ifdef _WIN32
#include <windows.h>
#endif 

#include <stdint.h>
#include <string.h>

typedef int16_t warch_t;

const warch_t warch64    = 0xA64;
const warch_t warch128   = 0xA128;
const warch_t wtarg32    = 0xA32; /* Not implemented */
const warch_t wtargmarch = 0xA + (0x13)*'m'+'a'+'t'+'h' + 2; /* These numbers are just 'random'. */
const warch_t wtargfast  = 0xAFA; /* Deprecated, not implemented */
const warch_t wdebugger  = 0xA6D;

const char *nameof(warch_t tar) {
  switch (tar) {
    case warch64: return "warch64"; break;
    case wtarg32: return "wtarg32"; break;
    case wtargmarch: return "wtargmarch"; break;
    case wtargfast: return "wtargfast"; break;
    case wdebugger: return "wdebugger"; break;
    case warch128: return "warch128"; break;
    default: return "unknown"; break;
  }
}

warch_t ofname(const char *name) {
  if (strcmp(name, nameof(warch64)) == 0) return warch64;
  else if (strcmp(name, nameof(wtarg32)) == 0) return wtarg32;
  else if (strcmp(name, nameof(wtargmarch)) == 0) return wtargmarch;
  else if (strcmp(name, nameof(wtargfast)) == 0) return wtargfast;
  else if (strcmp(name, nameof(wdebugger)) == 0) return wdebugger;
  else if (strcmp(name, nameof(warch128)) == 0) return warch128;
  else return strlen(name); /* This can create errors... BUT WHY SET TO AN UNKNOWN TARGET ? */
}

enum set_arch64 {
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
  sal, 
  sar, 
  owthrow, 
  clfn, // Call Linked Function, std:wy2.4
  empl,
  push_mmio, 
  pop_mmio,
  connect_mmio, 
  deconnect_mmio,
  oxor, 
  oor, 
  oand,
  inc, 
  dec
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