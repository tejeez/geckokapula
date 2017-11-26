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
#include "rail.h"

static int di_i = 0;
static uint8_t aaa=0, ttt=0;

#define GPIO_PortOutSet(g, p) GPIO->P[g].DOUT |= (1<<(p));
#define GPIO_PortOutClear(g, p) GPIO->P[g].DOUT &= ~(1<<(p));

void writedata(uint8_t d) {
	GPIO_PortOutSet(TFT_DC_PORT, TFT_DC_PIN);
	GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
	USART_SpiTransfer(USART1, d);
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
	//USART_Tx(USART0, 'd');
}

void writecommand(uint8_t d) {
	GPIO_PortOutClear(TFT_DC_PORT, TFT_DC_PIN);
	GPIO_PortOutClear(TFT_CS_PORT, TFT_CS_PIN);
	USART_SpiTransfer(USART1, d);
	GPIO_PortOutSet(TFT_CS_PORT, TFT_CS_PIN);
	//USART_Tx(USART0, 'c');
}

void display_loop() {
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
		writedata(aaa>>8);
		writedata(aaa);
		writedata(0);
		writedata(160);
		writecommand(0x2C); // memory write
		unsigned i;
		for(i=0;i<128;i++) {
			/*writedata(aaa ^ i);
			writedata(aaa * i / 16);
			writedata(aaa - i);*/
			unsigned x = (aaa ^ (i + ttt)) & 0xFF;
			writedata(x * ((ttt * 1234) & 0xFF00) >> 16);
			writedata(x * ((ttt * 2345) & 0xFF00) >> 16);
			writedata(x * ((ttt * 4321) & 0xFF00) >> 16);
		}
		aaa++;
		if(aaa >= 80) { aaa = 0; ttt++; }
	}
}

int fftrow = 80;
#define FFTLEN 128
void display_fft_line(float *data) {
	writecommand(0x2A); // column address set
	writedata(0);
	writedata(0);
	writedata(0);
	writedata(128);
	writecommand(0x2B); // row address set
	writedata(fftrow>>8);
	writedata(fftrow);
	writedata(0);
	writedata(160);
	writecommand(0x2C); // memory write
	unsigned i;
	float mag[FFTLEN], mag_avg = 0;
	for(i=0;i<FFTLEN;i++) {
		float fft_i = data[2*i], fft_q = data[2*i+1];
		mag_avg +=
		mag[i] = fft_i*fft_i + fft_q*fft_q;
	}
	mag_avg = (100.0f*FFTLEN) / mag_avg;
	for(i=0;i<FFTLEN;i++) {
		int mag_norm = mag[i] * mag_avg;
		writedata(mag_norm < 255 ? mag_norm : 255);
		mag_norm /= 4;
		writedata(mag_norm < 255 ? mag_norm : 255);
		mag_norm /= 4;
		writedata(mag_norm < 255 ? mag_norm : 255);
	}
	fftrow++;
	if(fftrow >= 160) fftrow = 80;
}
