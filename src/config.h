#ifndef _INVADERS_CONFIG_H
#define _INVADERS_CONFIG_H
// CPU diagnostics build. Requires a CPU diagnostics binary present at
// `tmp/cpudiag.bin`
// This also enables CP/M prints and exists
// #define CPUDIAG

// Disable all prints for debugging. This does not disable PANIC, TODO and TRACE
// macros
#define PRINT_DISABLE_ALL

// Disables tracing
#define DISABLE_TRACE

#ifndef PRINT_DISABLE_ALL
// Prints CPU execution logs to stdout
#define PRINT_CPU_STATUS
// Prints external interrupts to stdout
#define PRINT_INTERRUPTS
// Prints memory writes to stdout
#define PRINT_MEM_WRITES
#endif

#ifdef CPUDIAG
#pragma message("Building for CPU diagnostics")
#define CPM_EMU
#endif

#ifdef CPM_EMU
#pragma message("Building with CP/M print and exit emulation")
#endif

#ifdef PRINT_CPU_STATUS
#pragma message(                                                               \
    "Building with CPU execution logging to stdout. It is recommended to "     \
    "redirect the log to a file as it is very verbose")
#endif

#ifdef PRINT_INTERRUPTS
#pragma message("Building with external interrupt logging to stdout")
#endif

#ifdef PRINT_MEM_WRITES
#pragma message("Building with memory write logging to stdout")
#endif

#endif
