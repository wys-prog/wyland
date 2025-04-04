#include "wyland-runtime/wylrt.h"

#include "SDL2/include/SDL.h"

wyland_extern(wyland_monitor_init, flags) {
  wint sdl_init = SDL_Init(SDL_INIT_EVERYTHING);

  if (!sdl_init) {
    wylrterror error = wyland_make_error("wyland_monitor_init", "SDL init error", SDL_GetError(), flags->ip, NULL, NULL, -1, flags->thread);
    wyland_throw(&error);
  }

  
}