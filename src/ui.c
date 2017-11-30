/*
 * ui.c
 *
 *  Created on: Nov 30, 2017
 *      Author: Tatu
 */

#include "display.h"
#include "font8x8_basic.h"

static uint8_t aaa=0, ttt=0;

void ui_character(int x1, int y1, unsigned char c, int highlighted) {
	int x, y;
	uint8_t bgb = ttt % 64;
	if(!display_ready()) return;

	display_area(y1, x1, y1+7, x1+7);
	display_start();
	char *font = font8x8_basic[c];
	for(x=0; x<8; x++) {
		for(y=7; y>=0; y--) {
			if(font[y] & (1<<x)) {
				if(highlighted)
					display_pixel(0,0,0);
				else
					display_pixel(128,255,128);
			} else {
				if(highlighted)
					display_pixel(255,255,255);
				else
					display_pixel(0,0,bgb);
			}
		}
	}
	display_end();
}

#define TEXT_LEN 20
char textline[TEXT_LEN] = "geckokapula         ";
int text_hilight = 6;

void ui_loop() {
	unsigned i;
	display_init_loop();
	if(!display_ready()) return;
#if 0
	display_area(96, aaa, 128, aaa+1);
	display_start();
	for(i=0;i<32;i++) {
		unsigned x = (aaa ^ (i + ttt)) & 0xFF;
		display_pixel(
			(x * ((ttt * 1234) & 0xFF00) >> 16),
			(x * ((ttt * 2345) & 0xFF00) >> 16),
			(x * ((ttt * 4321) & 0xFF00) >> 16));
	}
	display_end();
	aaa++;
	if(aaa >= 160) { aaa = 0; ttt++; }
#endif
	ui_character(aaa*8, 128-8, textline[aaa], aaa == text_hilight);
	aaa++;
	if(aaa >= TEXT_LEN) { aaa = 0; ttt++; }
}

int fftrow = 80;
#define FFTLEN 128
#define FFT_BIN1 4
#define FFT_BIN2 124
void ui_fft_line(float *data) {
	unsigned i;
	float mag[FFTLEN], mag_avg = 0;

	display_area(0,fftrow, FFT_BIN2-FFT_BIN1, fftrow);
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

	for(i=FFT_BIN1;i<FFT_BIN2;i++) {
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
	if(fftrow >= 160) fftrow = 0;
}
