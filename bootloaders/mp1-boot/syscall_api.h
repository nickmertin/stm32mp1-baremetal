#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void syscall_putbytes(const uint8_t *bytes, size_t count);
bool syscall_init_sd_card(uint32_t *error);
bool syscall_read_sd_card(
	uint8_t *data, uint32_t block_address, uint32_t block_count, uint32_t timeout, uint32_t *error);

#ifdef __cplusplus
}
#endif