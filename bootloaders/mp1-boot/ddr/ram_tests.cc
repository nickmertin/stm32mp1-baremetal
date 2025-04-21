#include "print_messages.hh"
#include <cstdint>

namespace RamTests
{

bool test_all_words()
{
	constexpr uint32_t value = 0x12345678;
	volatile uint32_t *addr = (uint32_t *)0xC0000000;

	for (uint32_t i = 0; i < 32 * 1024; i++) {
		addr[i] = value;
	}

	for (uint32_t i = 0; i < 32 * 1024; i++) {
		auto read_value = addr[i];
		if (read_value != value) {
			panic("RAM Test Fail: test_all_words() at 0x",
				  Hex{(uint32_t)&addr[i]},
				  ": Wrote 0x12345678, read 0x",
				  Hex{read_value});
			return false;
		}
	}

	return true;
}

bool run_all(uint32_t ram_start, uint32_t ram_size)
{
	bool ok = test_all_words();

	return ok;
}

} // namespace RamTests
