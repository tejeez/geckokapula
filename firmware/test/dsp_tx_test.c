/* SPDX-License-Identifier: MIT */

#include "dsp.h"
#include "rig.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* How many transmit samples to process at a time */
#define TX_DSP_BLOCK 32

/* Oversampling for I/Q output file */
#define IQ_OVERSAMP 4

rig_parameters_t p = {
	.keyed = 0,
	.mode = MODE_FM,
	.frequency = RIG_DEFAULT_FREQUENCY,
	.split_freq = 0,
	.offset_freq = 0,
	.volume = 10,
	.waterfall_averages = 20,
	.squelch = 15,
	.ctcss = 123.0f,
};
rig_status_t rs = {0};

static void fm_to_iq(float *phase, fm_out_t *in, iq_float_t *out, size_t length)
{
	// Frequency step in Hz
	const double step = 38.4e6 / (1L<<18);
	// Sample rate in Hz
	const double fs = 24000.0 * (double)IQ_OVERSAMP;
	// FM deviation coefficient in radians per sample per step
	const float dev = (float)(M_PI*2.0 * step / fs);

	float ph = *phase;

	size_t i;
	for (i = 0; i < length * IQ_OVERSAMP; i++)
	{
		float f = dev * (((float)in[i / IQ_OVERSAMP]) - 32.0f);
		ph = fmodf(ph + f, (float)(M_PI*2.0));
		out[i].i = cosf(ph);
		out[i].q = sinf(ph);
	}

	*phase = ph;
}

// Check that FM modulation samples are in allowed range
static void check_fm_limits(fm_out_t *fm, size_t length)
{
	size_t i;
	for (i = 0; i < length; i++) {
		assert(fm[i] <= 63);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 4)
		return 1;
	if (strcmp(argv[1], "USB") == 0) {
		p.mode = MODE_USB;
	}
	FILE *fm_out_file = fopen(argv[2], "wb");
	if (fm_out_file == NULL)
		return 2;
	FILE *iq_out_file = fopen(argv[3], "wb");
	if (iq_out_file == NULL)
		return 3;

	dsp_update_params();

	float phase = 0.0f;
	for (;;) {
		audio_in_t audio_in[TX_DSP_BLOCK];
		fm_out_t fm_out[TX_DSP_BLOCK];
		iq_float_t iq_out[TX_DSP_BLOCK * IQ_OVERSAMP];
		if (fread(audio_in, sizeof(audio_in), 1, stdin) != 1)
			break;
		dsp_fast_tx(audio_in, fm_out, TX_DSP_BLOCK);
		check_fm_limits(fm_out, TX_DSP_BLOCK);
		fm_to_iq(&phase, fm_out, iq_out, TX_DSP_BLOCK);
		(void)fwrite(fm_out, sizeof(fm_out), 1, fm_out_file);
		(void)fwrite(iq_out, sizeof(iq_out), 1, iq_out_file);
	}
	fclose(fm_out_file);
	return 0;
}
