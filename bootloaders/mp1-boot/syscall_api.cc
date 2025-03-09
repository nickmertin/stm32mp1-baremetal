#include "syscall_api.h"
#include "delay.h"
#include "drivers/pinconf.hh"
#include "print.hh"
#include "stm32mp1xx_hal_sd.h"

void putchar_s(const char c);

static SD_HandleTypeDef hsd;

void syscall_putbytes(const uint8_t *bytes, size_t count)
{
	for (size_t i = 0; i < count; i++)
		putchar_s(bytes[i]);
}

// bool syscall_init_sd_card(uint32_t *error)
// {
// 	RCC->SDMMC12CKSELR = 3; // HSI = 64MHz. Default value (just showing it here for educational purposes)

// 	HAL_SD_DeInit(&hsd);
// 	hsd.Instance = SDMMC1;
// 	hsd.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
// 	hsd.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
// 	hsd.Init.BusWide = SDMMC_BUS_WIDE_4B;
// 	hsd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
// 	hsd.Init.ClockDiv = 64; // 64MHz/2 / 64 = 500kHz, seems to be the max OSD32-BRK can handle reliably

// 	// These pins are not board-specific, they are required by BOOTROM
// 	// for booting with SDMMC1
// 	// D1 - D3 are not used by BOOTROM, so need to be init by FSBL
// 	PinConf{GPIO::C, PinNum::_9, PinAF::AF_12}.init(PinMode::Alt);
// 	PinConf{GPIO::C, PinNum::_10, PinAF::AF_12}.init(PinMode::Alt);
// 	PinConf{GPIO::C, PinNum::_11, PinAF::AF_12}.init(PinMode::Alt);

// 	// D0, CK, CMD are used by BOOTROM and should already be init. We re-init them just in case...
// 	PinConf{GPIO::C, PinNum::_8, PinAF::AF_12}.init(PinMode::Alt);
// 	PinConf{GPIO::C, PinNum::_12, PinAF::AF_12}.init(PinMode::Alt);
// 	PinConf{GPIO::D, PinNum::_2, PinAF::AF_12}.init(PinMode::Alt);

// 	udelay(10000);

// 	auto ok = HAL_SD_Init(&hsd);
// 	if (ok != HAL_OK) {
// 		*error = hsd.ErrorCode;
// 		return false;
// 	}

// 	udelay(10000);

// 	return true;
// }

// bool syscall_read_sd_card(
// 	uint8_t *data, uint32_t block_address, uint32_t block_count, uint32_t timeout, uint32_t *error)
// {
// 	auto err = HAL_SD_ReadBlocks(&hsd, data, block_address, block_count, timeout);
// 	if (err = HAL_OK)
// 		return true;

// 	*error = hsd.ErrorCode;
// 	return false;
// }
