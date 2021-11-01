#include <functional>
#include <stdint.h>

// Returns the register pair (a, b)
#define GET_RP(a, b) (((uint16_t)a << 8) | (uint16_t)b)

// Set the register pair (a, b) with (val) in little-endian
#define SET_RP(a, b, val)                                                      \
  do {                                                                         \
    a = val >> 8;                                                              \
    b = val & 0xFF;                                                            \
  } while (0)

// Set the register pair (a, b) with (val1, val2) in little-endian
#define SET_RP8(a, b, val1, val2)                                              \
  do {                                                                         \
    a = val2;                                                                  \
    b = val1;                                                                  \
  } while (0)

namespace invaders {
typedef std::function<uint8_t(uint16_t)> ReadFunction;
typedef std::function<void(uint16_t, uint8_t)> WriteFunction;

#pragma once
class CPU {
  // Program counter
  uint16_t pc;

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
    // Zero
    uint8_t z : 1;
    // Sign
    uint8_t s : 1;
    // Parity
    uint8_t p : 1;
    // Carry
    uint8_t cy : 1;
    // Auxilary Carry
    uint8_t ac : 1;
    // Padding the extra bytes :)
    uint8_t pad : 3;
  } flags;

  // Are interrupts enabled
  bool interrupts;

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
  ReadFunction ReadBus;
  WriteFunction WriteBus;

  inline void UnimplementedOpcode();

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

  void ExecuteOpcode();

public:
  CPU(ReadFunction, WriteFunction);

  void Reset();
  void Tick();
};
} // namespace invaders
