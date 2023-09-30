/* SPDX-License-Identifier: MIT */

#include "config.h"
#include "em_gpio.h"
#include "InitDevice.h"

bool tx_freq_allowed(uint32_t frequency)
{
	if (GPIO_PinInGet(TEST_PORT, TEST_PIN) == 0) {
		// Test mode: allow transmitting on any frequency
		return 1;
	}
	if (frequency >= 432000000UL && frequency <= 438000000UL)
		return 1;
	if (frequency >= 2300000000UL && frequency <= 2450000000UL)
		return 1;
	return 0;
}
