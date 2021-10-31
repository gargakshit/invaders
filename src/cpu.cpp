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

void CPU::LogicFlagsA() {
  flags.cy = 0;
  flags.ac = 0;
  flags.z = a == 0;
  flags.s = (0x80 == (a & 0x80));
  flags.p = Parity(a, 8);
}

inline uint16_t CPU::GetHL() { return ((uint16_t)h) << 8 | (uint16_t)l; }

inline void CPU::UnimplementedOpcode() { TODO("Unimplemented Opcode"); }

inline uint8_t CPU::GetOperand8_0(uint8_t opcode) {
  // Match with the second nibble
  switch ((opcode >> 3) & 0x7) {
  case 0: return b;
  case 1: return c;
  case 2: return d;
  case 3: return e;
  case 4: return h;
  case 5: return l;
  case 6: return ReadBus(GetHL());
  case 7: return a;
  default: PANIC("Impossible state");
  }
}

inline uint8_t CPU::GetOperand8_1(uint8_t opcode) {
  // Match with the first nibble
  switch (opcode & 0x07) {
  case 0: return b;
  case 1: return c;
  case 2: return d;
  case 3: return e;
  case 4: return h;
  case 5: return l;
  case 6: return ReadBus(GetHL());
  case 7: return a;
  default: PANIC("Impossible state");
  }
}

inline void CPU::SetOperand8_0(uint8_t opcode, uint8_t value) {
  // Match with the first nibble
  switch ((opcode >> 3) & 0x7) {
  case 0: b = value; break;
  case 1: c = value; break;
  case 2: d = value; break;
  case 3: e = value; break;
  case 4: h = value; break;
  case 5: l = value; break;
  case 6: WriteBus(GetHL(), value); break;
  case 7: a = value; break;
  default: PANIC("Impossible state");
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

  case 0x01 ... 0x3f: {
    break;
  }

  // MOV
  // clang-format off
  case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46:
  case 0x47: case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D:
  case 0x4E: case 0x4F: case 0x50: case 0x51: case 0x52: case 0x53: case 0x54:
  case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5A: case 0x5B:
  case 0x5C: case 0x5D: case 0x5E: case 0x5F: case 0x60: case 0x61: case 0x62:
  case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69:
  case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F: case 0x70:
  case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77: case 0x78:
  case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F: {
    // clang-format on
    SetOperand8_0(opcode, GetOperand8_1(opcode));
    break;
  }

  // ADD operand
  // clang-format off
  case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86:
  case 0x87: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a + (uint16_t)GetOperand8_1(opcode);
    ArithFlagsA(res);
    a = res & 0xff;
    break;
  }

  // ADC operand
  // clang-format off
  case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e:
  case 0x8f: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a + (uint16_t)GetOperand8_1(opcode) + flags.cy;
    ArithFlagsA(res);
    a = res & 0xff;
    break;
  }

  // SUB operand
  // clang-format off
  case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96:
  case 0x97: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)GetOperand8_1(opcode);
    ArithFlagsA(res);
    a = res & 0xff;
    break;
  }

  // SBB operand
  // clang-format off
  case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e:
  case 0x9f: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)GetOperand8_1(opcode) - flags.cy;
    ArithFlagsA(res);
    a = res & 0xff;
    break;
  }

  // ANA operand
  // clang-format off
  case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6:
  case 0xa7: {
    // clang-format on
    a &= GetOperand8_1(opcode);
    LogicFlagsA();
    break;
  }

  // XRA operand
  // clang-format off
  case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae:
  case 0xaf: {
    // clang-format on
    a ^= GetOperand8_1(opcode);
    LogicFlagsA();
    break;
  }

  // ORA operand
  // clang-format off
  case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6:
  case 0xb7: {
    // clang-format on
    a |= GetOperand8_1(opcode);
    LogicFlagsA();
    break;
  }

  // CMP operand
  // clang-format off
  case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe:
  case 0xbf: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)GetOperand8_1(opcode);
    ArithFlagsA(res);
    break;
  }

  default: {
    UnimplementedOpcode();
    break;
  }
  }
}
} // namespace invaders
