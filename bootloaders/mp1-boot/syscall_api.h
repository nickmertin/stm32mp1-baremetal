#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void syscall_putbytes(const uint8_t *bytes, size_t count);
__attribute__((noreturn)) void syscall_start_program(uint8_t *entry);

#ifdef __cplusplus
}
#endif