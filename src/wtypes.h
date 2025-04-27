#ifndef ___WYLAND_TYPES_H___
#define ___WYLAND_TYPES_H___

#ifdef _WIN32
#include <windows.h>
#endif 

#include <stdint.h>

typedef void (*lambda)();

typedef struct {
  int64_t integer;
  uint64_t floating;
} _march_floatr_t;

typedef int32_t _march_exp_t;

typedef struct {
  int64_t integer;
  uint64_t floating;
  _march_exp_t exponent;
} _march_rprecision_t;

/* in the future ! */

#define MARCH_STACK_SIZE 512

typedef struct {
  int32_t id;
  uint64_t object;
} _march_task_t;

typedef struct {
  _march_task_t tasks[MARCH_STACK_SIZE];
  uint16_t      pos;
} _march_stack_task_t;

#endif // ___WYLAND_TYPES_H___