/* SPDX-License-Identifier: MIT */

/* ---------------------
 * Power management code
 * ---------------------
 */

#include "InitDevice.h"

#include "em_cmu.h"
#include "em_emu.h"
#include "em_rmu.h"
#include "em_gpio.h"

/* Turn off the device.
 *
 * To avoid the need to carefully turn off every peripheral
 * and bring the software into a state suitable for sleep,
 * the processor is simply reset instead.
 * Reset should turn off most peripherals automatically.
 * maybe_sleep is called after reset and it then checks whether
 * the device should go to sleep or start running normal code.
 */
void shutdown(void)
{
	NVIC_SystemReset();
}

// Delay that can be used before any timers are initialized.
static void busy_delay(unsigned length)
{
	unsigned i;
	for (i = 0; i < length; i++)
		__NOP();
}

static void go_to_sleep(void)
{
	EMU_EM4Init_TypeDef e = EMU_EM4INIT_DEFAULT;
	e.pinRetentionMode = emuPinRetentionEm4Exit;
	EMU_EM4Init(&e);

	// Encoder button PF7 is EM4WU1 which is bit 17
	GPIO_EM4EnablePinWakeup(1 << 17, 1);

	EMU_EnterEM4S();
	// EnterEM4 should never return because exit from EM4
	// resets the processor, but just in case
	for (;;);
}

// Called before initializing any peripherals.
// Check whether the device should go to sleep.
void maybe_sleep(void)
{
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Make sure all extra hardware is turned off
	// or in a state that should consume minimum current.
	GPIO_PinModeSet(TFT_EN_PORT, TFT_EN_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(PWM_PORT,    PWM_PIN,    gpioModePushPull, 1);
	// Encoder button GPIO is used to wake up from sleep.
	GPIO_PinModeSet(ENCP_PORT, ENCP_PIN, gpioModeInputPullFilter, 1);

	// Check the reset cause.
	uint32_t cause = RMU_ResetCauseGet();
	RMU_ResetCauseClear();

	if (cause == RMU_RSTCAUSE_SYSREQRST) {
		// Calling shutdown() causes a system reset, so if the cause is
		// system reset request, go to sleep.
		// Wait a moment before going to sleep, so that it is
		// still possible to use SWD. The delay also gives some time
		// for the wakeup button debouncing capacitor to charge
		// so the button is not erroneously seen as being pressed.
		busy_delay(1000000);
		go_to_sleep();
	} else if (cause == RMU_RSTCAUSE_EM4RST) {
		// If the cause was a wakeup from this EM4 sleep, button
		// has probably been pressed and the device should wake up.
		// I am not sure if something else could cause an EM4 wakeup,
		// so check the button again just in case.
		busy_delay(1000000);
		if (GPIO_PinInGet(ENCP_PORT, ENCP_PIN)) {
			// Button was not actually pressed.
			go_to_sleep();
		}
	}

	// If reset cause was something else, return and let the device wake up.
}
