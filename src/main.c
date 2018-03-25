/***************************************************************************//**
 * @file main.c
 * @brief Simple RAIL application which includes hal
 * @copyright Copyright 2017 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
#include "hal_common.h"

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
#include "arm_math.h"
#include "arm_const_structs.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

#include "display.h"
#include "ui.h"
#include "rig.h"

#define NTASKS 4
TaskHandle_t taskhandles[NTASKS];
#define rail_task_h taskhandles[0]
#define ui_task_h taskhandles[1]
#define task1h taskhandles[2]
#define task2h taskhandles[3]
//TaskHandle_t rail_task_h=NULL, ui_task_h=NULL, task1h=NULL, task2h=NULL;

int testnumber=73;

//const arm_cfft_instance_f32 *fftS = &arm_cfft_sR_f32_len128;
const arm_cfft_instance_f32 *fftS = &arm_cfft_sR_f32_len256;
float fftbuf[2*FFTLEN];
volatile int fftbufp = 0;

void task1() {
	for(;;) {
		// TODO: semaphore?
		if(fftbufp >= 2*FFTLEN) {
			arm_cfft_f32(fftS, fftbuf, 0, 1);
			ui_fft_line(fftbuf);
			fftbufp = 0;
		}
		vTaskDelay(2);
	}
}

void task2() {
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
				USART_Tx(USART0, o[i]);
			vTaskDelay(100);
		}
	}
}

void rail_task();

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
	USART_Tx(USART0, 'a');

 	TIMER_TopSet(TIMER0, 200);
 	TIMER_CompareBufSet(TIMER0, 0, 33);

 	ADC_Start(ADC0, adcStartSingle);
	USART_Tx(USART0, 'b');

	xTaskCreate(ui_task, "ui_task", 0x280, NULL, 3, &ui_task_h);
	USART_Tx(USART0, 'c');
	xTaskCreate(rail_task, "rail_task", 0x200, NULL, 2, &rail_task_h);
	USART_Tx(USART0, 'd');
	xTaskCreate(task1, "task1", 0x200, NULL, 3, &task1h);
	USART_Tx(USART0, 'e');
	xTaskCreate(task2, "task2", 0x80, NULL, 4, &task2h);
	USART_Tx(USART0, 'f');
	USART_Tx(USART0, '\n');
 	vTaskStartScheduler();
	return 0;
}
