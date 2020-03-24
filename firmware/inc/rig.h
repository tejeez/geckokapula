/*
 * rig.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Tatu
 */

#ifndef INC_RIG_H_
#define INC_RIG_H_

enum rig_mode { MODE_FM, MODE_AM, MODE_DSB, MODE_NONE };

// parameters communicated from UI to RAIL and DSP parts
typedef struct {
	int channel;
	char channel_changed, keyed;
	enum rig_mode mode;
	uint32_t frequency;
	uint8_t volume;
	int volume2;
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

#endif /* INC_RIG_H_ */
