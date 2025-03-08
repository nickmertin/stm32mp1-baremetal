#pragma once
#include "drivers/i2c_conf.hh"
#include "drivers/leds.hh"
#include "stm32mp1xx.h"

namespace OdomMkV
{

using RedLED = Led<GPIO::E, PinNum::_11, LedActive::High>;
// using GreenLED = Led<GPIO::E, PinNum::_13, LedActive::High>;
// using RedLED2 = Led<GPIO::I, PinNum::_8, LedActive::High>;
// using GreenLED2 = Led<GPIO::I, PinNum::_9, LedActive::High>;

using OrangeLED = RedLED;									// For compatibility with STM32MP1 Discovery board
using BlueLED = Led<GPIO::E, PinNum::_13, LedActive::High>; // For compatibility with STM32MP1 Discovery board

constexpr uint32_t ConsoleUART = UART4_BASE;
constexpr PinConf UartRX{GPIO::B, PinNum::_2, PinAF::AF_8};
constexpr PinConf UartTX{GPIO::G, PinNum::_11, PinAF::AF_6};

// Specify whether the board has a PMIC power management IC, and what I2C bus it's on
namespace PMIC
{
constexpr bool HasSTPMIC = false;
constexpr I2C_Config I2C_config{};
} // namespace PMIC

// Specify whether the board has a USBC Interface IC, and what I2C bus it's on
namespace USBC_Interface
{
constexpr bool HasSTUSB1600 = false;
constexpr I2C_Config I2C_config{};
}; // namespace USBC_Interface

constexpr uint32_t HSE_Clock_Hz = 8000000;
constexpr uint32_t MPU_MHz = 650;

} // namespace OdomMkV
