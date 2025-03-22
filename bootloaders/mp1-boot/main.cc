#include "boot_media_loader.hh"
#include "clocks.hh"
#include "ddr/ram_tests.hh"
#include "ddr/stm32mp1_ram.h"
#include "delay.h"
#include "drivers/leds.hh"
#include "drivers/uart.hh"
#include "pmic.hh"
#include "print.hh"
#include "stm32mp157cxx_ca7.h"
#include "systeminit.h"
#include <cstring>

#include "odom_mk_v_conf.hh"
#include "osd32brk_conf.hh"
#include "stm32disco_conf.hh"

#include "sd_shared.hh"
#include "syscall_api.h"

// Uncomment one of these to select your board:
// namespace Board = OSD32BRK;
// namespace Board = STM32MP1Disco;
namespace Board = OdomMkV;

char *const LOG_START = reinterpret_cast<char *>(0x2fffc000u);
char *const LOG_WRAP = reinterpret_cast<char *>(0x30000000u);

extern "C" void run_ssbl(SSBL_Config *config);

void main()
{
	memset(LOG_START, 0, LOG_WRAP - LOG_START);
	Board::RedLED led;
	led.on();

	auto clockspeed = SystemClocks::init_core_clocks(Board::HSE_Clock_Hz, Board::MPU_MHz);
	security_init();

	Uart<Board::ConsoleUART> console(Board::UartRX, Board::UartTX, 115200);
	print("\n\nMP1-Boot\n\n");
	print("MPU clock: ", clockspeed, " Hz\n");

	QSPI_init();

	if constexpr (Board::PMIC::HasSTPMIC) {
		STPMIC1 pmic{Board::PMIC::I2C_config};

		if (!pmic.setup_vddcore_pwr())
			panic("Could not setup PMIC VDDCORE\n");

		if (!pmic.setup_ddr3_pwr())
			panic("Could not setup PMIC DDR voltages\n");
	}

	print("Initializing RAM\n");
	stm32mp1_ddr_setup();

	print("Testing RAM.\n");
	RamTests::run_all(DRAM_MEM_BASE, stm32mp1_ddr_get_size());

	SSBL_Config config = {
		.enable_sd = false,
	};

	// Find SD Card FAT partition.
	BootSDLoader sd_loader;
	my_sd_loader = &sd_loader;
	gpt_header gpt_hdr;
	const uint32_t last_block = sd_loader.hsd.SdCard.BlockNbr;
	const uint32_t gpt_addrs[2] = {1, last_block - 1};

	constexpr uint32_t InvalidPartitionNum = 0xFFFFFFFF;
	uint64_t fat_blockaddr = InvalidPartitionNum;
	for (auto blockaddr : gpt_addrs) {
		sd_loader.read(gpt_hdr, blockaddr);
		if (validate_gpt_header(&gpt_hdr, blockaddr, last_block)) {
			std::array<gpt_entry, 4> ptes;

			constexpr uint32_t fat_part_num = 3;
			uint32_t part_lba = gpt_hdr.partition_entry_lba + (fat_part_num / 4);
			sd_loader.read(ptes, part_lba);
			if (validate_partition_entry(ptes[fat_part_num % 4])) {
				fat_blockaddr = ptes[fat_part_num % 4].starting_lba;
			}

			if (fat_blockaddr != InvalidPartitionNum) {
				config.enable_sd = true;
				config.sd_fatfs_block_address = fat_blockaddr;
				config.sd_fatfs_block_count = ptes[fat_part_num % 4].ending_lba - fat_blockaddr + 1;

				break;
			}
		}
	}

	if (fat_blockaddr == InvalidPartitionNum) {
		pr_err("No valid GPT header found\n");
	}

	udelay(100000);

	run_ssbl(&config);

	// Should not reach here, but in case we do, blink LED rapidly
	print("FAILED!\n");
	constexpr uint32_t dlytime = 50000;
	while (1) {
		led.on();
		udelay(dlytime);
		led.off();
		udelay(dlytime);
	}
}

// void putchar_s(const char c) { Uart<Board::ConsoleUART>::putchar(c); }
void putchar_s(const char c)
{
	static char *log_location = LOG_START;
	*(log_location++) = c;
	if (log_location == LOG_WRAP)
		log_location = LOG_START;
}
