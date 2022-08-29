/* SPDX-License-Identifier: MIT */

// emlib
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_wdog.h"
#include "em_ldma.h"

#include "InitDevice.h"

#include <stdint.h>
#include <stdio.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// rig
#include "display.h"
#include "ui.h"
#include "rig.h"

/* --------------------
 * Interrupt priorities
 * --------------------
 * When changing these, remember to check
 * configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY in FreeRTOSConfig.h
 */

#define IRQPRI_RAIL 3
#define IRQPRI_AUDIO_ADC 3


/* ---------------------------------
 * Variables and function prototypes
 * ---------------------------------
 */

int testnumber=73;

#define NTASKS 5
TaskHandle_t taskhandles[NTASKS];

void rail_task(void *);
void fast_dsp_task(void *);
void slow_dsp_task(void *);
void misc_fast_task(void *);

void debug_init(void);
void dsp_rtos_init(void);
void slow_dsp_rtos_init(void);

void maybe_sleep(void);
void beep(unsigned period, unsigned pulsewidth, unsigned pulses);
void shutdown(void);

/* -------------
 * Main function
 * -------------
 * Initializes some hardware and tasks
 * before starting the RTOS scheduler.
 */
int main(void) {
	CHIP_Init();
	debug_init();
	printf("Gekkokapula\n");

	CMU_ClockEnable(cmuClock_GPIO, true);
	maybe_sleep();
	// If maybe_sleep returned, the device should turn on.
	// Crystal oscillator needs to be powered on before configuring clocks.
	// It's behind the same load switch as the display, so turn them on.
	GPIO_PinModeSet(TFT_EN_PORT, TFT_EN_PIN, gpioModePushPull, 0);

	enter_DefaultMode_from_RESET();
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	NVIC_SetPriorityGrouping(0);
	NVIC_SetPriority( FRC_PRI_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority(     FRC_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority(   MODEM_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority( RAC_SEQ_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority( RAC_RSM_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority(    BUFC_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority(     AGC_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority(PROTIMER_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority(   SYNTH_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority( RFSENSE_IRQn, IRQPRI_RAIL);
	NVIC_SetPriority(    ADC0_IRQn, IRQPRI_AUDIO_ADC);
	{
		LDMA_Init_t init = LDMA_INIT_DEFAULT;
		LDMA_Init(&init);
	}

	TIMER_TopSet(TIMER0, 200);
	TIMER_CompareBufSet(TIMER0, 0, 33);
	TIMER_CompareBufSet(TIMER0, 1, 20);
	printf("Peripherals initialized\n");

	dsp_rtos_init();
	slow_dsp_rtos_init();
	ui_rtos_init();

	xTaskCreate(misc_fast_task, "Misc", 0x100, NULL, 4, &taskhandles[3]);
	xTaskCreate(display_task, "Display", 0x300, NULL, 2, &taskhandles[0]);
	xTaskCreate(rail_task, "RAIL", 0x300, NULL, 2, &taskhandles[1]);
	xTaskCreate(fast_dsp_task, "Fast DSP", 0x300, NULL, 4, &taskhandles[4]);
	xTaskCreate(slow_dsp_task, "Slow DSP", 0x300, NULL, 2, &taskhandles[2]);

	printf("Starting scheduler\n");
	vTaskStartScheduler();
	return 0;
}


/* ---------------------
 * Some application code
 * ---------------------
 */

extern char rail_watchdog;

static inline void restart_rail_task() {
	vTaskDelete(taskhandles[1]);
	xTaskCreate(rail_task, "rail_task", 0x200, NULL, 2, &taskhandles[1]);
}


/* Task to do various "small" things which have to run regularly,
 * don't take much CPU time and don't need a dedicated task.
 * These are typically things that poll for something.
 * This includes:
 * - Reading user interface inputs
 * - Controlling display backlight brightness
 * - Monitoring other tasks
 */
void misc_fast_task(void *arg) {
	(void)arg;
	for(;;) {
		ui_check_buttons();

		// Hack: if audio volume is turned to 0, shut down.
		// This is for testing before adding it properly to the UI.
		if (p.volume == 0)
			shutdown();

		ui_control_backlight();
		//testnumber++;
		int ti;
		for(ti=0; ti<NTASKS; ti++) {
#if 0
			int v;
			v = uxTaskGetStackHighWaterMark(taskhandles[ti]);
			printf("%d:%4d | ", ti, v);
			if(ti == NTASKS-1) printf("\n");
#endif
		}
		if(++rail_watchdog >= 200)
			restart_rail_task();
		vTaskDelay(10);
	}
}


/* ---------------------
 * Power management code
 * ---------------------
 */

/* Turn off the device.
 *
 * To avoid the need to carefully turn off every peripheral
 * and bring the software into a state suitable for sleep,
 * the processor is simply reset instead.
 * Reset should turn off most peripherals automatically.
 * The main function then first checks whether it should go
 * to sleep or start running normal code.
 */
void shutdown(void)
{
	NVIC_SystemReset();
}


/* Check whether the device should be on or off and act accordingly.
 * This is only called before any other hardware is initialized.
 */
void maybe_sleep(void)
{
	// Make sure all extra hardware is turned off
	// or in a state that should consume minimum current.
	GPIO_PinModeSet(TFT_EN_PORT, TFT_EN_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(PWM_PORT,    PWM_PIN,    gpioModePushPull, 1);

	// Check whether encoder button is pressed.
	// Wait a moment before checking the state
	// to let the debouncing capacitor charge.
	GPIO_PinModeSet(ENCP_PORT, ENCP_PIN, gpioModeInputPullFilter, 1);
	beep(20000, 20, 10); // works as a delay too
	if (GPIO_PinInGet(ENCP_PORT, ENCP_PIN) == 0) {
		// Button was pressed.
		// Don't go to sleep.
		beep(5000, 10, 10);
		return;
	}
	// Wait a moment so that SWD may still work without reset connected
	beep(100000, 20, 20);

	// Go to sleep.

	EMU_EM4Init_TypeDef e = EMU_EM4INIT_DEFAULT;
	e.pinRetentionMode = emuPinRetentionEm4Exit;
	EMU_EM4Init(&e);

	// PF7 is EM4WU1 which is bit 17
	GPIO_EM4EnablePinWakeup(1 << 17, 1);

	EMU_EnterEM4S();
	// EnterEM4 should never return because exit from EM4
	// resets the processor, but just in case
	for (;;);
}


/* Beep that can be used before any timers are initialized.
 * This is useful for testing early initialization code. */
void beep(unsigned period, unsigned pulsewidth, unsigned pulses)
{
	unsigned i;
	period -= pulsewidth;
	for (i = 0; i < pulses; i++) {
		unsigned p;
		for (p = 0; p < pulsewidth; p++)
			GPIO_PinOutClear(PWM_PORT, PWM_PIN);
		for (p = 0; p < period; p++)
			GPIO_PinOutSet(PWM_PORT, PWM_PIN);
	}
}


/* --------------------------------------
 * RTOS hook functions and related things
 * --------------------------------------
 */

const char *current_task_name(void)
{
	TaskHandle_t t = xTaskGetCurrentTaskHandle();
	if (t != NULL)
		return pcTaskGetTaskName(t);
	else
		return "none";
}


void vApplicationStackOverflowHook()
{
	printf(":( Stack overflow in task %s\n", current_task_name());
	// beep
	uint32_t piip=0;
	for(;;) {
		TIMER_TopBufSet(TIMER0, 200);
		TIMER_CompareBufSet(TIMER0, 0, (((++piip)>>7)&63)+68);
	}
}


void vApplicationMallocFailedHook()
{
	printf(":( Malloc failed in task %s\n", current_task_name());
	// beep
	uint32_t piip=0;
	for(;;) {
		TIMER_TopBufSet(TIMER0, 200);
		TIMER_CompareBufSet(TIMER0, 0, (((++piip)>>8)&63)+68);
	}
}


void vApplicationIdleHook()
{
	__DSB();
	__WFI();
	__ISB();
}


void Default_Handler()
{
	/* Find the number of the current interrupt */
	printf(":( IRQ %d in task %s\n",
		(int)(SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) - 16,
		current_task_name());
	for (;;);
}
