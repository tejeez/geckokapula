/*
 * ui.c
 *
 *  Created on: Nov 30, 2017
 *      Author: Tatu
 */

#include "display.h"

static uint8_t aaa=0, ttt=0;
void ui_loop() {
	unsigned i;
	display_init_loop();
	if(display_area(0, aaa, 128, aaa+1)) return;
	display_start();
	for(i=0;i<128;i++) {
		unsigned x = (aaa ^ (i + ttt)) & 0xFF;
		display_pixel(
			(x * ((ttt * 1234) & 0xFF00) >> 16),
			(x * ((ttt * 2345) & 0xFF00) >> 16),
			(x * ((ttt * 4321) & 0xFF00) >> 16));
	}
	display_end();
	aaa++;
	if(aaa >= 20) { aaa = 0; ttt++; }
}

int fftrow = 80;
#define FFTLEN 128
void ui_fft_line(float *data) {
	unsigned i;
	float mag[FFTLEN], mag_avg = 0;

	if(display_area(0,fftrow, 128,fftrow+1)) return;
	display_start();

#if 0
	for(i=0;i<FFTLEN;i++) {
		writedata(128 + 1000.0f*data[2*i]);
		writedata(128 + 1000.0f*data[2*i+1]);
		writedata(0);
	}
#else


	for(i=0;i<FFTLEN;i++) {
		float fft_i = data[2*i], fft_q = data[2*i+1];
		mag_avg +=
		mag[i ^ (FFTLEN/2)] = fft_i*fft_i + fft_q*fft_q;
	}
	mag_avg = (100.0f*FFTLEN) / mag_avg;

	for(i=0;i<FFTLEN;i++) {
		int mag_norm = mag[i] * mag_avg;
		int color1, color2;
		color1 = mag_norm >= 255 ? 255 : mag_norm;
		mag_norm /= 4;
		color2 = mag_norm >= 255 ? 255 : mag_norm;
		display_pixel(color2, color2, color1);
	}
#endif

	display_end();
	fftrow++;
	if(fftrow >= 160) fftrow = 20;
}
