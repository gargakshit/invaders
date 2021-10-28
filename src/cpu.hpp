#include <functional>
#include <stdint.h>

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

  ReadFunction ReadBus;
  WriteFunction WriteBus;

  inline void UnimplementedOpcode();

  // MOV gets the value from the registers / memory as defined by the opcode and
  // then returns it. It is upto the callee to assign it
  inline uint8_t MOV(uint8_t op);

  void ExecuteOpcode();

public:
  CPU(ReadFunction, WriteFunction);

  void Reset();
  void Tick();
};
} // namespace invaders
