#include <stdint.h>

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

inline uint16_t CPU::GetHL() { return ((uint16_t)h) << 8 | (uint16_t)l; }

inline void CPU::UnimplementedOpcode() { TODO("Unimplemented Opcode"); }

inline uint8_t CPU::MOV(uint8_t opcode) {
  // Match with the last nibble
  switch (opcode & 0x07) {
  case 0:
    return b;
  case 1:
    return c;
  case 2:
    return d;
  case 3:
    return e;
  case 4:
    return h;
  case 5:
    return l;
  case 6:
    return ReadBus(GetHL());
  case 7:
    return a;
  default:
    PANIC("Invalid opcode for MOV");
  }
}

void CPU::ExecuteOpcode() {
  uint8_t opcode = ReadBus(pc);
  pc += 1;

  switch (opcode) {
  // NOP
  case 0x00: {
    break;
  }

  // MOV B,something
  case 0x40:
  case 0x41:
  case 0x42:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x46:
  case 0x47: {
    b = MOV(opcode);
    break;
  }
  // MOV C,something
  case 0x48:
  case 0x49:
  case 0x4a:
  case 0x4b:
  case 0x4c:
  case 0x4d:
  case 0x4e:
  case 0x4f: {
    c = MOV(opcode);
    break;
  }
  // MOV D,something
  case 0x50:
  case 0x51:
  case 0x52:
  case 0x53:
  case 0x54:
  case 0x55:
  case 0x56:
  case 0x57: {
    d = MOV(opcode);
    break;
  }
  // MOV E,something
  case 0x58:
  case 0x59:
  case 0x5a:
  case 0x5b:
  case 0x5c:
  case 0x5d:
  case 0x5e:
  case 0x5f: {
    e = MOV(opcode);
    break;
  }
  // MOV H,something
  case 0x60:
  case 0x61:
  case 0x62:
  case 0x63:
  case 0x64:
  case 0x65:
  case 0x66:
  case 0x67: {
    h = MOV(opcode);
    break;
  }
  // MOV L,something
  case 0x68:
  case 0x69:
  case 0x6a:
  case 0x6b:
  case 0x6c:
  case 0x6d:
  case 0x6e:
  case 0x6f: {
    l = MOV(opcode);
    break;
  }
  // MOV M,something
  case 0x70:
  case 0x71:
  case 0x72:
  case 0x73:
  case 0x74:
  case 0x75:
  case 0x77: {
    WriteBus(GetHL(), MOV(opcode));
    break;
  }
  // MOV A,something
  case 0x78:
  case 0x79:
  case 0x7a:
  case 0x7b:
  case 0x7c:
  case 0x7d:
  case 0x7e:
  case 0x7f: {
    a = MOV(opcode);
    break;
  }

  default: {
    UnimplementedOpcode();
    break;
  }
  }
}
} // namespace invaders
