#include <functional>
#include <stdint.h>

#include "config.h"

// Returns the register pair (a, b)
#define GET_RP(a, b) (((uint16_t)a << 8) | (uint16_t)b)

// Set the register pair (a, b) with (val) in little-endian
#define SET_RP(a, b, val)                                                      \
  do {                                                                         \
    uint16_t _res = val;                                                       \
    a = _res >> 8;                                                             \
    b = _res & 0xFF;                                                           \
  } while (0)

// Set the register pair (a, b) with (val1, val2) in little-endian
#define SET_RP8(a, b, lowByte, highByte)                                       \
  do {                                                                         \
    a = highByte;                                                              \
    b = lowByte;                                                               \
  } while (0)

namespace invaders {
typedef std::function<uint8_t(uint16_t)> ReadBusFunction;
typedef std::function<void(uint16_t, uint8_t)> WriteBusFunction;

typedef std::function<uint8_t(uint8_t)> ReadIOFunction;
typedef std::function<void(uint8_t, uint8_t)> WriteIOFunction;

#pragma once
class CPU {
  // Stack pointer
  uint16_t sp;

  // Registers
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;
  uint8_t h;
  uint8_t l;

  // Flags
  struct {
    union {
      struct {
        // Carry
        uint8_t cy : 1;
        // Padding
        uint8_t pad : 1;
        // Parity
        uint8_t p : 1;
        // Moar padding
        uint8_t pad2 : 1;
        // Auxilary Carry
        uint8_t ac : 1;
        // Even moar padding
        uint8_t pad3 : 1;
        // Zero
        uint8_t z : 1;
        // Sign
        uint8_t s : 1;
      };

      // All flags
      uint8_t all;
    };
  } flags;

  // Utility functions for register pairs. Inline'd for the best performance
  inline uint16_t GetHL();

  // Check the parity of an input
  inline int Parity(int x, int size);
  // Calculate the flags for arithmetic operation on acc. Pass the 2nd argument
  // as false to not change the carry flag
  void ArithFlagsA(uint16_t res, bool carry = true);
  // Calculate the flags for logic operation on acc
  void LogicFlagsA();

  // Bus operations
  ReadBusFunction ReadBus;
  WriteBusFunction WriteBus;
  // IO operations
  ReadIOFunction ReadIO;
  WriteIOFunction WriteIO;

  inline void UnimplementedOpcode(uint8_t opcode);

  // It returns the first operand by decoding the opcode (opcode >> 3) & 0x7)
  inline uint8_t GetOperand8_0(uint8_t opcode);
  // It returns the second operand by decoding the opcode (opcode & 0x07)
  inline uint8_t GetOperand8_1(uint8_t opcode);
  // It sets the first operand by decoding the opcode (opcode >> 3) & 0x7)
  inline void SetOperand8_0(uint8_t opcode, uint8_t value);
  // It returns the 16-bit operand (register pair) from the opcode
  // `((opcode >> 4) & 0x3)`
  inline uint16_t GetRP(uint8_t opcode);
  // It sets the 16-bit operand (register pair) from the opcode
  // `((opcode >> 4) & 0x3)`
  inline void SetRP(uint8_t opcode, uint16_t value);
  // It sets the 16-bit operand (register pair) from the opcode
  // `((opcode >> 4) & 0x3)` encoded using little-endian
  inline void SetRP(uint8_t opcode, uint8_t value1, uint8_t value2);
  // It checks the branch condition based on the opcode `((opcode >> 3) & 0x7)`
  inline bool BranchCondition(uint8_t opcode);
  // It returns the 16-bit operand (register pair) from the opcode for `PUSH`.
  // `((opcode >> 4) & 0x3)`
  inline uint16_t GetStackRP(uint8_t opcode);
  // It returns the 16-bit operand (register pair) from the opcode for `POP`.
  // `((opcode >> 4) & 0x3)`
  inline void SetStackRP(uint8_t opcode, uint16_t value);
  // It returns the 16-bit address from the opcode for `RST`.
  // `((opcode >> 3) & 0x7)`
  inline uint16_t GetRSTAddr(uint8_t opcode);

  // TODO: confirm stack ops
  inline void StackPush(uint16_t data);
  inline uint16_t StackPop();

  uint8_t cycles[256] = {
      4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  4,  10,
      7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 16, 5,
      5,  5,  7,  4,  4,  10, 16, 5,  5,  5,  7,  4,  4,  10, 13, 5,  10, 10,
      10, 4,  4,  10, 13, 5,  5,  5,  7,  4,  5,  5,  5,  5,  5,  5,  7,  5,
      5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  5,  5,
      5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,
      5,  5,  7,  5,  7,  7,  7,  7,  7,  7,  7,  7,  5,  5,  5,  5,  5,  5,
      7,  5,  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
      4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,
      4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,
      4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  11, 10, 10, 10, 17, 11,
      7,  11, 11, 10, 10, 10, 10, 17, 7,  11, 11, 10, 10, 10, 17, 11, 7,  11,
      11, 10, 10, 10, 10, 17, 7,  11, 11, 10, 10, 18, 17, 11, 7,  11, 11, 5,
      10, 5,  17, 17, 7,  11, 11, 10, 10, 4,  17, 11, 7,  11, 11, 5,  10, 4,
      17, 17, 7,  11,
  };

  uint8_t pendingCycles = 0;

  void ExecuteOpcode(uint8_t opcode);

public:
  CPU(ReadBusFunction, WriteBusFunction, ReadIOFunction, WriteIOFunction);

  // Program counter
  uint16_t pc;

  // Last opcode
  uint8_t opcode;

  // Are interrupts enabled
  bool interrupts = true;

  void Reset();
  void Tick();
  void Interrupt(uint8_t vector);
};
} // namespace invaders
