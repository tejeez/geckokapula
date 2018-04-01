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
#include "hw.h"

#define NTASKS 4
TaskHandle_t taskhandles[NTASKS];

int testnumber=73;

void debugputc(char c) {USART_Tx(USART0, c); }
void debugc(char c) {USART0->TXDATA = c;}
void debugputs(char *c) { while(*c != '\0') { USART_Tx(USART0, *c); c++; } }

void rail_task();
void dsp_task();
extern char rail_watchdog;

/*static inline void restart_rail_task() {
	vTaskDelete(taskhandles[2]);
	xTaskCreate(rail_task, "RAIL", 0x200, NULL, 2, &taskhandles[2]);
}*/

void dump_memory(uint8_t *mem, int len, char last) {
	int m;
	char o[4];
	for(m=0; m<len; m++) {
		int n, i;
		n = snprintf(o, 4, "%02x ", mem[m]);
		for(i=0; i<n; i++)
			debugputc(o[i]);
	}
	debugputc(last);
}

void monitor_task() {
	for(;;) {
		//testnumber++;
		int ti;
		for(ti=0; ti<NTASKS; ti++) {
			char o[20];
			int v, n, i;
			v = uxTaskGetStackHighWaterMark(taskhandles[ti]);
			n = snprintf(o, 18, "%d:%4d | ", ti, v);
			if(ti == NTASKS-1) o[n++] = '\n';
			for(i=0; i<n; i++)
				debugputc(o[i]);
			/*if(++rail_watchdog >= 3)
				restart_rail_task();*/
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
		debugc('$');
		TIMER_TopBufSet(TIMER0, 200);
		TIMER_CompareBufSet(TIMER0, 0, (((++piip)>>7)&63)+68);
	}
}

void dsp_rx_testsignals();
void start_rx_dsp();
int main(void) {
	enter_DefaultMode_from_RESET();
	{
		static const LDMA_Init_t init = {
		.ldmaInitCtrlNumFixed     = 7, /* Fixed priority arbitration.*/
		.ldmaInitCtrlSyncPrsClrEn = 0, /* No PRS Synctrig clear enable*/
		.ldmaInitCtrlSyncPrsSetEn = 0, /* No PRS Synctrig set enable. */
		.ldmaInitIrqPriority      = 3  /* IRQ priority level 3.       */
		};
		LDMA_Init(&init);
	}

	TIMER_TopSet(TIMER0, TIMER0_PERIOD);

	dsp_rx_testsignals();
	start_tx_dsp();
	start_rx_dsp();
	LDMA_IntDisable((1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7));
#if 1
	xTaskCreate(monitor_task, "MON", 0x200, NULL, 3, &taskhandles[0]);
	xTaskCreate(ui_task, "UI", 0x200, NULL, 3, &taskhandles[1]);
	xTaskCreate(rail_task, "RAIL", 0x200, NULL, 3, &taskhandles[2]);
	xTaskCreate(dsp_task, "DSP", 0x200, NULL, 3, &taskhandles[3]);
	debugputc('\n');
 	vTaskStartScheduler();
#else
 	for(;;);
#endif
	return 0;
}

#define HANDLER(x) void x ## _Handler() { USART0->TXDATA = '!'; debugputs("\n" #x " :(\n"); for(;;); }
HANDLER(HardFault);
HANDLER(BusFault);
HANDLER(UsageFault);
HANDLER(MemManage);
HANDLER(NMI);
HANDLER(Default);
