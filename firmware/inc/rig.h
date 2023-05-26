/* SPDX-License-Identifier: MIT */

#ifndef INC_RIG_H_
#define INC_RIG_H_

#include <stdint.h>

enum rig_mode { MODE_NONE, MODE_FM, MODE_AM, MODE_DSB };

// parameters communicated from UI to RAIL and DSP parts
typedef struct {
	//int channel;
	char channel_changed, keyed;
	enum rig_mode mode;
	uint32_t frequency;
	int32_t offset_freq;
	uint8_t volume;
	//int volume2;
	uint8_t waterfall_averages;
	unsigned squelch;
} rig_parameters_t;
extern rig_parameters_t p;

// status communicated from DSP to UI
typedef struct {
	uint32_t smeter;
} rig_status_t;
extern rig_status_t rs;
#define FFTLEN 256

#if KAPULA_v2
#define RIG_DEFAULT_FREQUENCY 433550000UL
#else
#define RIG_DEFAULT_FREQUENCY 2397500000UL
#endif

#endif /* INC_RIG_H_ */
