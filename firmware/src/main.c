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
#include "dsp_driver.h"
#include "power.h"
#include "railtask.h"

/* --------------------
 * Interrupt priorities
 * --------------------
 * When changing these, remember to check
 * configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY in FreeRTOSConfig.h
 */

#define IRQPRI_RAIL 3
#define IRQPRI_SAMPLE_RATE_TIMER 3


/* ---------------------------------
 * Variables and function prototypes
 * ---------------------------------
 */

#define NTASKS 5
TaskHandle_t taskhandles[NTASKS];

void slow_dsp_task(void *);
void misc_fast_task(void *);

void debug_init(void);
void slow_dsp_rtos_init(void);

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
	NVIC_SetPriority( WTIMER0_IRQn, IRQPRI_SAMPLE_RATE_TIMER);
	{
		LDMA_Init_t init = LDMA_INIT_DEFAULT;
		LDMA_Init(&init);
	}

	TIMER_TopSet(TIMER0, 199);
	TIMER_CompareBufSet(TIMER0, 0, 33);
	TIMER_CompareBufSet(TIMER0, 1, 20);
	dsp_hw_init();
	printf("Peripherals initialized\n");

	dsp_rtos_init();
	slow_dsp_rtos_init();
	ui_rtos_init();
	railtask_rtos_init();

	xTaskCreate(misc_fast_task, "Misc", 0x100, NULL, 4, &taskhandles[3]);
	xTaskCreate(display_task, "Display", 0x300, NULL, 2, &taskhandles[0]);
	xTaskCreate(railtask_main, "RAIL", 0x300, NULL, 2, &taskhandles[1]);
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
		ui_control_backlight();
		int ti;
		for(ti=0; ti<NTASKS; ti++) {
#if 0
			int v;
			v = uxTaskGetStackHighWaterMark(taskhandles[ti]);
			printf("%d:%4d | ", ti, v);
			if(ti == NTASKS-1) printf("\n");
#endif
		}
		vTaskDelay(10);
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
