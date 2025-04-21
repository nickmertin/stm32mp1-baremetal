#include "syscall_api.h"

void putchar_s(const char c);

void syscall_putbytes(const uint8_t *bytes, size_t count)
{
	for (size_t i = 0; i < count; i++)
		putchar_s(bytes[i]);
}
