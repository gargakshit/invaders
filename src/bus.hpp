#include <stdint.h>

#include "cpu.hpp"

namespace invaders {
#pragma once
class Bus {
  // Keep the full addressable range as memory for now
  uint8_t mem[(1 << 16) - 1] = {0};

  void WriteMem(uint16_t, uint8_t);
  uint8_t ReadMem(uint16_t);

  void WriteIO(uint8_t, uint8_t);
  uint8_t ReadIO(uint8_t);

public:
  CPU cpu;

  bool LoadFileAt(std::string path, uint16_t start, bool cpm = false);

  // CPU
  void ResetCPU();
  void TickCPU();

  Bus();
};
} // namespace invaders
