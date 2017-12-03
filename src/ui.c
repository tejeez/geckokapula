/*
 * ui.c
 *
 *  Created on: Nov 30, 2017
 *      Author: Tatu
 */

#include "display.h"
#include "font8x8_basic.h"
#include "rig.h"
#include "ui_hw.h"
#include <stdio.h>
#include <math.h>

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
char textline[TEXT_LEN+1] = "geckokapula";
int text_hilight = 0;

int ui_cursor = 6;

const char *p_mode_names[] = { " FM", "DSB" };

void ui_update_text() {
	int i;
	int s_dB = 10.0*log10(p.smeter);
	i = snprintf(textline, TEXT_LEN+1, "%10u %s %3d",
			(unsigned)p.frequency, p_mode_names[p.mode], s_dB);
	for(; i<TEXT_LEN; i++) textline[i] = ' ';
	text_hilight = ui_cursor;
}

static int wrap(int a, int b) {
	while(a < 0) a += b;
	while(a >= b) a -= b;
	return a;
}

// count only every 4th position
#define ENCODER_DIVIDER 4

void ui_check_buttons() {
	const int steps[] = { 1e9, 1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1, 1 };
	static unsigned pos_prev;
	int pos_now, pos_diff;
	pos_now = get_encoder_position() / ENCODER_DIVIDER;
	pos_diff = pos_now - pos_prev;
	if(pos_diff) {
		if(pos_diff >= 0x8000 / ENCODER_DIVIDER)
			pos_diff -= 0x10000 / ENCODER_DIVIDER;
		else if(pos_diff < -0x8000 / ENCODER_DIVIDER)
			pos_diff += 0x10000 / ENCODER_DIVIDER;

		if(get_encoder_button()) {
			ui_cursor = wrap(ui_cursor + pos_diff, TEXT_LEN);
		} else {
			if(ui_cursor <= 9) {
				p.frequency += pos_diff * steps[ui_cursor];
				p.channel_changed = 1;
			} else if(ui_cursor >= 11 && ui_cursor <= 13) {
				p.mode = wrap(p.mode + pos_diff, 2);
			}
		}
	}

	pos_prev = pos_now;
}


void ui_loop() {
	ui_check_buttons();
	display_init_loop();
	if(!display_ready()) return;
#if 0
	unsigned i;
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
	if(aaa >= TEXT_LEN) {
		ui_update_text();
		aaa = 0; ttt++;
	}
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
