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
#include "InitDevice.h"
#include "rail.h"

#define DISPLAY_DMA_CH 0
static int display_initialized = 0, display_doing_dma = 0;

#define GPIO_PortOutSet(g, p) GPIO->P[g].DOUT |= (1<<(p));
#define GPIO_PortOutClear(g, p) GPIO->P[g].DOUT &= ~(1<<(p));

void display_start() {
	while (!(USART1->STATUS & USART_STATUS_TXC));
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
	GPIO_PortOutSet(TFT_DC_PORT, TFT_DC_PIN);
	GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
}

void display_end() {
	while (!(USART1->STATUS & USART_STATUS_TXC));
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
}

static void writedata(uint8_t d) {
	display_start();
	USART_SpiTransfer(USART1, d);
	display_end();
}

static void writecommand(uint8_t d) {
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
	GPIO_PortOutClear(TFT_DC_PORT, TFT_DC_PIN);
	GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
	USART_SpiTransfer(USART1, d);
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
}

void display_pixel(uint8_t r, uint8_t g, uint8_t b) {
	USART_Tx(USART1, r);
	USART_Tx(USART1, g);
	USART_Tx(USART1, b);
}

extern int testnumber;
void LDMA_IRQHandler() {
	testnumber++;
}

void display_transfer(uint8_t *dmadata, int dmalen) {
	LDMA_TransferCfg_t tr =
			LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART1_TXBL);
	LDMA_Descriptor_t desc =
			LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(dmadata, &USART1->TXDATA, dmalen);
	LDMA_IntEnable(1<<DISPLAY_DMA_CH);
	display_doing_dma = 1;
	LDMA_StartTransfer(DISPLAY_DMA_CH, &tr, &desc);
}

void display_area(int x1,int y1,int x2,int y2) {
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

int display_ready() {
	if(display_doing_dma) {
		if(LDMA_TransferDone(DISPLAY_DMA_CH))
			display_doing_dma = 0;
		else return 0;
	}
	return display_initialized;
}

// minimum delay between display init commands (us)
#define DISPLAY_INIT_DELAY_US 20000
#define CMD(x) ((x)|0x100)
void display_init_loop() {
	static int di_i = 0;
	static uint32_t next_time = 0;
	const uint16_t display_init_commands[] = {
			CMD(0x01), CMD(0x01), CMD(0x11), CMD(0x11), CMD(0x29), CMD(0x29),
			/*CMD(0x61), 0x03, 0x03,  // VLE (enable vertical scroll)
			CMD(0x80), 0, 3,
			CMD(0x81), 0, 4, 0, 10,
			CMD(0x83), 0, 70,
			CMD(0x84), 0, 4, 0, 10,
			CMD(0x03), (1<<(12-8)), (1<<3),
			CMD(0x31), 0, 20,
			CMD(0x32), 0, 1,
			CMD(0x33), 3, 4*/
			CMD(0x33), 0, 16, 0, 160-16, 0, 0 // vertical scrolling definition
	};

	uint32_t time = RAIL_GetTime();
	if(di_i != 0 && next_time - time >= 0x80000000UL) return;
	next_time = time + DISPLAY_INIT_DELAY_US;

	if(di_i <  sizeof(display_init_commands)/sizeof(display_init_commands[0])) {
		unsigned c = display_init_commands[di_i];
		if(c & 0x100) writecommand(c & 0xFF);
		else writedata(c);
		di_i++;
	} else {
		display_initialized = 1;
	}
}

void display_scroll(unsigned y) {
	writecommand(0x37);
	writedata(y>>8);
	writedata(y);
}
