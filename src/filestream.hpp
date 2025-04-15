#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>

namespace wylma {
  namespace wyland {

    class fstream {
    private:
      std::fstream &stream;
      uint64_t base_address = 0;

    public:
      fstream(std::fstream &base_stream, uint64_t base = 0x0)
        : base_address(base), stream(base_stream) { }

      void open(const std::string &path, std::ios::openmode mode) {
        stream.open(path, mode | std::ios::binary);
      }

      void set_base(uint64_t base) {
        base_address = base;
      }

      bool is_open() const {
        return stream.is_open();
      }

      void close() {
        stream.close();
      }

      void seek(uint64_t offset) {
        stream.seekg(base_address + offset);
        stream.seekp(base_address + offset);
      }

      uint64_t tell() {
        return static_cast<std::streamoff>(stream.tellg()) - base_address;
      }

      void read(char *buffer, std::streamsize size) {
        stream.seekg(base_address + tell());
        stream.read(buffer, size);
      }

      template<typename T>
      void read_typed(T &out) {
        stream.seekg(base_address + tell());
        stream.read(reinterpret_cast<char*>(&out), sizeof(T));
      }

      void write(const char *buffer, std::streamsize size) {
        stream.seekp(base_address + tell());
        stream.write(buffer, size);
      }

      template<typename T>
      void write_typed(const T &in) {
        stream.seekp(base_address + tell());
        stream.write(reinterpret_cast<const char*>(&in), sizeof(T));
      }

      void flush() {
        stream.flush();
      }

      bool eof() const {
        return stream.eof();
      }

      bool good() const {
        return stream.good();
      }

      std::fstream &raw() {
        return stream;
      }

      std::streamsize gcount() { return stream.gcount(); }
    };

  } // namespace wyland
} // namespace wylma
