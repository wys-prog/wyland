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
  uint8_t memory[MEMORY_SIZE];

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
    The r64-63. This register is used as IP, Instruction Pointer.
    */

  uint8_t  r8[8];
  uint16_t r16[16];
  uint32_t r32[32];
  uint64_t r64[64];

  /* 2 - SIMDs. They are the largest registers, 128 bits. 
     To implement them, we need the <stdfloat> header file, 
     included in the 23 standard of the C++. */

  #include <stdfloat>
  std::float128_t SIMD128[8];
  std::float64_t  SIMD64[8];
  std::float32_t  SIMD32[8];

/* III - Instructions
  In x87 instructions takes 8 bytes.
  So for instructions, we'll use @uint64_t.
  Here's how 8 bytes are organized:

 +--------------+-------------+------+---------------+------------+----------+-------+
 | size in byte |      1      |  1   |      1        |      1     |    2     |   2   |
 +--------------+-------------+------+---------------+------------+----------+-------+
 | parameter    | instruction | size | register-type | value-type | register | value |
 +--------------+-------------+------+---------------+------------+----------|-------+
 | address      |     0x00    | 0x01 |      0x02     |    0x03    |   0x04   |  0x05 |
 |+-------------+-------------+------+---------------+------------+----------+-------+
  */
  

