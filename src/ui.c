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
#include "ui_parameters.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

#define BACKLIGHT_ON_TIME 2000
#define BACKLIGHT_DIM_LEVEL 50
int backlight_timer = 0;

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

	//display_area(y1, x1, y1+7, x1+7);
	display_area(x1, y1, x1+7, y1+7);
	display_start();
	char *font = font8x8_basic[c];
	uint8_t *bufp = displaybuf;
	/*for(x=0; x<8; x++) {
		for(y=7; y>=0; y--) {*/
	for(y=0; y<8; y++) {
		for(x=0; x<8; x++) {
			if(font[y] & (1<<x)) {
				if(highlighted)
					display_buf_pixel(0,0,0);
				else
					display_buf_pixel(128,255,128);
			} else {
				if(highlighted)
					display_buf_pixel(255,255,255);
				else
					display_buf_pixel(0,0,128);
			}
		}
	}
	display_transfer(displaybuf, 3*8*8);
}

#define TEXT_LEN 40
char textline[TEXT_LEN+1] = "geckokapula";
char textprev[TEXT_LEN+1] = "";

static unsigned char aaa = 0, ui_cursor = 6, ui_keyed = 0;

const char *p_mode_names[] = { " FM", " AM", "DSB", "---" };
const char *p_keyed_text[] = { "rx", "tx" };

typedef struct {
	char pos1, pos2, color;
} ui_field_t;
#define N_UI_FIELDS 13
const ui_field_t ui_fields[N_UI_FIELDS] = {
	{ 0, 0, 0 },
	{ 1, 1, 0 },
	{ 2, 2, 0 },
	{ 3, 3, 0 },
	{ 4, 4, 0 },
	{ 5, 5, 0 },
	{ 6, 6, 0 },
	{ 7, 7, 0 },
	{ 8, 8, 0 },
	{ 9, 9, 0 },
	{11,13, 1 },
	{14,15, 2 },
	{16,17, 1 }
};

extern int testnumber;
void ui_update_text() {
	int i;
	int pos1, pos2;
	int s_dB = 10.0*log10(rs.smeter);

	i = snprintf(textline, TEXT_LEN+1, "%10u %3s%2s%2d %2d %6d                ",
			(unsigned)p.frequency, p_mode_names[p.mode], p_keyed_text[(int)p.keyed],
			p.volume,
			s_dB, testnumber);
	for(; i<TEXT_LEN; i++) textline[i] = ' ';

	pos1 = ui_fields[ui_cursor].pos1;
	pos2 = ui_fields[ui_cursor].pos2;
	for(i=pos1; i<=pos2; i++) textline[i] |= 0x80;
}


static void ui_knob_turned(int cursor, int diff) {
	if(cursor >= 0 && cursor <= 9) { // frequency
		const int steps[] = { 1e9, 1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1, 1 };
		p.frequency += diff * steps[(int)ui_cursor];
		p.channel_changed = 1;
	} else if(cursor == 10) { // mode
		p.mode = wrap(p.mode + diff, sizeof(p_mode_names) / sizeof(p_mode_names[0]));
	} else if(cursor == 11) { // keyed
		ui_keyed = wrap(ui_keyed + diff, 2);
	} else if(cursor == 12) { // volume
		p.volume = wrap(p.volume + diff, 12);
	}
}

// count only every 4th position
#define ENCODER_DIVIDER 4

void ui_check_buttons() {
	static unsigned pos_prev;
	int pos_now, pos_diff;
	char button = get_encoder_button();
	p.keyed = ui_keyed ? 1 : get_ptt();
	pos_now = get_encoder_position() / ENCODER_DIVIDER;
	pos_diff = pos_now - pos_prev;
	if(button)
		backlight_timer = 0;
	if(pos_diff) {
		if(pos_diff >= 0x8000 / ENCODER_DIVIDER)
			pos_diff -= 0x10000 / ENCODER_DIVIDER;
		else if(pos_diff < -0x8000 / ENCODER_DIVIDER)
			pos_diff += 0x10000 / ENCODER_DIVIDER;

		if(button) {
			ui_cursor = wrap(ui_cursor + pos_diff, N_UI_FIELDS);
		} else {
			ui_knob_turned(ui_cursor, pos_diff);
		}
		backlight_timer = 0;
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
	if(backlight_timer <= BACKLIGHT_ON_TIME) {
		display_backlight(BACKLIGHT_DIM_LEVEL + BACKLIGHT_ON_TIME - backlight_timer);
		backlight_timer++;
	}

	if(fftline_ready) {
		ui_draw_fft_line(fftline_data);
		fftline_ready = 0;
		return;
	}

	/* Update text when starting to draw next line of text.
	 * Find the next character to be drawn
	 * and draw one character per call.
	 */
	if(aaa == 0)
		ui_update_text();

	while(textline[aaa] == textprev[aaa] && aaa < TEXT_LEN)
		aaa++;

	if(aaa < TEXT_LEN/* && textline[aaa] != textprev[aaa]*/) {
		char c = textline[aaa];
		if(aaa < 16) // first line
			ui_character(aaa*8, /*128-8*/0, c&0x7F, (c&0x80) != 0);
		else // second line
			ui_character((aaa-16)*8, /*128-16*/8, c&0x7F, (c&0x80) != 0);
		textprev[aaa] = c;
	}

	if(aaa >= TEXT_LEN)
		aaa = 0;
}

int fftrow = FFT_ROW2;
#define FFT_BIN1 64
#define FFT_BIN2 192
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
	display_scroll(fftrow);
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

	fftrow--;
	if(fftrow < FFT_ROW1) fftrow = FFT_ROW2;
}


void ui_task() {
	/* TODO: Put display control and button check in different tasks?
	 * Have to ensure that they are thread safe first.
	 */
	for(;;) {
		ui_loop();
		vTaskDelay(2);
	}
}
