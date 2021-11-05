#include <stdint.h>

#include "cpu.hpp"

namespace invaders {
#pragma once
enum KeyboardState {
  LEFT = 0b0010'0000,
  RIGHT = 0b0100'0000,
};

#pragma once
class Bus {
  // Keep the full addressable range as memory for now
  uint8_t mem[1 << 16] = {0};

  void WriteMem(uint16_t addr, uint8_t data);
  uint8_t ReadMem(uint16_t addr);

  void WriteIO(uint8_t port, uint8_t data);
  uint8_t ReadIO(uint8_t port);

  // Shift register state
  uint16_t shift0;
  uint16_t shift1;
  uint16_t shiftOffset;

  uint8_t port1 = 0;

public:
  CPU cpu;

  bool LoadFileAt(std::string path, uint16_t start, bool cpm = false);

  // CPU
  void Reset();
  void TickCPU();

  // IO
  void SetKeyboardState(KeyboardState state, bool pressed);

  Bus();
};
} // namespace invaders
