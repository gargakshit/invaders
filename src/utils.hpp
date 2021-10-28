#include <iostream>

#define TODO(msg)                                                              \
  std::cerr << "TODO: " << msg << std::endl << "Aborting..." << std::endl;     \
  exit(1);

#define PANIC(msg)                                                             \
  std::cerr << "PANIC: " << msg << std::endl << "Aborting..." << std::endl;    \
  exit(1);
