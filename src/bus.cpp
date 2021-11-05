#include <fstream>
#include <functional>
#include <iostream>
#include <stdint.h>

#include "bus.hpp"
#include "cpu.hpp"

namespace invaders {
Bus::Bus()
    : cpu(std::bind(&Bus::ReadMem, this, std::placeholders::_1),
          std::bind(&Bus::WriteMem, this, std::placeholders::_1,
                    std::placeholders::_2),
          std::bind(&Bus::ReadIO, this, std::placeholders::_1),
          std::bind(&Bus::WriteIO, this, std::placeholders::_1,
                    std::placeholders::_2)) {}

void Bus::WriteMem(uint16_t addr, uint8_t data) { mem[addr] = data; }
uint8_t Bus::ReadMem(uint16_t addr) { return mem[addr]; }

// IO not implemented (yet)
void Bus::WriteIO(uint8_t port, uint8_t data) {
  switch (port) {
  // Shift register
  case 2: {
    shiftOffset = data & 0x07;
  } break;

  // Shift register
  case 4: {
    shift0 = shift1;
    shift1 = data;
  } break;

  // Black-hole all other writes
  default: break;
  }
}

uint8_t Bus::ReadIO(uint8_t port) {
  switch (port) {
  case 0: return 1;

  case 1: return port1;

  // Shift register
  case 3: {
    uint16_t v = (shift1 << 8) | shift0;
    return (v >> (8 - shiftOffset)) & 0xff;
  }

  // Return 0 on all other ports
  default: return 0;
  }
}

bool Bus::LoadFileAt(std::string path, uint16_t start, bool diag) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    std::cerr << "Unable to load file \"" << path << "\", skipping..."
              << std::endl;
    return false;
  }

  int i = 0;
  char b;

  while (file.get(b)) {
    mem[i + start] = b;
    ++i;
  }

  file.close();

  // CP/M CPU diagnostics program
  if (diag) {
    // Patch the program to jump to 0x0100
    mem[0] = 0xc3;
    mem[1] = 0;
    mem[2] = 0x01;

    // Fix the stack pointer from 0x6ad to 0x7ad
    // this 0x06 byte 112 in the code, which is
    // byte 112 + 0x100 = 368 in memory
    mem[368] = 0x7;

    // Skip DAA test
    mem[0x59c] = 0xc3; // JMP
    mem[0x59d] = 0xc2;
    mem[0x59e] = 0x05;
  }

  return true;
}

void Bus::Reset() {
  port1 = 0;
  cpu.Reset();
}

void Bus::TickCPU() { cpu.Tick(); }

void Bus::SetKeyboardState(KeyboardState state, bool pressed) {
  if (pressed) {
    port1 |= state;
  } else {
    port1 &= ~state;
  }
}
} // namespace invaders
