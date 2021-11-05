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
void Bus::WriteIO(uint8_t port, uint8_t data) {}
uint8_t Bus::ReadIO(uint8_t port) { return 0; }

bool Bus::LoadFileAt(std::string path, uint16_t start, bool cpm) {
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

  // CP/M program
  if (cpm) {
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

void Bus::ResetCPU() { cpu.Reset(); }
void Bus::TickCPU() { cpu.Tick(); }
} // namespace invaders
