/*
 * display.c
 *
 *  Created on: Sep 24, 2017
 *      Author: Tatu
 */

/*
 * P1 PC6  DATA
 * P3 PC7  CS
 * P5 PC8  CLK
 * P7 PC9  DC
 */

#include <stdint.h>
#include "em_usart.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_timer.h"
#include "InitDevice.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// rig
#include "ui_parameters.h"

#define DISPLAY_DMA_CH 0
static int display_initialized = 0, display_doing_dma = 0;

#define GPIO_PortOutSet(g, p) GPIO->P[g].DOUT |= (1<<(p));
#define GPIO_PortOutClear(g, p) GPIO->P[g].DOUT &= ~(1<<(p));

/*
 * The display has a pin which selects whether an SPI transfer is going
 * to be a command or a data byte. This is controlled by a GPIO output.
 * writecommand() and writedata() set the state of that pin and
 * busy-wait until the next transfer can be done, so that the GPIO is
 * set at the right moment.
 *
 * Busy-waiting during SPI transfers seems kind of ugly, but a typical
 * command-data cycle takes just a few tens of CPU clock cycles.
 * Thus, there's not much time to do a context switch to something
 * else during a command transfer.
 * For short commands, I guess it's most efficient to just busy wait...
 *
 * Pixel data is transferred using DMA and the task is blocked during
 * the transfer, allowing a context switch to another task.
 */

void display_start(void)
{
	while (!(USART1->STATUS & USART_STATUS_TXC));
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
	GPIO_PortOutSet(TFT_DC_PORT, TFT_DC_PIN);
	GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
}

void display_end(void)
{
	while (!(USART1->STATUS & USART_STATUS_TXC));
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
}

static void writedata(uint8_t d)
{
	//display_start();
	GPIO_PortOutSet(TFT_DC_PORT, TFT_DC_PIN);
	USART_SpiTransfer(USART1, d);
	//display_end();
}

static void writecommand(uint8_t d)
{
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
	GPIO_PortOutClear(TFT_DC_PORT, TFT_DC_PIN);
	GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
	USART_SpiTransfer(USART1, d);
	//GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
}

void display_pixel(uint8_t r, uint8_t g, uint8_t b)
{
	USART_Tx(USART1, r);
	USART_Tx(USART1, g);
	USART_Tx(USART1, b);
}

extern int testnumber;
static TaskHandle_t myhandle;

void LDMA_IRQHandler(void)
{
	//testnumber++;
	uint32_t pending = LDMA_IntGetEnabled();
	if(pending & (1<<DISPLAY_DMA_CH)) {
		LDMA->IFC = 1<<DISPLAY_DMA_CH;

		BaseType_t xHigherPriorityTaskWoken;
		/* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
		vTaskNotifyGiveFromISR() unblocks the handling task, and the priority of
		the handling task is higher than the priority of the currently running task,
		then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
		xHigherPriorityTaskWoken = pdFALSE;

		/* Unblock the handling task so the task can perform any processing necessitated
		by the interrupt.  xHandlingTask is the task's handle, which was obtained
		when the task was created. */
		vTaskNotifyGiveFromISR(myhandle, &xHigherPriorityTaskWoken);
		//xSemaphoreGiveFromISR(dma_semaphore, &xHigherPriorityTaskWoken);

		/* Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
		The macro used to do this is dependent on the port and may be called
		portEND_SWITCHING_ISR. */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

void display_transfer(const uint8_t *dmadata, int dmalen)
{
	LDMA_TransferCfg_t tr =
			LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART1_TXBL);
	LDMA_Descriptor_t desc =
			LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(dmadata, &USART1->TXDATA, dmalen);
	//LDMA_IntEnable(1<<DISPLAY_DMA_CH);
	display_doing_dma = 1;
	myhandle = xTaskGetCurrentTaskHandle();
	LDMA_StartTransfer(DISPLAY_DMA_CH, &tr, &desc);
	//xSemaphoreTake(dma_semaphore, 10);
	ulTaskNotifyTake(pdFALSE, 100);
}

void display_area(int x1,int y1,int x2,int y2)
{
	writecommand(0x2A); // column address set
	writedata(0);
	writedata(x1);
	writedata(0);
	writedata(x2);
	writecommand(0x2B); // row address set
	writedata(0);
	writedata(y1);
	writedata(0);
	writedata(y2);
	writecommand(0x2C); // memory write
}

int display_ready()
{
	if(display_doing_dma) {
		if(LDMA_TransferDone(DISPLAY_DMA_CH))
			display_doing_dma = 0;
		else return 0;
	}
	return display_initialized;
}

#define CMD(x) ((x)|0x100)
#define INIT_LIST_END 0x200

const uint16_t display_init_commands[] = {
		CMD(0x01), CMD(0x01), CMD(0x11), CMD(0x11), CMD(0x29), CMD(0x29),
		CMD(0x33), // vertical scrolling definition
			0, FFT_ROW1, 0, FFT_ROW2+1-FFT_ROW1, 0, 159-FFT_ROW2,
		INIT_LIST_END
};

int display_init(void)
{
	display_initialized = 0;
	unsigned i;
	for (i = 0;; i++) {
		unsigned c = display_init_commands[i];
		if (c == INIT_LIST_END)
			break;
		if (c & 0x100) {
			vTaskDelay(30);
			writecommand(c & 0xFF);
		} else {
			writedata(c);
		}
	}
	display_initialized = 1;
	return 0;
}


void display_scroll(unsigned y)
{
	writecommand(0x37);
	writedata(y>>8);
	writedata(y);
}

void display_backlight(int b)
{
	if(b < 0) b = 0;
	if(b > 200) b = 200;
 	TIMER_CompareBufSet(TIMER0, 1, b);
}

/*void display_preinit() {
	dma_semaphore = xSemaphoreCreateBinary();
}*/
