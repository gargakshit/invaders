#include <stdint.h>

#include "cpu.hpp"
#include "utils.hpp"

namespace invaders {
CPU::CPU(ReadBusFunction readBus, WriteBusFunction writeBus,
         ReadIOFunction readIO, WriteIOFunction writeIO)
    : ReadBus(readBus), WriteBus(writeBus), ReadIO(readIO), WriteIO(writeIO) {}

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
  interrupts = true;
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

void CPU::ArithFlagsA(uint16_t res, bool carry) {
  if (carry) {
    flags.cy = (res > 0xff);
  }
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

inline uint16_t CPU::GetRP(uint8_t opcode) {
  // Match with the first byte
  switch ((opcode >> 4) & 0x3) {
  case 0: return GET_RP(b, c);
  case 1: return GET_RP(d, e);
  case 2: return GET_RP(h, l);
  case 3: return sp;
  default: PANIC("Impossible state");
  }
}

inline void CPU::SetRP(uint8_t opcode, uint16_t value) {
  // Match with the first byte
  switch ((opcode >> 4) & 0x3) {
  case 0: SET_RP(b, c, value); break;
  case 1: SET_RP(d, e, value); break;
  case 2: SET_RP(h, l, value); break;
  case 3: sp = value; break;
  default: PANIC("Impossible state");
  }
}

inline void CPU::SetRP(uint8_t opcode, uint8_t lowByte, uint8_t highByte) {
  // Match with the first byte
  switch ((opcode >> 4) & 0x3) {
  case 0: SET_RP8(b, c, lowByte, highByte); break;
  case 1: SET_RP8(d, e, lowByte, highByte); break;
  case 2: SET_RP8(h, l, lowByte, highByte); break;
  case 3: sp = (uint16_t)lowByte | ((uint16_t)highByte << 8); break;
  default: PANIC("Impossible state");
  }
}

inline bool CPU::BranchCondition(uint8_t opcode) {
  // Match with the first byte
  switch ((opcode >> 3) & 0x7) {
  case 0: return flags.z == 0;
  case 1: return flags.z != 0;
  case 2: return flags.cy == 0;
  case 3: return flags.cy != 0;
  case 4: return flags.p == 0;
  case 5: return flags.p != 0;
  case 6: return flags.s == 0;
  case 7: return flags.s != 0;
  default: PANIC("Impossible state");
  }
}

inline uint16_t CPU::GetStackRP(uint8_t opcode) {
  // Match with the last byte
  switch ((opcode >> 4) & 0x03) {
  case 0: return GET_RP(b, c);
  case 1: return GET_RP(d, e);
  case 2: return GET_RP(h, l);
  case 3: return GET_RP(a, flags.all);
  default: PANIC("Impossible state");
  }
}

inline void CPU::SetStackRP(uint8_t opcode, uint16_t value) {
  // Match with the last byte
  switch ((opcode >> 4) & 0x03) {
  case 0: SET_RP(b, c, value); break;
  case 1: SET_RP(d, e, value); break;
  case 2: SET_RP(h, l, value); break;
  case 3: SET_RP(a, flags.all, value); break;
  default: PANIC("Impossible state");
  }
}

inline void CPU::StackPush(uint16_t data) {
  WriteBus(sp - 1, (data >> 8) & 0xff);
  WriteBus(sp - 2, data & 0xff);
  sp -= 2;
}

inline uint16_t CPU::StackPop() {
  uint16_t ret = (uint16_t)ReadBus(sp) | ((uint16_t)ReadBus(sp + 1) << 8);
  sp += 2;
  return ret;
}

inline uint16_t CPU::GetRSTAddr(uint8_t opcode) {
  switch (opcode) {
  case 0: return 0x0000;
  case 1: return 0x0008;
  case 2: return 0x0010;
  case 3: return 0x0018;
  case 4: return 0x0020;
  case 5: return 0x0028;
  case 6: return 0x0030;
  case 7: return 0x0038;
  default: PANIC("Impossible State");
  }
}

void CPU::ExecuteOpcode() {
  uint8_t opcode = ReadBus(pc);
  pc += 1;

  switch (opcode) {
  // NOP
  case 0x00: break;

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
  } break;

  // ADD operand
  // clang-format off
  case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86:
  case 0x87: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a + (uint16_t)GetOperand8_1(opcode);
    ArithFlagsA(res);
    a = res & 0xff;
  } break;

  // ADC operand
  // clang-format off
  case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e:
  case 0x8f: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a + (uint16_t)GetOperand8_1(opcode) + flags.cy;
    ArithFlagsA(res);
    a = res & 0xff;
  } break;

  // SUB operand
  // clang-format off
  case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96:
  case 0x97: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)GetOperand8_1(opcode);
    ArithFlagsA(res);
    a = res & 0xff;
  } break;

  // SBB operand
  // clang-format off
  case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e:
  case 0x9f: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)GetOperand8_1(opcode) - flags.cy;
    ArithFlagsA(res);
    a = res & 0xff;
  } break;

  // ANA operand
  // clang-format off
  case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6:
  case 0xa7: {
    // clang-format on
    a &= GetOperand8_1(opcode);
    LogicFlagsA();
  } break;

  // XRA operand
  // clang-format off
  case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae:
  case 0xaf: {
    // clang-format on
    a ^= GetOperand8_1(opcode);
    LogicFlagsA();
  } break;

  // ORA operand
  // clang-format off
  case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6:
  case 0xb7: {
    // clang-format on
    a |= GetOperand8_1(opcode);
    LogicFlagsA();
  } break;

  // CMP operand
  // clang-format off
  case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe:
  case 0xbf: {
    // clang-format on
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)GetOperand8_1(opcode);
    ArithFlagsA(res);
  } break;

  // MVI operand,u8
  // clang-format off
  case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x36:
  case 0x3E: {
    // clang-format on
    SetOperand8_0(opcode, ReadBus(pc));
    ++pc;
  } break;

  // INR operand
  // clang-format off
  case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x34:
  case 0x3c: {
    // clang-format on
    uint8_t result = GetOperand8_0(opcode) + 1;
    SetOperand8_0(opcode, result);
    ArithFlagsA(result, false);
  } break;

  // DCR operand
  // clang-format off
  case 0x05: case 0x0d: case 0x15: case 0x1d: case 0x25: case 0x2d: case 0x35:
  case 0x3d: {
    // clang-format on
    uint8_t result = GetOperand8_0(opcode) - 1;
    SetOperand8_0(opcode, result);
    ArithFlagsA(result, false);
  } break;

  // INX operand
  // clang-format off
  case 0x03: case 0x13: case 0x23: case 0x33: {
    // clang-format on
    SetRP(opcode, GetRP(opcode) + 1);
  } break;

  // DCX operand
  // clang-format off
  case 0x0b: case 0x1b: case 0x2b: case 0x3b: {
    // clang-format on
    SetRP(opcode, GetRP(opcode) - 1);
  } break;

  // DAD operand
  // clang-format off
  case 0x09: case 0x19: case 0x29: case 0x39: {
    // clang-format on
    uint16_t val = GetRP(opcode);
    // Use higher precision for easier flag calculation
    uint32_t res = (uint32_t)GetHL() + (uint32_t)val;
    SET_RP(h, l, (uint16_t)res);
    // Set the carry flag
    flags.cy = (res & 0xffff0000) != 0;
  } break;

  // LXI operand,u16
  // clang-format off
  case 0x01: case 0x11: case 0x21: case 0x31: {
    // clang-format on
    SetRP(opcode, ReadBus(pc), ReadBus(pc + 1));
    pc += 2;
  } break;

  // STAX operand
  // clang-format off
  case 0x02: case 0x12: {
    // clang-format on
    WriteBus(GetRP(opcode), a);
  } break;

  // SHLD u16
  case 0x22: {
    uint16_t offset = (uint16_t)ReadBus(pc) | ((uint16_t)ReadBus(pc + 1) << 8);
    WriteBus(offset, l);
    WriteBus(offset + 1, h);
    pc += 2;
  } break;

  // STA u16
  case 0x32: {
    uint16_t offset = (uint16_t)ReadBus(pc) | ((uint16_t)ReadBus(pc + 1) << 8);
    WriteBus(offset, a);
    pc += 2;
  } break;

  // LDAX operand
  // clang-format off
  case 0x0a: case 0x1a: {
    // clang-format on
    a = ReadBus(GetRP(opcode));
  } break;

  // LDHL u16
  case 0x2a: {
    uint16_t offset = (uint16_t)ReadBus(pc) | ((uint16_t)ReadBus(pc + 1) << 8);
    l = ReadBus(offset);
    h = ReadBus(offset + 1);
    pc += 2;
  } break;

  // LDA u16
  case 0x3a: {
    uint16_t offset = (uint16_t)ReadBus(pc) | ((uint16_t)ReadBus(pc + 1) << 8);
    a = ReadBus(offset);
    pc += 2;
  } break;

  // RLC
  case 0x07: {
    uint8_t oldA = a;
    a = ((oldA & 0x80) >> 7) | (oldA << 1);
    flags.cy = (oldA & 0x80) == 0x80;
  } break;

  // RAL
  case 0x17: {
    uint8_t oldA = a;
    a = flags.cy | (oldA << 1);
    flags.cy = (oldA & 0x80) == 0x80;
  } break;

  // DAA
  case 0x27: {
    // Binary coded decimal ugh...
    if ((a & 0xf) > 9) {
      a += 6;
    }
    if ((a & 0xf0) > 0x90) {
      uint16_t res = (uint16_t)a + 0x60;
      a = res & 0xff;
      ArithFlagsA(res);
    }
  } break;

  // STC
  case 0x37: {
    flags.cy = 1;
  } break;

  // RRC
  case 0x0f: {
    uint8_t oldA = a;
    a = ((oldA & 0x1) << 7) | (oldA >> 1);
    flags.cy = (oldA & 0x1) == 0x1;
  } break;

  // RAR
  case 0x1f: {
    uint8_t oldA = a;
    a = (flags.cy << 7) | (oldA >> 1);
    flags.cy = (oldA & 0x1) == 0x1;
  } break;

  // CMA
  case 0x2f: {
    a = ~a;
  } break;

  // CMC
  case 0x3f: {
    // TODO: confirm
    flags.cy = ~flags.cy;
  } break;

  // JUMP condition,u16 (JZ u16, JPE u16, etc)
  // clang-format off
  case 0xc2: case 0xca: case 0xd2: case 0xda: case 0xe2: case 0xea: case 0xf2:
  case 0xfa: {
    // clang-format on
    if (BranchCondition(opcode)) {
      // TODO: confirm
      pc = ((uint16_t)ReadBus(pc + 1) << 8) | (uint16_t)ReadBus(pc);
    } else {
      pc += 2;
    }
  } break;

  // JMP u16
  case 0xc3: {
    // TODO: confirm
    pc = ((uint16_t)ReadBus(pc + 1) << 8) | (uint16_t)ReadBus(pc);
  } break;

  // CALL condition,u16 (CZ u16, CPE u16, etc)
  // clang-format off
  case 0xc4: case 0xcc: case 0xd4: case 0xdc: case 0xe4: case 0xec: case 0xf4:
  case 0xfc: {
    // clang-format on
    if (BranchCondition(opcode)) {
      StackPush(pc + 2);
      pc = ((uint16_t)ReadBus(pc + 1) << 8) | (uint16_t)ReadBus(pc);
    } else {
      pc += 2;
    }
  } break;

  // CALL u16
  case 0xcd: {
    StackPush(pc + 2);
    pc = ((uint16_t)ReadBus(pc + 1) << 8) | (uint16_t)ReadBus(pc);
  } break;

  // RET condition,u16 (RZ u16, RPE u16, etc)
  // clang-format off
  case 0xc0: case 0xc8: case 0xd0: case 0xd8: case 0xe0: case 0xe8: case 0xf0:
  case 0xf8: {
    // clang-format on
    if (BranchCondition(opcode)) {
      pc = StackPop();
    }
  } break;

  // RET u16
  case 0xc9: {
    pc = StackPop();
  } break;

  // PUSH operand
  // clang-format off
  case 0xc5: case 0xd5: case 0xe5: case 0xf5: {
    // clang-format on
    StackPush(GetStackRP(opcode));
  } break;

  // POP operand
  // clang-format off
  case 0xc1: case 0xd1: case 0xe1: case 0xf1: {
    // clang-format on
    SetStackRP(opcode, StackPop());
  } break;

  // ADI u8
  case 0xc6: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a + (uint16_t)ReadBus(pc);
    ++pc;
    ArithFlagsA(res);
    a = res & 0xff;
  } break;

  // ACI u8
  case 0xce: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a + (uint16_t)ReadBus(pc) + flags.cy;
    ++pc;
    ArithFlagsA(res);
    a = res & 0xff;
  } break;

  // SUI u8
  case 0xd6: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)ReadBus(pc);
    ++pc;
    ArithFlagsA(res);
    a = res & 0xff;
  } break;

  // ABI u8
  case 0xde: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)ReadBus(pc) - flags.cy;
    ++pc;
    ArithFlagsA(res);
    a = res & 0xff;
  } break;

  // ANI u8
  case 0xe6: {
    a &= ReadBus(pc);
    ++pc;
    LogicFlagsA();
  } break;

  // XRI u8
  case 0xee: {
    a ^= ReadBus(pc);
    ++pc;
    LogicFlagsA();
  } break;

  // ORI u8
  case 0xf6: {
    a |= ReadBus(pc);
    ++pc;
    LogicFlagsA();
  } break;

  // CPI u8
  case 0xfe: {
    // Use higher precision for easier flag calculation
    uint16_t res = (uint16_t)a - (uint16_t)ReadBus(pc);
    ArithFlagsA(res);
    ++pc;
  } break;

  // RST u8
  // clang-format off
  case 0xc7: case 0xcf: case 0xd7: case 0xdf: case 0xe7: case 0xef: case 0xf7:
  case 0xff: {
    // clang-format on
    StackPush(pc + 2);
    pc = GetRSTAddr(opcode);
  } break;

  // XCHG
  case 0xeb: {
    uint8_t tmp1 = d;
    uint8_t tmp2 = e;
    d = h;
    e = l;
    h = tmp1;
    l = tmp2;
  } break;

  // SPHL
  case 0xf9: {
    sp = GET_RP(h, l);
  } break;

  // XTHL
  case 0xe3: {
    uint16_t tmp = GET_RP(h, l);
    SET_RP(h, l, StackPop());
    StackPush(tmp);
  } break;

  // DI
  case 0xf3: {
    interrupts = false;
  } break;

  // EI
  case 0xfb: {
    interrupts = true;
  } break;

  // OUT d8
  case 0xd3: {
    uint8_t port = ReadBus(pc);
    ++pc;

    if (port >= 8) {
      PANIC("I/O port out of bounds");
    } else {
      WriteIO(port, a);
    }
  } break;

  // IN d8
  case 0xdb: {
    uint8_t port = ReadBus(pc);
    ++pc;

    if (port >= 8) {
      PANIC("I/O port out of bounds");
    } else {
      a = ReadIO(port);
    }
  } break;

  default: {
    UnimplementedOpcode();
  } break;
  }
}
} // namespace invaders
