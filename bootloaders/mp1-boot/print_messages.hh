#pragma once
#include "print.hh"

/////////////////////////////////////////////
// Set desired verbosity here:
constexpr bool PrintDebugMessages = false;
constexpr bool PrintLogMessages = true;
constexpr bool PrintErrorMessages = true;
/////////////////////////////////////////////

template<typename... Types>
static inline void debug(Types... args)
{
	if constexpr (PrintDebugMessages)
		print(args...);
}

template<typename... Types>
static inline void log(Types... args)
{
	if constexpr (PrintLogMessages)
		print(args...);
}

template<typename... Types>
static inline void pr_err(Types... args)
{
	if constexpr (PrintErrorMessages)
		print(args...);
}

extern "C" void abort();

template<typename... Types>
static inline void panic(Types... args)
{
	if constexpr (PrintErrorMessages)
		print(args...);

	abort();
	// TODO: Reboot?
	// NVIC_SystemReset();
}
