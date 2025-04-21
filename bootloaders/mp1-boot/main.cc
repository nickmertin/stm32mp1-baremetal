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

#include "syscall_api.h"

// Uncomment one of these to select your board:
// namespace Board = OSD32BRK;
// namespace Board = STM32MP1Disco;
namespace Board = OdomMkV;

char *const LOG_START = reinterpret_cast<char *>(0x2fffc000u);
char *const LOG_WRAP = reinterpret_cast<char *>(0x30000000u);

extern "C" void run_ssbl();
Board::BlueLED led;

void main()
{
	memset(LOG_START, 0, LOG_WRAP - LOG_START);

	// Enable debugging
	RCC->MP_APB5ENSETR = RCC_MP_APB5ENSETR_BSECEN;
	BSEC->BSEC_DENABLE = 0x47F;

	auto clockspeed = SystemClocks::init_core_clocks(Board::HSE_Clock_Hz, Board::MPU_MHz);
	security_init();

	led.on();

	print("\n\nMP1-Boot\n\n");
	print("MPU clock: ", clockspeed, " Hz\n");

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

	// These pins are not board-specific, they are required by BOOTROM
	// for booting with SDMMC1
	// D1 - D3 are not used by BOOTROM, so need to be init by FSBL
	PinConf{GPIO::C, PinNum::_9, PinAF::AF_12}.init(PinMode::Alt);
	PinConf{GPIO::C, PinNum::_10, PinAF::AF_12}.init(PinMode::Alt);
	PinConf{GPIO::C, PinNum::_11, PinAF::AF_12}.init(PinMode::Alt);

	// D0, CK, CMD are used by BOOTROM and should already be init. We re-init them just in case...
	PinConf{GPIO::C, PinNum::_8, PinAF::AF_12}.init(PinMode::Alt);
	PinConf{GPIO::C, PinNum::_12, PinAF::AF_12}.init(PinMode::Alt);
	PinConf{GPIO::D, PinNum::_2, PinAF::AF_12}.init(PinMode::Alt);

	// udelay(100000);

	run_ssbl();

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

extern "C" void abort()
{
	led.off();
	Board::RedLED red_led;
	red_led.on();
	while (1)
		;
}
