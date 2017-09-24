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
#include "InitDevice.h"

extern uint8_t displayinit[];
extern const int displayinit_len;
static int di_i = 0, di_numargs = 0, di_ms = 0;

static uint8_t aaa=0;


#define GPIO_PortOutSet(g, p) GPIO->P[g].DOUT |= (1<<(p));
#define GPIO_PortOutClear(g, p) GPIO->P[g].DOUT &= ~(1<<(p));

void writedata(uint8_t d) {
	GPIO_PortOutSet(TFT_DC_PORT, TFT_DC_PIN);
	GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
	USART_SpiTransfer(USART1, d);
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
	USART_Tx(USART0, 'd');
}

void writecommand(uint8_t d) {
	GPIO_PortOutClear(TFT_DC_PORT, TFT_DC_PIN);
	GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
	USART_SpiTransfer(USART1, d);
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
	USART_Tx(USART0, 'c');
}

void display_loop() {
#if 0
	if(di_i < displayinit_len) {
		if(di_numargs <= 0) {
			int na;
			writecommand(displayinit[di_i++]);
			na = displayinit[di_i++];
			di_numargs = na & 0x7F;
			di_ms = na & 0x80;
		} else {
			writedata(displayinit[di_i++]);
			di_numargs--;
			if(di_numargs == 0 && di_ms)
				di_i++; // skip delay byte
		}
	} else {
#endif
	switch(di_i) {
	case 0:
		writecommand(0x01); di_i++; break;
	case 1:
		writecommand(0x11); di_i++; break;
	case 2:
		writecommand(0x29); di_i++; break;
	default:
		writecommand(0x2A); // column address set
		writedata(0);
		writedata(0);
		writedata(0);
		writedata(128);
		writecommand(0x2B); // row address set
		writedata(0);
		writedata(aaa & 127);
		writedata(0);
		writedata(160);
		writecommand(0x2C); // memory write
		unsigned i;
		for(i=0;i<300;i++)
			writedata(i);
		aaa++;
	}
}
