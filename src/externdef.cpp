#include <boost/container/flat_map.hpp>

#include "wmmio.hpp"
#include "disk.hpp"
#include "bios/bios.hpp"

#include "cache.hpp"

namespace wylma {
  namespace wyland {
    namespace wyland_terminate_data {
      typedef void(*wyscppecllb)(void);
      std::vector<wyscppecllb> end_callables;
    }

    [[maybe_unused]] uint8_t *memory;
    [[maybe_unused]] uint64_t code_start;
  }
}
