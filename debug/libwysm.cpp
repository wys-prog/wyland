#ifndef VM_WRITER_HPP
#define VM_WRITER_HPP

#include <fstream>
#include <vector>
#include <cstdint>

class VMWriter
{
public:
  VMWriter(const std::string &filename) : filename(filename) {}

  void add_instruction(uint8_t instruction, uint8_t size, uint8_t reg_type, uint8_t val_type, uint16_t reg_val, uint16_t val_val)
  {
    uint8_t parameters[8] = {instruction, size, reg_type, val_type,
                             static_cast<uint8_t>(reg_val >> 8), static_cast<uint8_t>(reg_val & 0xFF),
                             static_cast<uint8_t>(val_val >> 8), static_cast<uint8_t>(val_val & 0xFF)};
    instructions.push_back(Instruction{parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5], parameters[6], parameters[7]});
  }

  void write_to_file()
  {
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile)
    {
      throw std::runtime_error("Error opening file for writing: " + filename);
    }
    for (const auto &instr : instructions)
    {
      outfile.write(reinterpret_cast<const char *>(instr.parameters), 8);
    }
    outfile.close();
  }

private:
  struct Instruction
  {
    uint8_t parameters[8];
  };

  std::string filename;
  std::vector<Instruction> instructions;
};

#endif // VM_WRITER_HPP