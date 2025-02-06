#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <functional>
#include <unordered_map>

/* This compiler is really simple. It's a JIT compiler. */

class compiler {
public:
  /* Types */

  /* Type to represent a label */
  typedef struct {
    std::string name;
    uint64_t address;
  } label;
private:
  std::ostream &output; /* Output stream */
  std::istream &stream; /* Input stream */
  uint64_t current_address = 0x0000000000000000;

  void _mov(int dst, int src) {
    
  }

public:
};

/* Code example: 
    mov $1, 0
    mov $2, 98
    add $1, $2 ; Will result to 98 (0 + 98)
    store $1, 0x00

    ; label declaration
    lbl main:
      ; code
      ret ; end of code. 

    example: 

    lbl buff: 
      resb(8) ; reserve 8bytes.

    lbl main: 
      mov $0, 5
      add $0, $0 ; = 10
      
 */