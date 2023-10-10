/* SPDX-License-Identifier: MIT */

#include "dsp.h"
#include "rig.h"
#include <stdio.h>

/* How many transmit samples to process at a time */
#define TX_DSP_BLOCK 32

rig_parameters_t p = {
	.channel_changed = 1,
	.keyed = 0,
	.mode = MODE_FM,
	.frequency = RIG_DEFAULT_FREQUENCY,
	.offset_freq = 0UL,
	.volume = 10,
	.waterfall_averages = 20,
	.squelch = 15
};
rig_status_t rs = {0};

int main(int argc, char *argv[])
{
	if (argc < 2)
		return 1;
	FILE *fm_out_file = fopen(argv[1], "wb");
	if (fm_out_file == NULL)
		return 2;
	for (;;) {
		audio_in_t audio_in[TX_DSP_BLOCK];
		fm_out_t fm_out[TX_DSP_BLOCK];
		if (fread(audio_in, sizeof(audio_in), 1, stdin) != 1)
			break;
		dsp_fast_tx(audio_in, fm_out, TX_DSP_BLOCK);
		(void)fwrite(fm_out, sizeof(fm_out), 1, fm_out_file);
	}
	fclose(fm_out_file);
	return 0;
}
