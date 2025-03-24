#pragma once

#include <string>
#include <sstream>

#include "wylrt.h"

namespace wylma {
  namespace wyland {
    namespace runtime {

      class wyland_runtime_error : wylrterror {
      private:
        wylrterror *object;

      public:

        virtual const char *what() const {
          return (const char*)object->what;
        }

        virtual const char *name() const {
          return (const char*)object->name;
        }

        virtual const char *caller() const {
          return (const char*)object->caller;
        }

        virtual const char *exception_name() const {
          return this->object->exception_type;
        }

        uint64_t ip() const {
          return object->ip;
        }

        uint64_t thread() const {
          return object->thread;
        }

        uint64_t *segmbeg() const {
          return object->segmbeg;
        }

        uint64_t *segmend() const {
          return object->segmend;
        }

        uint64_t segmsize() const {
          return object->segsize;
        }

        std::string fmterr(const char *on_new_line = "") const {
          std::stringstream ss;

          ss << name() << ": " << what() << "\n" << on_new_line << 
          "from:\t" << caller() << "\n" << on_new_line << 
          "ip:  \t" << ip() << "\n" << on_new_line << 
          "flags:\tbeg(" << std::hex << segmbeg() << "), end(" << segmend() << ")\n" << on_new_line << 
          "segment size: " << segmsize() << std::endl;

          return ss.str();
        }
      
        wyland_runtime_error() : object(nullptr) {
          object = new wylrterror;
        }

        ~wyland_runtime_error() {
          delete[] object;
        }
      };


    }
  }
}
