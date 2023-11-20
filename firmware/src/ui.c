/* SPDX-License-Identifier: MIT */

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "display.h"
#include "rig.h"
#include "ui.h"
#include "ui_hw.h"
#include "ui_parameters.h"
#include "dsp.h"
#include "power.h"
#include "railtask.h"
#include "config.h"

#include "font8x8_basic.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

rig_parameters_t p = {
	.keyed = 0,
	.mode = MODE_FM,
	.frequency = RIG_DEFAULT_FREQUENCY,
	.split_freq = 0,
	.offset_freq = 0,
	.volume = 10,
	.waterfall_averages = 20,
	.squelch = 15
};
rig_status_t rs = {0};

#define BACKLIGHT_ON_TIME 2000
#define BACKLIGHT_DIM_LEVEL 50
int backlight_timer = 0;

#define DISPLAYBUF_SIZE 384
#define DISPLAYBUF2_SIZE 384
uint8_t displaybuf[DISPLAYBUF_SIZE], displaybuf2[DISPLAYBUF2_SIZE];

volatile struct display_ev display_ev;
SemaphoreHandle_t display_sem;

#if DISPLAYBUF_SIZE < 3*8*8
#error "Too small display buffer for text"
#endif

// Wrap number between 0 and b-1
static int wrap(int a, int b) {
	while(a < 0) a += b;
	while(a >= b) a -= b;
	return a;
}

// Wrap number between -b and b-1
static int wrap_signed(int a, int b) {
	while(a < -b) a += 2*b;
	while(a >= b) a -= 2*b;
	return a;
}

#define display_buf_pixel(r,g,b) do{ *bufp++ = r; *bufp++ = g; *bufp++ = b; }while(0)

void ui_character(int x1, int y1, unsigned char c, int highlighted) {
	int x, y;
	if(!display_ready()) return;

	display_area(x1, y1, x1+7, y1+7);
	display_start();
	const char *font = font8x8_basic[c];
	uint8_t *bufp = displaybuf;
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

#define TEXT_LEN 64
char textline[TEXT_LEN+1] = "geckokapula";
char textprev[TEXT_LEN+1] = "";

static unsigned char ui_cursor = 6, ui_keyed = 0;

const char *p_mode_names[] = { "---", " FM", " AM", "SSB", "---", "off" };
const char *p_keyed_text[] = { "rx", "tx" };

typedef struct {
	char pos1, pos2, color;
	const char *tip;
} ui_field_t;
#define N_UI_FIELDS 21
const ui_field_t ui_fields[N_UI_FIELDS] = {
	{ 0, 0, 0, "Freq GHz" },
	{ 1, 1, 0, "Freq 100 MHz" },
	{ 2, 2, 0, "Freq 10 MHz"},
	{ 3, 3, 0, "Freq MHz" },
	{ 4, 4, 0, "Freq 100 kHz" },
	{ 5, 5, 0, "Freq 10 kHz" },
	{ 6, 6, 0, "Freq kHz" },
	{ 7, 7, 0, "Freq 100 Hz" },
	{ 8, 8, 0, "Freq 10 Hz" },
	{ 9, 9, 0, "Freq 1 Hz" },
	{11,13, 1, "Mode" }, // mode
	{14,15, 2, "PTT" }, // rx/tx
	{16,17, 1, "Volume" }, // volume
	{19,20, 2, "Waterfall" }, // averages
	{22,23, 1, "Squelch" }, // squelch
	{25,27, 1, "TX split MHz"},
	{28,28, 1, "TX split 100 kHz"},
	{32,34, 0, "SSB finetune kHz" }, // offset frequency
	{35,35, 0, "Finetune 100 Hz" }, // offset frequency
	{36,36, 0, "Finetune 10 Hz" }, // offset frequency
	{37,37, 0, "SSB finetune Hz" }, // offset frequency
};

void ui_update_text() {
	int i;
	int pos1, pos2;
	int s_dB = 10.0*log10(rs.smeter);

	int split = p.split_freq;
	int keyed = p.keyed;
	enum rig_mode mode = p.mode;

	unsigned freq = p.frequency;
	if (keyed)
		freq += p.split_freq;
	if (p.mode == MODE_DSB)
		freq += p.offset_freq;

	i = snprintf(textline, TEXT_LEN+1, "%10u %3s%2s%2d %2d %2d %4d   %6d RSSI: %2d",
			freq, p_mode_names[mode], p_keyed_text[keyed],
			p.volume, p.waterfall_averages, p.squelch,
			split / 100000,
			(int)p.offset_freq,
			s_dB);
	for (; i < 48; i++) textline[i] = ' ';
	i = 48 + snprintf(textline + 48, TEXT_LEN+1-48, "%s",
		ui_fields[ui_cursor].tip);
	for(; i<TEXT_LEN; i++) textline[i] = ' ';

	pos1 = ui_fields[ui_cursor].pos1;
	pos2 = ui_fields[ui_cursor].pos2;
	for(i=pos1; i<=pos2; i++) textline[i] |= 0x80;
}


static const int ui_steps[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9 };
static void ui_knob_turned(int cursor, int diff)
{
	if(cursor >= 0 && cursor <= 9) { // frequency
		p.frequency += diff * ui_steps[9 - ui_cursor];
		xSemaphoreGive(railtask_sem);
	} else if(cursor == 10) { // mode
		p.mode = wrap(p.mode + diff, sizeof(p_mode_names) / sizeof(p_mode_names[0]));
		dsp_update_params();
	} else if(cursor == 11) { // keyed
		ui_keyed = wrap(ui_keyed + diff, 2);
	} else if(cursor == 12) { // volume
		p.volume = wrap(p.volume + diff, 20);
		dsp_update_params();
	} else if(cursor == 13) {
		p.waterfall_averages = wrap(p.waterfall_averages + diff, 100);
	} else if(cursor == 14) {
		p.squelch = wrap(p.squelch + diff, 100);
		dsp_update_params();
	} else if(cursor >= 15 && cursor <= 16) {
		int step = cursor == 15 ? 1000000 : 100000;
		p.split_freq = wrap_signed(p.split_freq + step * diff, 100000000);
		xSemaphoreGive(railtask_sem);
	} else if(cursor >= 17 && cursor <= 20) {
		p.offset_freq = wrap_signed(p.offset_freq + ui_steps[20-cursor] * diff, 10000);
		dsp_update_params();
	}
}

// count only every 4th position
#define ENCODER_DIVIDER 4


/* ui_check_buttons is regularly called from the misc task.
 * TODO: think about thread safety when other tasks
 * read the updated data */
void ui_check_buttons(void)
{
	static unsigned pos_prev;
	static unsigned char button_prev, ptt_prev, keyed_prev;
	int pos_now, pos_diff;
	char button = get_encoder_button(), ptt = get_ptt();
	pos_now = get_encoder_position() / ENCODER_DIVIDER;
	pos_diff = pos_now - pos_prev;

	if (p.mode == MODE_OFF && button_prev && (!button)) {
		// Shut down after button has been released.
		shutdown();
	}

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
	if (pos_diff != 0 || ptt != ptt_prev) {
		if (tx_freq_allowed(p.frequency + p.split_freq)) {
			p.keyed = ui_keyed || ptt;
		} else {
			p.keyed = 0;
			ui_keyed = 0;
		}
		if (p.keyed != keyed_prev)
			xSemaphoreGive(railtask_sem);
		keyed_prev = p.keyed;


		/* Something on the display may have changed at this point,
		 * so make the display task check for that. */
		display_ev.text_changed = 1;
		xSemaphoreGive(display_sem);
	}

	pos_prev = pos_now;
	ptt_prev = ptt;
	button_prev = button;
}


/* ui_control_backlight is regularly called from the misc task. */
void ui_control_backlight(void)
{
	if(backlight_timer <= BACKLIGHT_ON_TIME) {
		display_backlight(BACKLIGHT_DIM_LEVEL + BACKLIGHT_ON_TIME - backlight_timer);
		backlight_timer++;
	}
}


int fftrow = FFT_ROW2;
#if DISPLAYBUF2_SIZE < 3*(FFT_BIN2-FFT_BIN1)
#error "Too small display buffer for FFT"
#endif

/* Check for the waterfall line flag and draw the line.
 * If the flag is not set, just return. */
static void ui_display_waterfall(void)
{
	if (!display_ev.waterfall_line)
		return;
	display_ev.waterfall_line = 0;
	if (!display_ready()) {
		printf("Bug? Display not ready in waterfall\n");
		return;
	}
	display_scroll(fftrow);
	display_area(0,fftrow, FFT_BIN2-FFT_BIN1, fftrow);
	display_start();
	display_transfer(displaybuf2, 3*(FFT_BIN2-FFT_BIN1));

	fftrow--;
	if(fftrow < FFT_ROW1) fftrow = FFT_ROW2;
}


static const uint8_t offset_cursor_data[3*9] = {
	255,255,  0,  255,255,  0,  255,255,  0,
	  0,255,  0,  255,255,  0,    0,255,  0,
	  0,  0,  0,    0,255,255,    0,  0,  0
};

/* Draw the offset frequency cursor above waterfall */
void ui_display_offset_cursor(void)
{
	display_area(0, 24, 127, 26);
	display_start();
	// First 33*8 bytes of font data is zeros
	int i;
	for (i = 0; i < 128 * 3 * 3 / (8*16); i++)
		display_transfer((const uint8_t*)font8x8_basic[0], 8*16);

	// Calculate the position based on sample rate and FFT size
	int x = 64 + p.offset_freq * 256 / (RX_IQ_FS/2);
	if (x < 1) x = 1;
	if (x > 127) x = 127;
	display_area(x-1, 24, x+1, 26);
	display_start();
	display_transfer(offset_cursor_data, 3*9);
}


/* Update text on the display.
 *
 * To make both the text and the waterfall respond fast
 * for smooth user experience, draw the text one character
 * at a time and check for a possible new waterfall line
 * in between drawing each character.
 * Also update only the characters that have changed. */
static void ui_display_text(void)
{
	ui_update_text();
	int i;
	for (i = 0; i < TEXT_LEN; i++) {
		char c = textline[i], cp = textprev[i];
		if (c != cp) {
			if (i < 16) // first line
				ui_character(i*8, 0, c&0x7F, (c&0x80) != 0);
			else if (i < 32) // second line
				ui_character((i-16)*8, 8, c&0x7F, (c&0x80) != 0);
			else if(i < 48) // third line
				ui_character((i-32)*8, 16, c&0x7F, (c&0x80) != 0);
			else // bottom line
				ui_character((i-48)*8, 160-8, c&0x7F, (c&0x80) != 0);
			textprev[i] = c;
			ui_display_waterfall();
		}
	}
	ui_display_offset_cursor();
}


void display_task(void *arg)
{
	(void)arg;
	display_init();
	for (;;) {
		xSemaphoreTake(display_sem, portMAX_DELAY);
		ui_display_waterfall();
		if (display_ev.text_changed) {
			display_ev.text_changed = 0;
			ui_display_text();
		}
	}
}


/* Create the RTOS objects needed by the user interface.
 * Called before starting the scheduler. */
void ui_rtos_init(void)
{
	display_sem = xSemaphoreCreateBinary();
}
