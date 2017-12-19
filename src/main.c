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

int testnumber=73;

const arm_cfft_instance_f32 *fftS = &arm_cfft_sR_f32_len128;
float fftbuf[2*FFTLEN];
volatile int fftbufp = 0;

void task1() {
	for(;;) {
		if(fftbufp >= 2*FFTLEN) {
			arm_cfft_f32(fftS, fftbuf, 0, 1);
			ui_fft_line(fftbuf);
			fftbufp = 0;
		}
		taskYIELD();

		//testnumber = ADC_DataSingleGet(ADC0);
		ui_loop();
		taskYIELD();
	}
}

void task2() {
	for(;;) {
		testnumber++;
		taskYIELD();
	}
}

TaskHandle_t rail_task_h, task1h, task2h;

void rail_task();

int main(void) {
	enter_DefaultMode_from_RESET();
	USART_Tx(USART0, 'a');

 	TIMER_TopSet(TIMER0, 200);
 	TIMER_CompareBufSet(TIMER0, 0, 33);

 	ADC_Start(ADC0, adcStartSingle);

 	xTaskCreate(rail_task, "rail_task", 128, NULL, 1, &rail_task_h);
 	xTaskCreate(task1, "task1", 512, NULL, 1, &task1h);
 	xTaskCreate(task2, "task2", 128, NULL, 1, &task2h);
 	vTaskStartScheduler();
	return 0;
}
