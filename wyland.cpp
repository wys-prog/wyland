/* For a best exception caught, include: */
#include <stdexcept>
#include <new> /* To caught 'bad_alloc' */

/* Defining some constants */
#define MEMORY_SIZE 1024 * 1024 * 512

/* We need stdint.h file, to get uint8 and other int types. */
#include <cstdint>

/* I - Memory 
  Memory is divided in 3 segments.
  The first one (and the biggest) is the code segment.
  In this segment, we can find instructions, and data.
  The second one is the hardware segments. Used to communicate 
  with the virtual hardware.
  And the last one, the smallest, is the System's segment.
  Used for the stack, or other system componant. */
  uint8_t memory[MEMORY_SIZE] {0};

  /* Memory addressing:
    0x00000000 - 0x12C00000 : Code segment (~300Mo)
    0x12C00000 - 0x1F400000 : External hardware segment (~200Mo)
    0x1F400000 - 0x20000000 : System segment (~12Mo)
  */

  // Define segment sizes
  #define CODE_SEGMENT_SIZE (300 * 1024 * 1024)
  #define EXTERNAL_HARDWARE_SEGMENT_SIZE (200 * 1024 * 1024)
  #define SYSTEM_SEGMENT_SIZE (MEMORY_SIZE - CODE_SEGMENT_SIZE - EXTERNAL_HARDWARE_SEGMENT_SIZE)

  // Define segment start addresses
  #define CODE_SEGMENT_START 0x00000000
  #define EXTERNAL_HARDWARE_SEGMENT_START (CODE_SEGMENT_START + CODE_SEGMENT_SIZE)
  #define SYSTEM_SEGMENT_START (EXTERNAL_HARDWARE_SEGMENT_START + EXTERNAL_HARDWARE_SEGMENT_SIZE)

  // Define segment pointers
  uint8_t* code_segment = memory + CODE_SEGMENT_START;
  uint8_t* external_hardware_segment = memory + EXTERNAL_HARDWARE_SEGMENT_START;
  uint8_t* system_segment = memory + SYSTEM_SEGMENT_START;

/* II - Registers 
  Registers are something like a "global variable".
  They have a specific size, and can be used to calculate 
  or manipulate data. */

  /* 1 - General registers. 
    All general registers are "free", excepted one 64 registers.
    The r64-63. This register is used as IP, Instruction Pointer. */

  uint8_t  r8[8]   {0};
  uint16_t r16[16] {0};
  uint32_t r32[32] {0};
  uint64_t r64[64] {0};

  /* 2 - SIMDs. They are the largest registers, 128 bits. 
     To implement them, we need the <stdfloat> header file, 
     included in the 23 standard of the C++. */

  #include <stdfloat>
  std::float128_t SIMD128[8] {0};
  std::float64_t  SIMD64[8]  {0};
  std::float32_t  SIMD32[8]  {0};

  /* 3 - FLAGS. They are a bit different from the x86.
    They are at the count of 3. */
  
  char flags_zero; /* 1 if it's 0 */
  char flags_flow; /* 1: Overflow, -1: Underflow, 0: Nice */
  char flags_eq;   /* 1: NEQ, 0: EQ */

  // Read function that let us to read. (Wow.)
  uint8_t *read(uint64_t count, uint64_t from = r64[63]) {
    if (from >= MEMORY_SIZE || count >= MEMORY_SIZE) 
      throw std::out_of_range("Reading out of memory capacity.");
    uint8_t *buff = new uint8_t[count];
    for (uint64_t i = 0; i < count; i++) 
      buff[i] = memory[from+i];
    return buff;
  }
/* III - Instructions
  In x87 instructions takes 8 bytes.
  So for instructions, we can use @uint64_t, but it can 
  "complicate" processing with reading from a file, casting etc.
  Because of this, we'll use the type @ir_t. We'll define it later.
  Here's how the 8 bytes are organized:

 +--------------+-------------+------+---------------+------------+----------+-------+
 | size in byte |      1      |  1   |       1       |      1     |    2     |   2   |
 +--------------+-------------+------+---------------+------------+----------+-------+
 | parameter    | instruction | size | register-type | value-type | register | value |
 +--------------+-------------+------+---------------+------------+----------|-------+
 | address      |     0x00    | 0x01 |      0x02     |    0x03    |   0x04   |  0x05 |
 |+-------------+-------------+------+---------------+------------+----------+-------+
  */
  
  /* 1 - Type */
  typedef struct {
    uint8_t parameters[8];

    uint8_t &operator[](int i) {
      if (i >= 8) throw std::out_of_range("too large index");
      return parameters[i];
    }
  } ir_t; // Instruction Representation Type.

  /* 2 - Instruction set 
    Instruction set is simply a set, wich contains all 
    supported instructions. 
    Notice: instructions are enumerated in a uint8_t 
    because the instruction is only 1 byte (see the higher example). 
    If we give an instruction that's not present in this set, 
    the core will just throw an error. */
  enum instructions : uint8_t {
    i_nop,
    i_load, 
    i_store,
    i_lea,
    i_mov, 
    i_add, 
    i_sub, 
    i_mul, 
    i_div, 
    i_cmp, 
    i_jmp, 
    i_je, 
    i_jne, 
    i_jg, 
    i_jl, 
    i_jge, 
    i_jle, 
    i_jz, 
    i_jnz, 
    i_xor,
    i_or,  
    i_and,
    i_int, 
    i_call, 
    i_ret,
  };

  /* 3 - Unpacking arguments
    Because instructions are in 8 bytes, we need 
    to "unpack" them. */

  typedef struct {
    uint8_t  in; // Instruction
    uint8_t  is; // Instruction Size
    uint8_t  rt; // Register Type
    uint8_t  vt; // Value Type
    uint16_t rv; // Register Value
    uint16_t vv; // Value Value
  } uir_t;

  ir_t fetch() {    // fetch() is called to get a ir_t, and inc. the IP.
    ir_t ir{0x00};  // After fetching, we need to unpack arguments.
    for (char i = 0; i < 8; i++) 
      ir[i] = read(1, r64[63]++)[0];
    return ir;
  }

  uir_t unpack(const ir_t &ir) {    // Fetched. Now, unpacking.
    uir_t unpacked{0};              // Pretty simple, because 
    unpacked.in = ir.parameters[0]; // We use an array instead 
    unpacked.is = ir.parameters[1]; // an uint64.
    unpacked.rt = ir.parameters[2];
    unpacked.vt = ir.parameters[3];
    unpacked.rv |= (ir.parameters[4] << 8) | ir.parameters[5];
    unpacked.vv |= (ir.parameters[6] << 8) | ir.parameters[7];
    return unpacked;
  }

  /* To manipulate bytes, we need the bytes.hpp file. */
  #include "bytes.hpp"

  void load(const uir_t &unpacked) {
    switch (unpacked.vt) {
      case 1: // Value is embeeded.
        switch (unpacked.is) {
          case 8:  r8[unpacked.rv]  = unpacked.vv; break;
          case 16: r16[unpacked.rv] = unpacked.vv; break;
          case 32: r32[unpacked.rv] = unpacked.vv; break;
          case 64: r64[unpacked.rv] = unpacked.vv; break;
          default: throw std::runtime_error("Invalid operand size."); break;
        }
        break;
      case 0: // Value isn't embeeded.
        switch (unpacked.is) {
          case 8:  r8[unpacked.rv]  = bytemanip::from_bin<uint8_t>(read(1)); break;
          case 16: r16[unpacked.rv] = bytemanip::from_bin<uint16_t>(read(2)); break;
          case 32: r32[unpacked.rv] = bytemanip::from_bin<uint32_t>(read(4)); break;
          case 64: r64[unpacked.rv] = bytemanip::from_bin<uint64_t>(read(8)); break;
          default: throw std::runtime_error("Invalid operand size."); break;
        }
        break;
      default: throw std::runtime_error("Invalid type."); break;
    }
  }

  void store(const uir_t &unpacked) {
    // unpacked.vt cannot be embeeded, it's addresses.
    if (unpacked.vt) throw std::runtime_error("Invalid type.");

    switch (unpacked.is) {
      case 8:  memory[unpacked.vv] = r8[unpacked.rv]; break;
      case 16: bytemanip::fill(memory, r16[unpacked.rv], 512); break;
      case 32: bytemanip::fill(memory, r32[unpacked.rv], 512); break;
      case 64: bytemanip::fill(memory, r64[unpacked.rv], 512); break;
      default: throw std::runtime_error("Invalid size."); break;
    }
  }

  /* Basic instructions like MOV, ADD, etc. can be really
    LONG to code in a single function. To reduce the line 
    of code, we'll use two other libraries, 
    <functionnal> (to get std::function<T(...)>) and 
    "operatirs.hpp", to get basic functions. */
  
    #include <functional>
    #include "operators.hpp"
  
  /* Now, let's create some generic functions. */

  template <typename TyA, typename TyB>
  void opRIMM(const uir_t &unpacked, std::function<void(TyA&, TyB&)> &fn) {
    switch (unpacked.is) {
      case 8:  fn(r8[unpacked.rv], unpacked.vv); break;
      case 16: fn(r16[unpacked.rv], unpacked.vv); break;
      case 32: fn(r32[unpacked.rv], unpacked.vv); break;
      case 64: fn(r64[unpacked.rv], unpacked.vv); break;
      default: throw std::runtime_error("Invalid size."); break;
    }
  }

  template <typename TyA, typename TyB>
  void opIMMR(const uir_t &unpacked, std::function<void(TyA&, TyB&)> &fn) {
    switch (unpacked.is) {
      case 8:  fn(unpacked.vv, r8[unpacked.rv]); break;
      case 16: fn(unpacked.vv, r16[unpacked.rv]); break;
      case 32: fn(unpacked.vv, r32[unpacked.rv]); break;
      case 64: fn(unpacked.vv, r64[unpacked.rv]); break;
      default: throw std::runtime_error("Invalid size."); break;
    }
  }

  template <typename TyA, typename TyB>
  void opRR(const uir_t &unpacked, std::function<void(TyA&, TyB&)> &fn) {
    switch (unpacked.is) {
      case 8:  fn(r8[unpacked.vv], r8[unpacked.rv]); break;
      case 16: fn(r16[unpacked.vv], r16[unpacked.rv]); break;
      case 32: fn(r32[unpacked.vv], r32[unpacked.rv]); break;
      case 64: fn(r64[unpacked.vv], r64[unpacked.rv]); break;
      default: throw std::runtime_error("Invalid size."); break;
    }
  }

  template <typename TyA, typename TyB>
  void opIMMIMM(const uir_t &unpacked, std::function<void(TyA&, TyB&)> &fn) {
    fn(unpacked.vv, unpacked.rv);
  }

  void mov(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: break;
          case 1: break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: break;
          case 1: break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }