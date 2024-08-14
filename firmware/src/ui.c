/* SPDX-License-Identifier: MIT */
// OH2BTG versions. tehty volumesäätö joka tottelee dBm arvoa
// E10 kokeilua isommilla fonteilla
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
	.split_freq = -2000000,
	.offset_freq = 0,
	.volume = 11,
	.waterfall_averages = 20,
	.squelch = 14,
	.ctcss = 118,
};
rig_status_t rs = {0};

// CTCSS frequencies in multiples of 0.1 Hz
// List from https://en.wikipedia.org/wiki/Continuous_Tone-Coded_Squelch_System#List_of_tones
const uint16_t ctcss_freqs[] = {
	0, 670, 693, 719, 744, 770, 797, 825, 854, 885, 915, 948, 974, 1000, 1035, 1072, 1109, 1148, 1188, 1230, 1273, 1318, 1365, 1413, 1462, 1500, 1514, 1567, 1598, 1622, 1655, 1679, 1713, 1738, 1773, 1799, 1835, 1862, 1899, 1928, 1966, 1995, 2035, 2065, 2107, 2181, 2257, 2291, 2336, 2418, 2503, 2541
};


enum ui_field_name {
	// Common fields

	UI_FIELD_FREQ0,
	UI_FIELD_FREQ1,
	UI_FIELD_FREQ2,
	UI_FIELD_FREQ3,
	UI_FIELD_FREQ4,
	UI_FIELD_FREQ5,
	UI_FIELD_FREQ6,
	UI_FIELD_FREQ7,
	UI_FIELD_FREQ8,
	UI_FIELD_FREQ9,
	UI_FIELD_MODE,
	UI_FIELD_PTT,
	UI_FIELD_VOL,
	UI_FIELD_WF, // Waterfall averages
	UI_FIELD_SPLIT0,
	UI_FIELD_SPLIT1,
	UI_FIELD_MEMO,

	// FM specific fields

	UI_FIELD_SQ, // Squelch
	UI_FIELD_CTCSS,

	// SSB specific fields

	// SSB finetune
	UI_FIELD_FT0,
	UI_FIELD_FT1,
	UI_FIELD_FT2,
	UI_FIELD_FT3,
};

struct ui_field {
	enum ui_field_name name;
	unsigned char pos1, pos2, color;
	const char *tip;
};

struct ui_view {
	// Number of fields
	size_t n;
	// Function to format text for the view
	int (*text)(char*, size_t);
	// Array of fields
	struct ui_field fields[];
};

#define TEXT_LEN 96 // kokeilua OH2BTG oli 64

struct ui_state {
	// Current UI view
	const struct ui_view *view;
	// Previous encoder position
	unsigned pos_prev;
	int backlight_timer;
	// Number of currently selected field
	unsigned char cursor;
	unsigned char keyed;
	unsigned char button_prev, ptt_prev, keyed_prev;
	// Index to ctcss_freqs
	unsigned char ctcss;
	char text[TEXT_LEN+1];
	unsigned char color[TEXT_LEN+1];
	char textprev[TEXT_LEN+1];
	unsigned char colorprev[TEXT_LEN+1];
};

struct ui_state ui;

// UI fields common to every mode.
// To simplify code, these are repeated in every ui_view struct. x-alku,x-loppu,digits
#define UI_FIELDS_COMMON \
	{ UI_FIELD_FREQ0,     0, 0, 3, "Freq GHz"         },\
	{ UI_FIELD_FREQ1,     1, 1, 2, "Freq 100 MHz"     },\
	{ UI_FIELD_FREQ2,     2, 2, 2, "Freq 10 MHz"      },\
	{ UI_FIELD_FREQ3,     3, 3, 2, "Freq MHz"         },\
	{ UI_FIELD_FREQ4,     4, 4, 3, "Freq 100 kHz"     },\
	{ UI_FIELD_FREQ5,     5, 5, 3, "Freq 10 kHz"      },\
	{ UI_FIELD_FREQ6,     6, 6, 3, "Freq kHz"         },\
	{ UI_FIELD_FREQ7,     7, 7, 2, "Freq 100 Hz"      },\
	{ UI_FIELD_FREQ8,     8, 8, 2, "Freq 10 Hz"       },\
	{ UI_FIELD_FREQ9,     9, 9, 2, "Freq 1 Hz"        },\
	{ UI_FIELD_MODE,     17,19, 3, "Mode"             },\
	{ UI_FIELD_PTT,      21,22, 2, "PTT"              },\
	{ UI_FIELD_VOL,      24,25, 3, "Volume"           },\
	{ UI_FIELD_WF,       27,28, 2, "Waterfall"        },\
	{ UI_FIELD_SPLIT0,   31,31, 3, "TX split MHz"     },\
	{ UI_FIELD_SPLIT1,   32,32, 3, "TX split 100 kHz" },\
	{ UI_FIELD_MEMO,   	 44,46, 3, "V.012 OH2BTG"     }
#define UI_FIELDS_COMMON_N 17
// sain lisättyä allaolevaan kentän TST OH2BTG
static const char *const p_mode_names[] = {
	"E10", " FM", " AM", "USB", "LSB", "CWU", "CWL", "---", "off","TST" // OH2BTG ver ja TST mode
};

enum memo {
    MEMO_M1,  // 0
    MEMO_M2,  // 1
    MEMO_M3,  // 2
    MEMO_M4,  // 3
    MEMO_M5,  // 4
    MEMO_M6,  // 5
    MEMO_M7,  // 6
    MEMO_M8,  // 7
    MEMO_M9,  // 8
    MEMO_M0   // 9
};

struct Params {
 
    enum memo memo;
} m;
static const char *const m_memo_names[] = {
    "M1", "M2", "M3", "M4", "M5", "M6", "M7", "M8", "M9", "M0"
};
static int wrap_int(int value, int max) {
    if (value < 0) {
        return max - 1;
    } else if (value >= max) {
        return 0;
    }
    return value;
}
void update_display() {
    // Placeholder for display update logic
    printf("Display updated for memo: %s\n", m_memo_names[m.memo]);
}
void update_memo(int diff) {
    m.memo = wrap_int(m.memo + diff, sizeof(m_memo_names) / sizeof(m_memo_names[0]));

    update_display(); // Update display to reflect new memo

    printf("Selected Memo: %s\n", m_memo_names[m.memo]);
}
static const char *const p_keyed_text[] = { "rx", "tx" };
// Format text common for all views
static int ui_view_common_text(char *text, size_t maxlen){
	unsigned freq = p.frequency;
	int split = p.split_freq;
	int keyed = p.keyed;
	enum rig_mode mode = p.mode;
	int S_dB = ((10.0*log10(rs.smeter)) - 147); // OH2BTG Kaava muutettu dBm
	/*	if (S_dB > -90) {
				p.volume = 13;}  // tehty volumesäätö joka tottelee dBm arvoa
			    else {		
					p.volume = 12;
				}*/
				dsp_update_params();
	printf("Vol saato toimii: %ld\n", S_dB);
		if (keyed)
		freq += p.split_freq;
	if (mode >= MODE_USB && mode <= MODE_CWL)
		freq += p.offset_freq;

	return snprintf(text, maxlen,
		"%10u%4idBm%3s %2s "
		"%2d %2d%4d",
		freq, S_dB, p_mode_names[mode], p_keyed_text[keyed],
		p.volume, p.waterfall_averages, split / 100000 // OH2BTG lisätty dBm teksti
		);
	
	
	
	
}

// Format text specific to FM view
static int ui_view_fm_text(char *text, size_t maxlen)
{
	int ctcss = ctcss_freqs[ui.ctcss];
	if (ctcss != 0) {
		return snprintf(text, maxlen,
			"%2d%4d.%1d",  // OH2BTG kenttää säädetty ctcss on 3 rivillä
			p.squelch,
			ctcss / 10, ctcss % 10
		);
	} else {
		return snprintf(text, maxlen,
			"%2d off ",
			p.squelch
		);
	}
};

const struct ui_view ui_view_fm = {
	UI_FIELDS_COMMON_N + 2,
	ui_view_fm_text,
	{
	UI_FIELDS_COMMON,
	{ UI_FIELD_SQ,       33,34, 2, "Squelch"          },// OH2BTG tässä saa siirrettyä kursorin oikealle paikalle
	{ UI_FIELD_CTCSS,    36,40, 5, "CTCSS Hz"         },
	}
};

// Format text specific to SSB view
static int ui_view_ssb_text(char *text, size_t maxlen)
{
	return snprintf(text, maxlen,
		"%5d",
		(int)p.offset_freq
	);
};

const struct ui_view ui_view_ssb = {
	UI_FIELDS_COMMON_N + 4,
	ui_view_ssb_text,
	{
	UI_FIELDS_COMMON,
	{ UI_FIELD_FT0,      24,25, 2, "SSB finetune kHz" },
	{ UI_FIELD_FT1,      26,26, 2, "Finetune 100 Hz"  },
	{ UI_FIELD_FT2,      27,27, 2, "Finetune 10 Hz"   },
	{ UI_FIELD_FT3,      28,28, 2, "SSB finetune Hz"  },
	}
};

static int ui_view_other_text(char *text, size_t maxlen)
{
	(void)text; (void)maxlen;
	// Nothing to write
	return 0;
}

const struct ui_view ui_view_other = {
	UI_FIELDS_COMMON_N,
	ui_view_other_text,
	{
	UI_FIELDS_COMMON
	}
};

struct ui_state ui = {
	.view = &ui_view_fm,
	.cursor = 6,
	.ctcss = 18, //check from ctcss table correct index number 18 = 118.8
};

#define BACKLIGHT_ON_TIME 2000
#define BACKLIGHT_DIM_LEVEL 50

#define DISPLAYBUF_SIZE 384
#define DISPLAYBUF2_SIZE 384
uint8_t displaybuf[DISPLAYBUF_SIZE], displaybuf2[DISPLAYBUF2_SIZE];

volatile struct display_ev display_ev;
SemaphoreHandle_t display_sem;

#if DISPLAYBUF_SIZE < 3*8*8
#error "Too small display buffer for text"
#endif

// Wrap number between 0 and b-1
static int wrap(int a, int b)
{
	while(a < 0) a += b;
	while(a >= b) a -= b;
	return a;
}

// Wrap number between -b+1 and b
static int wrap_signed(int a, int b)
{
	while (a <= -b) a += 2*b;
	while (a >   b) a -= 2*b;
	return a;
}

struct ui_text_color {
	// Foreground
	uint8_t fr, fg, fb;
	// Background
	uint8_t br, bg, bb;
};

const struct ui_text_color ui_text_colors[] = {
	{ 0xE0, 0xE0, 0xE0,  0x40, 0x40, 0x40 },
	{ 0x00, 0x00, 0x00,  0xFF, 0xFF, 0xFF },
	{ 0x80, 0xFF, 0x80,  0x60, 0x60, 0xC0 },
	{ 0x80, 0xFF, 0x80,  0x00, 0x00, 0x80 },
};

void ui_character(int x1, int y1, unsigned char c, unsigned char color)
{
	int x, y;
	if(!display_ready()) return;

	struct ui_text_color colors = ui_text_colors[color];
	if (c > 0x80)
		c = 0;
	const char *font = font8x8_basic[c];

	display_area(x1, y1, x1+7, y1+7);
	display_start();

	uint8_t *bufp = displaybuf;
	for (y=0; y<8; y++) {
		for (x=0; x<8; x++) {
			if (font[y] & (1<<x)) {
				*bufp++ = colors.fr;
				*bufp++ = colors.fg;
				*bufp++ = colors.fb;
			} else {
				*bufp++ = colors.br;
				*bufp++ = colors.bg;
				*bufp++ = colors.bb;
			}
		}
	}
	display_transfer(displaybuf, 3*8*8);
}
void ui_character_big(int x1, int y1, unsigned char c, unsigned char color)
{
 int x, y;
 if(!display_ready()) return;

 struct ui_text_color colors = ui_text_colors[color];
 if (c > 0x80)
  c = 0;
 const char *font = font8x8_basic[c];

 display_area(x1, y1, x1+15, y1+15);  // OH2BTG kokeille
 display_start();

 for (y=-1; y<8; y++) {  // OH2BTG kokeilee y=-1 siirtää kirjaimen alun näkyviin
  uint8_t *bufp = displaybuf;
  for (x=0; x<8; x++) {
   if (font[y] & (1<<x)) {
    *bufp++ = colors.fr;
    *bufp++ = colors.fg;
    *bufp++ = colors.fb;
    *bufp++ = colors.fr;
    *bufp++ = colors.fg;
    *bufp++ = colors.fb;
   } else {
    *bufp++ = colors.br;
    *bufp++ = colors.bg;
    *bufp++ = colors.bb;
    *bufp++ = colors.br;
    *bufp++ = colors.bg;
    *bufp++ = colors.bb;
   }
  }
  // Repeat each row of pixels twice
  for (x=0; x<2; x++) {
   display_transfer(displaybuf, 3*16);
  }
 }
}

void ui_update_text(void)
{
	int r;
	// TODO: put signal strength back somewhere
	//int s_dB = 10.0*log10(rs.smeter);

	unsigned cursor = ui.cursor;
	const struct ui_view *view = ui.view;

	char *textbegin = ui.text;
	char *text = textbegin;
	size_t maxlen = TEXT_LEN + 1;

	r = ui_view_common_text(text, maxlen);
	text += r;
	maxlen -= r;

	r = view->text(text, maxlen);
	text += r;
	maxlen -= r;

	// Fill the rest with spaces
	for (; text < textbegin + 48; text++, maxlen--)
		*text = ' ';

	r = snprintf(text, maxlen,
		"%s",
		view->fields[cursor].tip
	);
	text += r;
	maxlen -= r;

	for (; maxlen > 0; text++, maxlen--)
		*text = ' ';

	size_t n, i;
	for (i = 0; i < TEXT_LEN; i++) {
		ui.color[i] = 0;
	}
	for (n = 0; n < view->n; n++) {
		char c = 0;
		if (cursor == n) {
			// Highlight cursor
			c = 1;
		} else {
			c = view->fields[n].color;
		}
		size_t pos1 = view->fields[n].pos1;
		size_t pos2 = view->fields[n].pos2;
		for (i = pos1; i <= pos2; i++)
			ui.color[i] = c;
	}
}

static void ui_choose_view(void)
{
	switch (p.mode) {
	case MODE_FM:
		ui.view = &ui_view_fm;
		break;
	case MODE_USB:
	case MODE_LSB:
	case MODE_CWU:
	case MODE_CWL:
		ui.view = &ui_view_ssb;
		break;
	default:
		ui.view = &ui_view_other;
	}
}

static const int ui_steps[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9 };
static void ui_knob_turned(enum ui_field_name f, int diff)
{
	if (/*f >= UI_FIELD_FREQ0 && */f <= UI_FIELD_FREQ9) {
		p.frequency += diff * ui_steps[UI_FIELD_FREQ9 - f];
		xSemaphoreGive(railtask_sem);
	}
	else if (f == UI_FIELD_MODE) {
		p.mode = wrap(p.mode + diff, sizeof(p_mode_names) / sizeof(p_mode_names[0]));
		dsp_update_params();
		ui_choose_view();
	}
	/*else if (f == UI_FIELD_MEMO) {
		m.memo = wrap(m.memo + diff, sizeof(p_memo_names) / sizeof(p_memo_names[0]));
			printf("Hello Memo\n");
	//		return snprintf(text, maxlen,
	//		"%2d off ",
	//		p.squelch);
	//		dsp_update_params();
			ui_choose_view();
	}*/
	else if (f == UI_FIELD_PTT) {
		ui.keyed = wrap(ui.keyed + diff, 10);
	}
	else if (f == UI_FIELD_VOL) {
		p.volume = wrap(p.volume + diff, 20);
		dsp_update_params();
	}
	else if (f == UI_FIELD_WF) {
		p.waterfall_averages = wrap(p.waterfall_averages + diff, 100);
	}
	else if (f >= UI_FIELD_SPLIT0 && f <= UI_FIELD_SPLIT1) {
		int step = f == UI_FIELD_SPLIT0 ? 1000000 : 100000;
		p.split_freq = wrap_signed(
			p.split_freq +
			step * diff,
			100000000
		);
		xSemaphoreGive(railtask_sem);
	}
	else if (f == UI_FIELD_SQ) {
		p.squelch = wrap(p.squelch + diff, 100);
		dsp_update_params();
	}
	else if (f == UI_FIELD_CTCSS) {
		ui.ctcss = wrap(ui.ctcss + diff, sizeof(ctcss_freqs) / sizeof(ctcss_freqs[0]));
		p.ctcss = 0.1f * (float)ctcss_freqs[ui.ctcss];
		dsp_update_params();
	}
	else if (f >= UI_FIELD_FT0 && f <= UI_FIELD_FT3) {
		p.offset_freq = wrap_signed(
			p.offset_freq +
			ui_steps[UI_FIELD_FT3 - f] * diff,
			6000
		);
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
	int pos_now, pos_diff;
	char button = get_encoder_button(), ptt = get_ptt();
	pos_now = get_encoder_position() / ENCODER_DIVIDER;
	pos_diff = pos_now - ui.pos_prev;

	if (p.mode == MODE_OFF && ui.button_prev && (!button)) {
		// Shut down after button has been released.
		shutdown();
	}
		if (p.mode == MODE_TST && ui.button_prev && (!button)&&(rs.smeter > 37)) {
				p.volume = 15;
			    if (rs.smeter < 36)
				{
					p.volume = 5;
				}
					
				
			  	/*  m.memo = MEMO_M1; // Initial memo
    			printf("Initial Memo: %s\n", m_memo_names[m.memo]);

    			// Simulate changing memo by +1
    			update_memo(2);

    			// Simulate changing memo by -1
    			update_memo(-2);

    			// Simulate changing memo by +5
    			update_memo(0);
				*/
    		//	return 0;
			//p.frequency = 14000000;
			//xSemaphoreGive(railtask_sem);
			//p.mode = MODE_USB;
			//p.split_freq = -2000000;
			//p.ctcss = 118;
			//dsp_update_params();
	}
	/*if (p.memo == MEMO && ui.button_prev && (!button)) {
		// Testing OH2BTG miten taajuus asetetaan
			//printf("Hello Word1\n");
			//int32_t S_dB = (10.0*log10(rs.smeter) - 147);  // Smeter toimii tässä OH2BTG
			//printf("S-meter: %ld\n", S_dB);
			p.frequency = 14000000;
			xSemaphoreGive(railtask_sem);
			p.mode = MODE_USB;
			//p.split_freq = -2000000;
			//p.ctcss = 118;
			dsp_update_params();
	}*/


	const struct ui_view *view = ui.view;

	if (button)
		ui.backlight_timer = 0;
	if (pos_diff) {
		if(pos_diff >= 0x8000 / ENCODER_DIVIDER)
			pos_diff -= 0x10000 / ENCODER_DIVIDER;
		else if(pos_diff < -0x8000 / ENCODER_DIVIDER)
			pos_diff += 0x10000 / ENCODER_DIVIDER;

		if (button) {
			ui.cursor = wrap(ui.cursor + pos_diff, view->n);
		} else {
			size_t c = ui.cursor;
			if (c < view->n) {
				ui_knob_turned(view->fields[c].name, pos_diff);
			}
		}
		ui.backlight_timer = 0;
	}
	if (pos_diff != 0 || ptt != ui.ptt_prev) {
		if (tx_freq_allowed(p.frequency + p.split_freq)) {
			p.keyed = ui.keyed || ptt;
		} else {
			p.keyed = 0;
			ui.keyed = 0;
		}
		if (p.keyed != ui.keyed_prev)
			xSemaphoreGive(railtask_sem);
		ui.keyed_prev = p.keyed;


		/* Something on the display may have changed at this point,
		 * so make the display task check for that. */
		display_ev.text_changed = 1;
		xSemaphoreGive(display_sem);
	}

	ui.pos_prev = pos_now;
	ui.ptt_prev = ptt;
	ui.button_prev = button;
}


/* ui_control_backlight is regularly called from the misc task. */
void ui_control_backlight(void)
{
	if (ui.backlight_timer <= BACKLIGHT_ON_TIME) {
		display_backlight(BACKLIGHT_DIM_LEVEL + BACKLIGHT_ON_TIME - ui.backlight_timer);
		ui.backlight_timer++;
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
	//	printf("Bug? Display not ready in waterfall\n");
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
void ui_display_offset_cursor(void) //
{
	display_area(0,33, 127, 36);  // OH2BTG try to change offset cursor place
	display_start();
	// First 33*8 bytes of font data is zeros
	int i;
	for (i = 0; i < 128 * 3 * 3 / (8*16); i++)
		display_transfer((const uint8_t*)font8x8_basic[0], 8*16);

	// Calculate the position based on sample rate and FFT size
	int x = 64 + p.offset_freq * 256 / (RX_IQ_FS/2);
	if (x < 1) x = 1;
	if (x > 127) x = 127;
	display_area(x-1, 33, x+1, 36); // OH2BTG try to change offset cursor place
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
		char c = ui.text[i],  cp = ui.textprev[i];
		unsigned char v = ui.color[i], vp = ui.colorprev[i];
		if (c != cp || v != vp) {
			//if (i < 8) // first line
			//	ui_character_big(i*16, 0, c, v);   // OH2BTG big text to display 
			if (i < 8) // first line
				ui_character_big(i*16, 0, c, v);
			else if (i<10) // 2 last digits of freq small fonts to end of 2nd line
				ui_character(((i-8)*8)+128-16, 16, c, v); 	
			else if (i <14) // second line
				ui_character_big(((i-10)*16),16, c, v);  // i define where text start
			else if(i < 17) // test end of 2nd line small fonts for dBm
				ui_character(((i-14)*8)+64, 24, c, v);  
			else if(i < 33) // third line FM RX  vol sql
				ui_character((i-17)*8, 32, c, v);  // 
			else if(i < 43) // forth line FM and mode specific 
				ui_character((i-33)*8, 40, c, v);  // 
			else // bottom line OH2BTG
				ui_character(((i-43)*8)-16, 152, c, v); 
			/*if (i < 16) // first line
				ui_character(i*8, 0, c, v);
			else if (i < 32) // second line
				ui_character((i-16)*8, 8, c, v);
			else if(i < 48) // third line
				ui_character((i-32)*8, 16, c, v);
			else // bottom line OH2BTG
				ui_character((i-48)*8, 32, c, v);*/
			ui.textprev[i] = c;
			ui.colorprev[i] = v;

			// ui_display_waterfall();  // E10 OH2BTG
		}
	}
	// ui_display_offset_cursor();  // OH2BTG remove display offset cursor
}


void display_task(void *arg)
{
	(void)arg;
	display_init();
	for (;;) {
		xSemaphoreTake(display_sem, portMAX_DELAY);
		 ui_display_waterfall(); // E10 OH2BTG
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
