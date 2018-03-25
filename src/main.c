/***************************************************************************//**
 * @file main.c
 * @brief Simple RAIL application which includes hal
 * @copyright Copyright 2017 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
#include "hal_common.h"

// emlib
#include "em_chip.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_wdog.h"
#include "em_adc.h"
#include "em_ldma.h"

#include "InitDevice.h"

#include <stdint.h>
#include <stdio.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// rig
#include "display.h"
#include "ui.h"
#include "rig.h"

#define NTASKS 4
TaskHandle_t taskhandles[NTASKS];

int testnumber=73;

static void debugputc(char c) {USART_Tx(USART0, c); }

void monitor_task() {
	char o[20];
	for(;;) {
		//testnumber++;
		int ti;
		for(ti=0; ti<NTASKS; ti++) {
			int v, n ,i;
			v = uxTaskGetStackHighWaterMark(taskhandles[ti]);
			n = snprintf(o, 18, "%d:%4d | ", ti, v);
			if(ti == NTASKS-1) o[n++] = '\n';
			for(i=0; i<n; i++)
				debugputc(o[i]);
			vTaskDelay(100);
		}
	}
}

void rail_task();
void dsp_task();

void vApplicationStackOverflowHook() {
	// beep
	uint32_t piip=0;
	for(;;) {
		TIMER_TopBufSet(TIMER0, 200);
		TIMER_CompareBufSet(TIMER0, 0, (((++piip)>>7)&63)+68);
	}
}

int main(void) {
	enter_DefaultMode_from_RESET();
	{
		LDMA_Init_t init = LDMA_INIT_DEFAULT;
		LDMA_Init(&init);
	}
	debugputc('a');

 	TIMER_TopSet(TIMER0, 200);
 	TIMER_CompareBufSet(TIMER0, 0, 33);

 	ADC_Start(ADC0, adcStartSingle);

	xTaskCreate(monitor_task, "task2", 0x80, NULL, 4, &taskhandles[3]);
	xTaskCreate(ui_task, "ui_task", 0x280, NULL, 3, &taskhandles[0]);
	xTaskCreate(rail_task, "rail_task", 0x200, NULL, 2, &taskhandles[1]);
	xTaskCreate(dsp_task, "task1", 0x200, NULL, 3, &taskhandles[2]);
	debugputc('\n');
 	vTaskStartScheduler();
	return 0;
}
