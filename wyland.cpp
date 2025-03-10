#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include "regs.hpp"
#include "wylrt.h"
#include "libcallc.hpp"

constexpr std::size_t operator""_MB(unsigned long long size) {
  return size * 1024 * 1024;
}

constexpr std::size_t operator""_GB(unsigned long long size) {
  return size * 1024 * 1024 * 1024;
}

#define SYSCALL_COUNT 11

template <typename TyVec>
std::string format(const std::initializer_list<TyVec> &v, char del = ' ') {
  std::string my_format = "";
  for (const auto &e:v) my_format += std::to_string(e) + del;
  my_format.pop_back();
  return my_format;
}

std::string format(const std::initializer_list<uint8_t> &v, char del = ' ') {
  std::ostringstream oss;

  oss << std::hex << std::uppercase;

  for (const auto&e:v) oss << (int)e << del;
  
  auto my_fmt = oss.str();
  
  my_fmt.pop_back();

  return my_fmt;
}

std::string format(const std::string &raw_string) {
  std::string result;
  for (char c : raw_string) {
    if (static_cast<unsigned char>(c) < 128) {
      result += c;
    }
  }
  return result;
}

template <typename T>
inline uint8_t* to_bin(const T &__T) {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  uint8_t *buff = new uint8_t[sizeof(T)];

  for (size_t i = 0; i < sizeof(T); i++) {
    buff[i] = (__T >> ((sizeof(T) - 1 - i) * 8)) & 0xFF;
  }

  return buff;
}

#define CODE_SEGMENT_SIZE 400_MB
#define HARDWARE_SEGMENT_SIZE 100_MB
#define SYSTEM_SEGMENT_SIZE 12_MB

#define CODE_SEGMENT_START 0
#define HARDWARE_SEGMENT_START (CODE_SEGMENT_START + CODE_SEGMENT_SIZE)
#define SYSTEM_SEGMENT_START (HARDWARE_SEGMENT_START + HARDWARE_SEGMENT_SIZE)

#define KEYBOARD_SEGMENT_START HARDWARE_SEGMENT_START
#define KEYBOARD_SEGMENT_END   HARDWARE_SEGMENT_START + 2_MB 

typedef void (*lambda)();

uint8_t memory[512_MB]{0};

namespace segments {
  static bool keyboard_reserved;
}

namespace manager {
  /* Don't use this namespace ! */
  namespace _ {
    std::vector<std::pair<uint64_t, uint64_t>> regions;
    bool regions_reserved;
  }

  void create_region(uint64_t a, uint64_t b) {
    while (_::regions_reserved) ;
    _::regions_reserved = true;

    for (const auto &pair : _::regions) {
      if (!(b <= pair.first || a >= pair.second)) 
        throw std::logic_error("Thread memory region overlaps with an existing one.");
    }

    _::regions.push_back({a, b});

    _::regions_reserved = false;
  }

  bool is_region_created(uint64_t a) {
    while (_::regions_reserved) ;
    _::regions_reserved = true;

    for (const auto &pair : _::regions) {
      if (a >= pair.first && a < pair.second) {
        _::regions_reserved = false;
        return true;
      }
    }

    _::regions_reserved = false;
    return false;
}

}

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

class core {
  using syscall_t = void(core::*)();
  using setfunc_t = void(core::*)();

private:
  uint64_t beg = 0x0000000000000000;
  uint64_t end = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip  = 0x0000000000000000;
  uint64_t local_ip = 0x0000000000000000;
  reg_t    regs;
  bool     halted = false;
  int      flags  = 0;
  bool     is_system = false;
  uint64_t thread_id = 'U' + 'n' + 'd' + 'e' + 'f' + 'T';
  uint8_t  children = 0;
  std::mutex mtx;
  std::condition_variable cv;
  std::unordered_map<uint64_t, libcallc::DynamicLibrary> libs;
  std::unordered_map<uint64_t, libcallc::DynamicLibrary::FunctionType> funcs;

  uint8_t read() {
    if (ip + 1 >= end) throw std::out_of_range("The 'end' flag is reached.\n"
      "Thread: " + std::to_string(thread_id));
    return memory[ip++];
  }

  template <typename T>
  T read() {
    if (ip + sizeof(T) > end) throw std::out_of_range("The 'end' flag is reached\n"
      "Thread: " + std::to_string(thread_id));

    T value = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
      value |= static_cast<T>(memory[ip + i]) << ((sizeof(T) - 1 - i) * 8);
    }

    ip += sizeof(T);
    return value;
  }

  void imov() {
    regs.set(regs.get(read()), read()); 
  };

  void iadd() {
    auto r1 = read(), r2 = read(); 
    auto v1 = regs.get(r1) + regs.get(r2);
    regs.set(r1, v1);
  };

  void isub() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r1) - regs.get(r2));
  };

  void imul() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r1) * regs.get(r2));
  };

  void idiv() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r1) / regs.get(r2));
  };

  void imod() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r1) % regs.get(r2));
  };

  void ijmp() {
    ip = read<uint64_t>();
  };

  void ije() {
    auto tmp = read<uint64_t>();
    if (flags == 0) ip = tmp;
  };

  void ijne() {
    auto tmp = read<uint64_t>();
    if (flags == 0) return;
    ip = tmp;
  };

  void ijg() {
    auto tmp = read<uint64_t>();
    if (flags == 1) ip = tmp;
  };

  void ijl() {
    auto tmp = read<uint64_t>();
    if (flags == -1) ip = tmp;
  };

  void ijge() {
    auto tmp = read<uint64_t>();
    if (flags == 1 || flags == 0) ip = tmp;
  };

  void ijle() {
    auto tmp = read<uint64_t>();
    if (flags == -1 || flags == 0) ip = tmp;
  };

  void icmp() {
    auto r1 = regs.get(read());
    auto r2 = regs.get(read());
  
    if (r1 > r2) {
      flags = 1;
    } else if (r1 == r2) {
      flags = 0;
    } else {
      flags = -1;
    }
  };

  void iload() {
    auto size = read();
    auto r1 = read();

    switch (size) {
      case 8:  regs.set(r1, read<uint8_t>()); break;
      case 16: regs.set(r1, read<uint16_t>()); break;
      case 32: regs.set(r1, read<uint32_t>()); break;
      case 64: regs.set(r1, read<uint64_t>()); break;
      default: throw std::invalid_argument("in iload(): Invalid size: " + std::to_string(size)
        + "\n\tfetched: [" + std::to_string((int)size) + "]"
          "\n\tThread:  " + std::to_string(thread_id) 
        + "\n\tIP:      " + std::to_string(ip)
        + "\n\tLIP:     " + std::to_string(local_ip)
        + "\n\tARGS:    [" + std::to_string((int)size) + ", " + std::to_string((int)r1) +  "]"); 
      break;
    }
    
  };

  void istore() {
    auto size = read() / 8;
    auto r1 = read();
    auto org = read<uint64_t>();
    auto array = to_bin(regs.get(r1));
    for (uint8_t i = 0; i < size; i++) {
      memory[org+i] = array[i];
    }
    delete[] array;
  };

  void ixint() {
    auto index = read();
    if (index >= SYSCALL_COUNT) {
      throw std::out_of_range("Invalid syscall index\nThread: " + std::to_string(thread_id));
    }
    (this->*syscalls[index])();
  };

  void nop() {};

  void ilea() {
    auto r1 = read();
    auto ad = read<uint64_t>();

    if (ad >= SYSTEM_SEGMENT_START && !is_system) 
      throw std::runtime_error("Permission denied: Accessing system's segment.\n"
      "Thread: " + std::to_string(thread_id));
    
    if (ad >= HARDWARE_SEGMENT_START) {
      while (segments::keyboard_reserved);
      segments::keyboard_reserved = true;
    }
    
    regs.set(r1, ad);

    if (ad >= HARDWARE_SEGMENT_START) segments::keyboard_reserved = false;
  }

  void iloadat() {
    auto dst = read();
    auto at = read<uint64_t>(); /* This function will at memory[at]. */
    
    if (at >= SYSTEM_SEGMENT_START && !is_system) 
    throw std::runtime_error("Permission denied: Accessing system's segment.\n"
    "Thread: " + std::to_string(thread_id));
  
    if (at >= HARDWARE_SEGMENT_START) {
      while (segments::keyboard_reserved);
      segments::keyboard_reserved = true;
    }
    
    regs.set(dst, memory[at]);

    if (at >= HARDWARE_SEGMENT_START) segments::keyboard_reserved = false;
  }

  void iret() { ip = regs.get(63); }

  void imovad() {
    auto a = read(), b = read();
    regs.set(a, memory[regs.get(b)]);
  }

  setfunc_t set[22];

  void swritec() {
    std::putchar((char)regs.get(0));
  }
  
  void swritec_stderr() {
    fputc(static_cast<char>(regs.get(0)), stderr);
    fflush(stderr);
  }

  void sreadc() {
    int c = getchar();
    if (c != EOF) {
      regs.set(0, static_cast<uint8_t>(c));
    } else {
      regs.set(0, 0);
    }
  }

  void scsystem() {
    auto beg = regs.get(48);
    auto len = regs.get(49);

    if (beg + len >= SYSTEM_SEGMENT_START && !is_system) 
      throw std::runtime_error("Permission denied: Accessing system's segment.\n"
        "Thread: " + std::to_string(thread_id)); 

    if (beg + len >= HARDWARE_SEGMENT_START) {
      // Unlock the segment.
      while (segments::keyboard_reserved) ;
      segments::keyboard_reserved = true;
    } else if (beg + len >= end || beg + len <= beg) 
      throw std::runtime_error("Permission denied: Reading out of local segment.\n"
        "Thread: " + std::to_string(thread_id));
    
    std::string buff;
    for (uint64_t i = 0; i < len; i++) 
      buff += (char)memory[beg + i];
    
    regs.set(50, std::system(format(buff).c_str()));
  }

  void sldlib() {
    /* Calling a C-external function.
     If the DynamicLibrary class throws an error, 
     it's the process that will handle this error.
     However, if it's the extern C function that throw the error, 
     in the first case we need to handle this error into this function, 
     and then throw a C++ Exception. */

    std::string lib = "";
    // The pointer is stored in the 48 register.
    auto mybeg = regs.get(48);
    auto len = regs.get(49);

    if (mybeg < beg || mybeg > end) throw std::out_of_range("syscall call C: pointer out of range: " + std::to_string(mybeg));
    if (mybeg + len > end) throw std::out_of_range("Too large string (out of segment): " + std::to_string(mybeg+len));
    
    for (uint64_t i = 0; i < len; i++) lib += memory[mybeg+i];
    
    libs[regs.get(50)].loadLibrary(lib.c_str());
  }
  
  void sldlcfun() {
    auto mybeg = regs.get(48);
    auto len = regs.get(49);
    auto lib = regs.get(50);

    std::string func = "";

    if (mybeg < beg || mybeg > end) throw std::out_of_range("syscall call C: pointer out of range: " + std::to_string(mybeg));
    if (mybeg + len > end) throw std::out_of_range("Too large string (out of segment): " + std::to_string(mybeg+len));
    
    for (uint64_t i = 0; i < len; i++) func += memory[mybeg+i];

    if (libs.find(lib) == libs.end()) {
      std::ostringstream oss;
      oss << "Null pointing dynamic librarie handle: 0x"
          << std::hex << std::uppercase << lib << std::endl
          << "\tThread:\t" << std::dec << thread_id 
          << "\n\tIP:\t\t" << ip 
          << "\n\tLIP:\t\t" << local_ip
          << "\n\tloading:\t" << func << std::endl;
      throw std::runtime_error(oss.str());
    }

    auto funcptr = libs[lib].loadFunction(func.c_str());
    funcs[reinterpret_cast<uint64_t>(funcptr)] = funcptr;
    regs.set(50, reinterpret_cast<uint64_t>(funcptr));
  }

  void suldlib() {
    auto libHandle = regs.get(48);
    
    if ((libs.find(libHandle)) == libs.end()) {
      std::ostringstream oss;
      oss << "Null pointing dynamic librarie handle: 0x"
          << std::hex << std::uppercase << libHandle << std::endl
          << "\tThread:\t" << std::dec << thread_id 
          << "\n\tIP:\t\t" << ip 
          << "\n\tLIP:\t\t" << local_ip << std::endl;
      throw std::runtime_error(oss.str());
    }

    libs[libHandle].unloadLibrary();
  }

  void scfun() {
    auto funHandle = regs.get(48);
    
    if ((funcs.find(funHandle)) == funcs.end()) {
      std::ostringstream oss;
      oss << "Null pointing function handle: 0x"
          << std::hex << std::uppercase << funHandle << std::endl
          << "\tThread:\t" << std::dec << thread_id 
          << "\n\tIP:\t\t" << ip 
          << "\n\tLIP:\t\t" << local_ip << std::endl;
      throw std::runtime_error(oss.str());
    }

    /* Create arguments. */
    while (segments::keyboard_reserved) ;
    segments::keyboard_reserved = true;

    libcallc::arg_t arg{};
    arg.keyboardstart = &memory[KEYBOARD_SEGMENT_START];
    arg.regspointer   = &regs;
    arg.segstart      = &memory[beg];
    arg.seglen        = end - beg;

    funcs[funHandle](&arg);

    segments::keyboard_reserved = false;
  }

  void sstartt() {
    auto beg = regs.get(48);
    auto end = regs.get(49);

    if (beg > end) 
      throw std::logic_error("Cannot create a new thread with given arguments."
        " 'beg' flag is bigger than 'end' flag.\nThread: " + std::to_string(thread_id));

    if (beg < HARDWARE_SEGMENT_START && end > HARDWARE_SEGMENT_START)
      throw std::runtime_error("Thread memory region overlaps with HARDWARE_SEGMENT_START.\n"
        "Thread: " + std::to_string(thread_id));
      
    core* c = new core();
    manager::create_region(beg, end);
    c->init(beg, end, false, end+1);

    std::thread thread([this, c]() mutable {
      {
        std::lock_guard<std::mutex> lock(mtx);
        children++;
      }
      c->run();
      delete c;
      {
        std::lock_guard<std::mutex> lock(mtx);
        children--;
        if (children == 0) {
          cv.notify_all();
        }
      }
    });

    thread.detach();    
  }

  void spseg() {
    auto beg = regs.get(48);
    auto len = regs.get(49);
    auto org = regs.get(50);
    
    if (manager::is_region_created(org)) 
      throw std::runtime_error("Operating on an assigned region: " + std::to_string(org));

    for (uint64_t i = 0; i < len; i++) 
      memory[org+i] = memory[beg+i];
  }

  void sreads() {
    std::string buff;
    std::getline(std::cin, buff);

    while (segments::keyboard_reserved); /* Wait ! */
    segments::keyboard_reserved = true;

    size_t i = 0;

    while (i < buff.size() && KEYBOARD_SEGMENT_START+i < KEYBOARD_SEGMENT_END) { 
      memory[KEYBOARD_SEGMENT_START+i] = buff[i];
      i++;
    }

    segments::keyboard_reserved = false;
    regs.set(50, buff.size());
  }

  syscall_t syscalls[SYSCALL_COUNT] = {
    &core::swritec,
    &core::swritec_stderr,
    &core::sreadc,
    &core::scsystem,
    &core::sldlib,
    &core::sstartt,
    &core::spseg,
    &core::sreads, 
    /* New */
    &core::sldlcfun, 
    &core::suldlib,
    &core::scfun,
  };

public:
  void init(uint64_t _memory_segment_begin, 
            uint64_t _memory_segment_end, 
            bool _is_system = false, 
            uint64_t _name = '?') {
    beg = _memory_segment_begin;
    end = _memory_segment_end;
    ip  = beg;
    is_system = _is_system;
    thread_id = _name;

    set[eins::nop] = &core::nop;
    set[eins::lea] = &core::ilea;
    set[eins::load] = &core::iload;
    set[eins::store] = &core::istore;
    set[eins::mov] = &core::imov;
    set[eins::add] = &core::iadd;
    set[eins::sub] = &core::isub;
    set[eins::mul] = &core::imul;
    set[eins::odiv] = &core::idiv;
    set[eins::mod] = &core::imov;
    set[eins::jmp] = &core::ijmp;
    set[eins::je] = &core::ije;
    set[eins::jne] = &core::ijne;
    set[eins::jl] = &core::ijl;
    set[eins::jg] = &core::ijg;
    set[eins::jle] = &core::ijle;
    set[eins::jge] = &core::ijge;
    set[eins::cmp] = &core::icmp;
    set[eins::xint] = &core::ixint;
    set[eins::loadat] = &core::iloadat;
    set[eins::ret]    = &core::iret;
    set[eins::movad]  = &core::imovad;
  }

  void run() {
    while (!halted) {
      
      if (ip < beg || ip > end) 
        throw std::out_of_range(
          "Reading out of the local segment.\n"
          "\tflag 'beg':\t" + std::to_string(beg) + "\n"
          "\tflag 'end':\t" + std::to_string(end) + "\n"
          "\tIP (global):\t" + std::to_string(ip)
        );
      auto fetched =  read();
      local_ip++;
      
      if (fetched == 0xFF) { halted = true; continue; }

      if (fetched >= sizeof(set) / sizeof(set[0])) {
        std::ostringstream oss;
        oss << "Invalid instruction: " << std::hex << std::uppercase << (int)fetched << "\n"
        "\tfetched:\t[" << (int)fetched<< "]\n"
        "\t4 bytes:\t[" <<  
        format({memory[ip-2],memory[ip-1],memory[ip],memory[ip+1]}, ',') 
        << "]\n"
        "\tCore IP:\t" << ip << std::dec << "\n"
        "\tLocal IP:\t" << local_ip << "\n\tThread: " << thread_id;
        throw std::runtime_error(oss.str());
      }

      (this->*set[fetched])();

      if (beg + 1 >= end) throw std::out_of_range("Reading out of segment.\n"
        "\tThread: " + std::to_string(thread_id));
    }

    {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this] { return children == 0; });
    }
  }
};

std::string version() {
  return "Wyland 1.0";
}

std::string name() {
  return "Orkhon Töresi";
}

void run(std::istream &file) {
  std::cout << version() << " —— " << name() << std::endl;
  
  unsigned int sectors = 0;
  while (!file.eof() && (sectors*512) < SYSTEM_SEGMENT_SIZE) {
    char buffer[512]{0};
    file.read(buffer, sizeof(buffer));

    for (unsigned int i = 0; i < sizeof(buffer); i++) 
      memory[SYSTEM_SEGMENT_START+i] = buffer[i];
    
    sectors++;
  }
  
  core c;
  std::cout << "Preparing..." << std::endl;
  c.init(SYSTEM_SEGMENT_START, SYSTEM_SEGMENT_START + SYSTEM_SEGMENT_SIZE, true, 0);
  std::cout << "Invoking..." << std::endl;
  auto start_exec = std::chrono::steady_clock::now();
  c.run();
  auto end_exec = std::chrono::steady_clock::now();
  std::cout << "Invokation duration:\t" 
  << std::chrono::duration_cast<std::chrono::nanoseconds>(end_exec - start_exec).count()
  << " ns" << std::endl;
}

int main(int argc, char *const argv[]) {
  if (argc <= 1) throw std::invalid_argument("No one task given.");
  std::cout << "Wys's Wyland Virtual Machine" << std::endl;

  for (int i = 1; i < argc; i++) {
    try {
        if (std::string(argv[i]) == "--v") std::cout << version() << std::endl;
        else if (std::string(argv[i]) == "--n") std::cout << name() << std::endl;
        else if (std::string(argv[i]) == "-run") {
          if (i+1 >= argc) throw std::runtime_error("Excepted input file after '-run' token.");
          std::ifstream input(argv[++i]);

          if (!input.is_open()) throw std::invalid_argument("Unable to open file:\t" + std::string(argv[i-1]));

          run(input);
        } else {
          throw std::invalid_argument("Unknown flag:\t" + std::string(argv[i]));
        }
      
    } catch (const std::invalid_argument &e) {
      std::cerr << "[e]:\tinvalid argument\n\twhat():\t" << e.what() << std::endl;;
    } catch (const std::runtime_error &e) {
      std::cerr << "[e]:\truntime error\n\twhat():\t" << e.what() << std::endl;
    } catch (const std::out_of_range &e) {
      std::cerr << "[e]:\tout of range\n\twhat():\t" << e.what() << std::endl;
    } catch (const std::logic_error &e) {
      std::cerr << "[e]:\tlogic error\n\twhat():\t" << e.what() << std::endl;
    } catch (const std::bad_alloc &e) {
      std::cerr << "[e]:\tbad alloc\n\twhat():\t" << e.what() << std::endl;
      std::cerr << "\texecution stopped after a bad allocation." << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[e]:\texception\n\twhat():\t" << e.what() << std::endl;
    } 
  }

  return 0;
}