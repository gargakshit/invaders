#include <iostream>

#include "config.h"

#ifndef DISABLE_TRACE
#define TRACE(msg)                                                             \
  std::cerr << "TRACE: " << msg << std::endl                                   \
            << "File: " << __FILE__ << std::endl                               \
            << "Line: " << __LINE__ << std::endl;
#else
#define TRACE(msg) ;
#endif

#define TODO(msg)                                                              \
  std::cerr << "TODO: " << msg << std::endl                                    \
            << "File: " << __FILE__ << std::endl                               \
            << "Line: " << __LINE__ << std::endl                               \
            << "Aborting..." << std::endl;                                     \
  exit(1);

#define PANIC(msg)                                                             \
  std::cerr << "PANIC: " << msg << std::endl                                   \
            << "File: " << __FILE__ << std::endl                               \
            << "Line: " << __LINE__ << std::endl                               \
            << "Aborting..." << std::endl;                                     \
  exit(1);
