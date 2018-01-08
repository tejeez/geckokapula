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
#include <string.h>
#include <math.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

static uint8_t aaa=0, ttt=0;

#define DISPLAYBUF_SIZE 384
uint8_t displaybuf[DISPLAYBUF_SIZE];

#if DISPLAYBUF_SIZE < 3*8*8
#error "Too small display buffer for text"
#endif

static int wrap(int a, int b) {
	while(a < 0) a += b;
	while(a >= b) a -= b;
	return a;
}
#define display_buf_pixel(r,g,b) do{ *bufp++ = r; *bufp++ = g; *bufp++ = b; }while(0)

void ui_character(int x1, int y1, unsigned char c, int highlighted) {
	int x, y;
	if(!display_ready()) return;

	uint8_t bgb = (x1 + ttt) & 0x7F;
	if(bgb > 0x40) bgb = 0x80 - bgb;

	display_area(y1, x1, y1+7, x1+7);
	display_start();
	char *font = font8x8_basic[c];
	uint8_t *bufp = displaybuf;
	for(x=0; x<8; x++) {
		for(y=7; y>=0; y--) {
			if(font[y] & (1<<x)) {
				if(highlighted)
					display_buf_pixel(0,0,0);
				else
					display_buf_pixel(128,255,128);
			} else {
				if(highlighted)
					display_buf_pixel(255,255,255);
				else
					display_buf_pixel(0,0,bgb);
			}
		}
	}
	display_transfer(displaybuf, 3*8*8);
}

#define TEXT_LEN 40
char textline[TEXT_LEN+1] = "geckokapula";
int text_hilight = 0;

int ui_cursor = 6;
static char ui_keyed = 0;

const char *p_mode_names[] = { " FM", "DSB" };
const char *p_keyed_text[] = { "rx", "tx" };

extern int testnumber;
void ui_update_text() {
	int i;
	int s_dB = 10.0*log10(p.smeter);
	for(i=0; i<TEXT_LEN; i++) textline[i] = ' ';
	snprintf(textline, 21, "%10u %s %s",
			(unsigned)p.frequency, p_mode_names[p.mode], p_keyed_text[(int)p.keyed]);
	snprintf(textline+20, 21, "%6d   %2d", testnumber, s_dB);
	text_hilight = ui_cursor;
}


static void ui_knob_turned(int cursor, int diff) {
	if(cursor >= 0 && cursor <= 9) { // frequency
		const int steps[] = { 1e9, 1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1, 1 };
		p.frequency += diff * steps[ui_cursor];
		p.channel_changed = 1;
	} else if(cursor >= 11 && cursor <= 13) { // mode
		p.mode = wrap(p.mode + diff, 2);
	} else if(cursor >= 15 && cursor <= 16) {
		ui_keyed = wrap(ui_keyed + diff, 2);
	}
}

// count only every 4th position
#define ENCODER_DIVIDER 4

void ui_check_buttons() {
	static unsigned pos_prev;
	int pos_now, pos_diff;
	p.keyed = ui_keyed ? 1 : get_ptt();
	pos_now = get_encoder_position() / ENCODER_DIVIDER;
	pos_diff = pos_now - pos_prev;
	if(pos_diff) {
		if(pos_diff >= 0x8000 / ENCODER_DIVIDER)
			pos_diff -= 0x10000 / ENCODER_DIVIDER;
		else if(pos_diff < -0x8000 / ENCODER_DIVIDER)
			pos_diff += 0x10000 / ENCODER_DIVIDER;

		if(get_encoder_button()) {
			ui_cursor = wrap(ui_cursor + pos_diff, /*TEXT_LEN*/20);
		} else {
			ui_knob_turned(ui_cursor, pos_diff);
		}
	}

	pos_prev = pos_now;
}


static float fftline_data[2*FFTLEN];
static char fftline_ready=0;
static void ui_draw_fft_line(float *data);

void ui_loop() {
	ui_check_buttons();
	display_init_loop();
	if(!display_ready()) return;

	if(fftline_ready) {
		ui_draw_fft_line(fftline_data);
		fftline_ready = 0;
	}

	if(aaa < 20) // first line
		ui_character(aaa*8, 128-8, textline[aaa], aaa == text_hilight);
	else // second line
		ui_character((aaa-20)*8, 128-16, textline[aaa], aaa == text_hilight);
	aaa++;
	if(aaa >= TEXT_LEN) {
		ui_update_text();
		aaa = 0; ttt++;
	}
}

int fftrow = 0;
//#define FFTLEN 128
#define FFT_BIN1 8
#define FFT_BIN2 120
#if DISPLAYBUF_SIZE < 3*(FFT_BIN2-FFT_BIN1)
#error "Too small display buffer for FFT"
#endif

void ui_fft_line(float *data) {
	/* This is called from another task.
	 * Copy data to an intermediate buffer so that only the display task
	 * writes to the display DMA buffer and controls the display.
	 */
	if(fftline_ready) return;
	memcpy(fftline_data, data, 2*FFTLEN*sizeof(float));
	fftline_ready = 1;
}

static void ui_draw_fft_line(float *data) {
	unsigned i;
	float mag[FFTLEN], mag_avg = 0;

	if(!display_ready()) return;
	display_area(0,fftrow, FFT_BIN2-FFT_BIN1, fftrow);
	display_start();

	for(i=0;i<FFTLEN;i++) {
		float fft_i = data[2*i], fft_q = data[2*i+1];
		mag_avg +=
		mag[i ^ (FFTLEN/2)] = fft_i*fft_i + fft_q*fft_q;
	}
	mag_avg = (70.0f*FFTLEN) / mag_avg;

	uint8_t *bufp = displaybuf;
	for(i=FFT_BIN1;i<FFT_BIN2;i++) {
		int mag_norm = mag[i] * mag_avg;
		bufp[2] = mag_norm >= 255 ? 255 : mag_norm;
		mag_norm /= 2;
		bufp[1] = mag_norm >= 255 ? 255 : mag_norm;
		mag_norm /= 2;
		bufp[0] = mag_norm >= 255 ? 255 : mag_norm;
		bufp+=3;
	}

	display_transfer(displaybuf, 3*(FFT_BIN2-FFT_BIN1));

	fftrow++;
	if(fftrow >= 160) fftrow = 0;
}


void ui_task() {
	/* TODO: Put display control and button check in different tasks?
	 * Have to ensure that they are thread safe first.
	 * Could also start using semaphores or timers instead of just yielding
	 * for more efficient CPU use and the possibility to have different
	 * task priorities. */
	for(;;) {
		ui_loop();
		taskYIELD();
	}
}
