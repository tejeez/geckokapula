/***************************************************************************//**
 * @file main.c
 * @brief Simple RAIL application which includes hal
 * @copyright Copyright 2017 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/

// emlib
#include "em_chip.h"
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

// rig
#include "display.h"
#include "ui.h"
#include "rig.h"

#define NTASKS 4
TaskHandle_t taskhandles[NTASKS];

int testnumber=73;

void rail_task();
void dsp_task();
extern char rail_watchdog;

static inline void restart_rail_task() {
	vTaskDelete(taskhandles[1]);
	xTaskCreate(rail_task, "rail_task", 0x200, NULL, 2, &taskhandles[1]);
}

void dump_memory(uint8_t *mem, int len, char last) {
	int m;
	char o[4];
	for(m=0; m<len; m++) {
		int n, i;
		printf("%02x ", mem[m]);
	}
}

void monitor_task() {
	for(;;) {
		//testnumber++;
		int ti;
		for(ti=0; ti<NTASKS; ti++) {
#if 0
			int v;
			v = uxTaskGetStackHighWaterMark(taskhandles[ti]);
			printf("%d:%4d | ", ti, v);
			if(ti == NTASKS-1) printf("\n");
#endif
			if(++rail_watchdog >= 20)
				restart_rail_task();
			vTaskDelay(100);
		}
#if 0
		// print some registers which might be related to the radio
		dump_memory((void*)0x40080000, 1024, '/');
		//dump_memory((void*)0x40081000, 1024, '/');
		//dump_memory((void*)0x40082000, 1024, '/');
		dump_memory((void*)0x40083000, 1024, '/');
		dump_memory((void*)0x40084000, 1024, '\n');
#endif
	}
}

void vApplicationStackOverflowHook() {
	// beep
	uint32_t piip=0;
	for(;;) {
		TIMER_TopBufSet(TIMER0, 200);
		TIMER_CompareBufSet(TIMER0, 0, (((++piip)>>7)&63)+68);
	}
}

void debug_init(void);

int main(void) {
	debug_init();
	printf("Gekkokapula\n");
	enter_DefaultMode_from_RESET();
	{
		LDMA_Init_t init = LDMA_INIT_DEFAULT;
		LDMA_Init(&init);
	}

 	TIMER_TopSet(TIMER0, 200);
 	TIMER_CompareBufSet(TIMER0, 0, 33);
 	TIMER_CompareBufSet(TIMER0, 1, 20);
	printf("Peripherals initialized\n");

	xTaskCreate(monitor_task, "task2", 0x100, NULL, 3, &taskhandles[3]);
	xTaskCreate(ui_task, "ui_task", 0x300, NULL, 3, &taskhandles[0]);
	xTaskCreate(rail_task, "rail_task", 0x280, NULL, /*2*/ 3, &taskhandles[1]);
	xTaskCreate(dsp_task, "task1", 0x280, NULL, 3, &taskhandles[2]);
	printf("Starting scheduler\n");
 	vTaskStartScheduler();
	return 0;
}
