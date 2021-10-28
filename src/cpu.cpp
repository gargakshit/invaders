#include "cpu.hpp"
#include "utils.hpp"

namespace invaders {
CPU::CPU(ReadFunction read, WriteFunction write)
    : ReadBus(read), WriteBus(write) {}

void CPU::Reset() {
  pc = 0;
  sp = 0;
  a = 0;
  b = 0;
  c = 0;
  d = 0;
  e = 0;
  h = 0;
  l = 0;
}

inline void CPU::UnimplementedOpcode() { TODO("Unimplemented Opcode"); }
} // namespace invaders
