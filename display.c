/*
 * display.c
 *
 *  Created on: Sep 24, 2017
 *      Author: Tatu
 */


#include <stdint.h>
#include "em_usart.h"
#include "em_gpio.h"
#include "InitDevice.h"

extern uint8_t displayinit[];
extern const int displayinit_len;
static int di_i = 0, di_numargs = 0, di_ms = 0;

static uint8_t aaa=0;

#define SPI_DC_CMD 0
#define SPI_DC_DATA 1
#define SPI_DC_NONE 2

void display_loop() {
	int spi_out, spi_dc = SPI_DC_NONE;
	if(di_i < displayinit_len) {
		if(di_numargs <= 0) {
			int na;
			spi_out = displayinit[di_i++];
			spi_dc = SPI_DC_CMD;
			na = displayinit[di_i++];
			di_numargs = na & 0x7F;
			di_ms = na & 0x80;
		} else {
			spi_out = displayinit[di_i++];
			spi_dc = SPI_DC_DATA;
			di_numargs--;
			if(di_numargs == 0 && di_ms)
				di_i++; // skip delay byte
		}
	} else {
		if(aaa == 0) {
			spi_out = 0x2C; // memory write command
			spi_dc = SPI_DC_CMD;
		} else {
			spi_out = 123;
			spi_dc = SPI_DC_DATA;
		}
		aaa++;
	}
	if(spi_dc != SPI_DC_NONE) {
		if(spi_dc)
			GPIO_PortOutSet(TFT_DC_PORT, TFT_DC_PIN);
		else
			GPIO_PortOutClear(TFT_DC_PORT, TFT_DC_PIN);
		GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
		USART_SpiTransfer(USART1, spi_out);
		GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
		USART_Tx(USART0, 's');
	}
}
