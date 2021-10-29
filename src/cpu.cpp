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

inline int CPU::Parity(int x, int size) {
  int p = 0;
  x = (x & ((1 << size) - 1));

  for (int i = 0; i < size; i++) {
    if (x & 0x1) {
      p++;
    }
    x = x >> 1;
  }

  return (0 == (p & 0x1));
}

void CPU::ArithFlagsA(uint16_t res) {
  flags.cy = (res > 0xff);
  flags.z = ((res & 0xff) == 0);
  flags.s = (0x80 == (res & 0x80));
  flags.p = Parity(res & 0xff, 8);
}

inline uint16_t CPU::GetHL() { return ((uint16_t)h) << 8 | (uint16_t)l; }

inline void CPU::UnimplementedOpcode() { TODO("Unimplemented Opcode"); }

inline uint8_t CPU::GetOperand8(uint8_t opcode) {
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
    PANIC("Impossible state");
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

  // MOV B,operand
  case 0x40:
  case 0x41:
  case 0x42:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x46:
  case 0x47: {
    b = GetOperand8(opcode);
    break;
  }
  // MOV C,operand
  case 0x48:
  case 0x49:
  case 0x4a:
  case 0x4b:
  case 0x4c:
  case 0x4d:
  case 0x4e:
  case 0x4f: {
    c = GetOperand8(opcode);
    break;
  }
  // MOV D,operand
  case 0x50:
  case 0x51:
  case 0x52:
  case 0x53:
  case 0x54:
  case 0x55:
  case 0x56:
  case 0x57: {
    d = GetOperand8(opcode);
    break;
  }
  // MOV E,operand
  case 0x58:
  case 0x59:
  case 0x5a:
  case 0x5b:
  case 0x5c:
  case 0x5d:
  case 0x5e:
  case 0x5f: {
    e = GetOperand8(opcode);
    break;
  }
  // MOV H,operand
  case 0x60:
  case 0x61:
  case 0x62:
  case 0x63:
  case 0x64:
  case 0x65:
  case 0x66:
  case 0x67: {
    h = GetOperand8(opcode);
    break;
  }
  // GetOperand8 L,operand
  case 0x68:
  case 0x69:
  case 0x6a:
  case 0x6b:
  case 0x6c:
  case 0x6d:
  case 0x6e:
  case 0x6f: {
    l = GetOperand8(opcode);
    break;
  }
  // MOV M,operand
  case 0x70:
  case 0x71:
  case 0x72:
  case 0x73:
  case 0x74:
  case 0x75:
  case 0x77: {
    WriteBus(GetHL(), GetOperand8(opcode));
    break;
  }
  // MOV A,operand
  case 0x78:
  case 0x79:
  case 0x7a:
  case 0x7b:
  case 0x7c:
  case 0x7d:
  case 0x7e:
  case 0x7f: {
    a = GetOperand8(opcode);
    break;
  }

  // ADD operand
  case 0x80:
  case 0x81:
  case 0x82:
  case 0x83:
  case 0x84:
  case 0x85:
  case 0x86:
  case 0x87: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a + (uint16_t)GetOperand8(opcode);
    ArithFlagsA(res);
    a = res & 0xff;
    break;
  }

  // ADC operand
  case 0x88:
  case 0x89:
  case 0x8a:
  case 0x8b:
  case 0x8c:
  case 0x8d:
  case 0x8e:
  case 0x8f: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a + (uint16_t)GetOperand8(opcode) + flags.cy;
    ArithFlagsA(res);
    a = res & 0xff;
    break;
  }

  // SUB operand
  case 0x90:
  case 0x91:
  case 0x92:
  case 0x93:
  case 0x94:
  case 0x95:
  case 0x96:
  case 0x97: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)GetOperand8(opcode);
    ArithFlagsA(res);
    a = res & 0xff;
    break;
  }

  // SBB operand
  case 0x98:
  case 0x99:
  case 0x9a:
  case 0x9b:
  case 0x9c:
  case 0x9d:
  case 0x9e:
  case 0x9f: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)GetOperand8(opcode) - flags.cy;
    ArithFlagsA(res);
    a = res & 0xff;
    break;
  }

  default: {
    UnimplementedOpcode();
    break;
  }
  }
}
} // namespace invaders
