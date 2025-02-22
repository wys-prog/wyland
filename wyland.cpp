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
    i_nop,   // OK
    i_load,  // OK
    i_store, // OK
    i_lea,   // OK
    i_mov,   // OK 
    i_add,   // OK
    i_sub,   // OK
    i_mul,   // OK
    i_div,   // OK
    i_cmp,   // OK
    i_jmp,   // OK
    i_je,    // OK
    i_jne,   // OK
    i_jg,    // OK
    i_jl,    // OK
    i_jge,   // OK
    i_jle,   // OK
    i_jz,    // OK
    i_jnz,   // OK
    i_xor,   // OK
    i_or,    // OK
    i_and,   // OK
    i_int,   // OK?
    i_call,  // OK
    i_ret,   // OK
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

  void lea(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: r64[unpacked.rv] = memory[unpacked.vv]; break;
      case 1: r64[unpacked.rv] = r64[unpacked.vv]; break;
      default: throw std::runtime_error("Need an argument between 1 and 0"); break;
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

  void opRIMM(const uir_t &unpacked, _fnOP &fn) {
    switch (unpacked.is) {
      case 8:  op::anyme(r8[unpacked.rv], unpacked.vv, fn); break;
      case 16: op::anyme(r16[unpacked.rv], unpacked.vv, fn); break;
      case 32: op::anyme(r32[unpacked.rv], unpacked.vv, fn); break;
      case 64: op::anyme(r64[unpacked.rv], unpacked.vv, fn); break;
      default: throw std::runtime_error("Invalid size."); break;
    }
  }

  void opIMMR(const uir_t &unpacked, _fnOP &fn) {
    switch (unpacked.is) {
      case 8:  op::anyme(memory[unpacked.rv], r8[unpacked.vv], fn); break;
      case 16: op::anyme(memory[unpacked.rv], r16[unpacked.vv], fn); break;
      case 32: op::anyme(memory[unpacked.rv], r32[unpacked.vv], fn); break;
      case 64: op::anyme(memory[unpacked.rv], r64[unpacked.vv], fn); break;
      default: throw std::runtime_error("Invalid size."); break;
    }
  }

  void opRR(const uir_t &unpacked, _fnOP &fn) {
    switch (unpacked.is) {
      case 8:  op::anyme(r8[unpacked.rv], r8[unpacked.vv], fn); break;
      case 16: op::anyme(r16[unpacked.rv], r16[unpacked.vv], fn); break;
      case 32: op::anyme(r32[unpacked.rv], r32[unpacked.vv], fn); break;
      case 64: op::anyme(r64[unpacked.rv], r64[unpacked.vv], fn); break;
      default: throw std::runtime_error("Invalid size."); break;
    }
  }

  void opIMMIMM(const uir_t &unpacked, _fnOP &fn) {
    op::anyme(unpacked.vv, unpacked.rv, fn);
  }

  void mov(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::mov); break;
          case 1: opRIMM(unpacked, op::mov); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::mov); break;
          case 1: opIMMIMM(unpacked, op::mov); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  void add(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::add); break;
          case 1: opRIMM(unpacked, op::add); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::add); break;
          case 1: opIMMIMM(unpacked, op::add); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  void sub(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::sub); break;
          case 1: opRIMM(unpacked, op::sub); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::sub); break;
          case 1: opIMMIMM(unpacked, op::sub); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  void mul(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::mul); break;
          case 1: opRIMM(unpacked, op::mul); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::mul); break;
          case 1: opIMMIMM(unpacked, op::mul); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  void _div(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::div); break;
          case 1: opRIMM(unpacked, op::div); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::div); break;
          case 1: opIMMIMM(unpacked, op::div); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  void mod(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::mod); break;
          case 1: opRIMM(unpacked, op::mod); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::mod); break;
          case 1: opIMMIMM(unpacked, op::mod); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  // We can continue with the other instructions, but I think you've understood the principle.

  void cmp(const uir_t &unpacked) {
    switch (unpacked.is) {
      case 8:  flags_eq = op::compare(r8[unpacked.rv], r8[unpacked.vv]); break;
      case 16: flags_eq = op::compare(r16[unpacked.rv], r16[unpacked.vv]); break;
      case 32: flags_eq = op::compare(r32[unpacked.rv], r32[unpacked.vv]); break;
      case 64: flags_eq = op::compare(r64[unpacked.rv], r64[unpacked.vv]); break;
      default: throw std::runtime_error("Invalid size."); break;
    }
  }

  void jmp(const uir_t&) {
    r64[63] = bytemanip::from_bin<uint64_t>(read(8));
  }

  void je(const uir_t&) {
    auto next = bytemanip::from_bin<uint64_t>(read(8));
    if (flags_zero == 0) r64[63] = next;
  }

  void jne(const uir_t&) {
    auto next = bytemanip::from_bin<uint64_t>(read(8));
    if (flags_eq != 0) r64[63] = next;
  }

  void jz(const uir_t&) {
    auto next = bytemanip::from_bin<uint64_t>(read(8));
    if (flags_zero) r64[63] = next;
  }

  void jnz(const uir_t&) {
    auto next = bytemanip::from_bin<uint64_t>(read(8));
    if (!flags_zero) r64[63] = next;
  }

  void jg(const uir_t&) {
    auto next = bytemanip::from_bin<uint64_t>(read(8));
    if (flags_eq == 1) r64[63] = next;
  }

  void jl(const uir_t&) {
    auto next = bytemanip::from_bin<uint64_t>(read(8));
    if (flags_eq == -1) r64[63] = next;
  }
  
  void jge(const uir_t&) {
    auto next = bytemanip::from_bin<uint64_t>(read(8));
    if (flags_eq == 0 || flags_eq == 1) r64[63] = next;
  }
  
  void jle(const uir_t&) {
    auto next = bytemanip::from_bin<uint64_t>(read(8));
    if (flags_eq == 0 || flags_eq == -1) r64[63] = next;
  }

  void xor_op(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::xor_op); break;
          case 1: opRIMM(unpacked, op::xor_op); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::xor_op); break;
          case 1: opIMMIMM(unpacked, op::xor_op); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  void and_op(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::and_op); break;
          case 1: opRIMM(unpacked, op::and_op); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::and_op); break;
          case 1: opIMMIMM(unpacked, op::and_op); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  void or_op(const uir_t &unpacked) {
    switch (unpacked.rt) {
      case 0: // Destination is a register.
        switch (unpacked.vt) {
          case 0: opRR(unpacked, op::or_op); break;
          case 1: opRIMM(unpacked, op::or_op); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      case 1: // Destination is an address.
        switch (unpacked.vt) {
          case 0: opIMMR(unpacked, op::or_op); break;
          case 1: opIMMIMM(unpacked, op::or_op); break;
          default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
        }
        break;
      default: throw std::invalid_argument("Need an argument between 0 and 1."); break;
    }
  }

  void _int(const uir_t &unpacked) {
    // TODO
  }

  void call(const uir_t &unpacked) {
    // This call function will only call 
    // addresses that stored in r64 registers.
    // And to come back at the current address after,
    // Current pointer will be stored in r64.62 register.
    r64[62] = r64[63];
    r64[63] = r64[unpacked.rv];
  }

  void ret(const uir_t &unpacked) {
    r64[63] = r64[62];
  }

  /* 4 - Mapping functions
    So now, we have ALL THE FUNCTIONS that let us to execute
    a binary program. However.. We didn't code any map.
    Why does we need to "code a map" ? 
    We need to code a map that map (logical..) all instructions 
    with a binary key, to a function. In our case, we'll use 
    hex. key to represent binary, because BINARY IS TOO LONG. */

  std::unordered_map<uint8_t, std::function<void(const uir_t&)>> instruction_set;

  // Ok. Need to initialize the table now.

  void initialize_instruction_set() {
    instruction_set[i_nop] = [](const uir_t&) { /* No operation */ };
    instruction_set[i_load] = load;
    instruction_set[i_store] = store;
    instruction_set[i_lea] = lea;
    instruction_set[i_mov] = mov;
    instruction_set[i_add] = add;
    instruction_set[i_sub] = sub;
    instruction_set[i_mul] = mul;
    instruction_set[i_div] = _div;
    instruction_set[i_cmp] = cmp;
    instruction_set[i_jmp] = jmp;
    instruction_set[i_je] = je;
    instruction_set[i_jne] = jne;
    instruction_set[i_jg] = jg;
    instruction_set[i_jl] = jl;
    instruction_set[i_jge] = jge;
    instruction_set[i_jle] = jle;
    instruction_set[i_jz] = jz;
    instruction_set[i_jnz] = jnz;
    instruction_set[i_xor] = xor_op;
    instruction_set[i_or] = or_op;
    instruction_set[i_and] = and_op;
    instruction_set[i_int] = _int;
    instruction_set[i_call] = call;
    instruction_set[i_ret] = ret;
  }

