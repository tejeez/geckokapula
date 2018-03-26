/*
 * rig.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Tatu
 */

#ifndef INC_RIG_H_
#define INC_RIG_H_

// parameters communicated from UI to RAIL and DSP parts
typedef struct {
	int channel;
	char channel_changed, keyed;
	enum { MODE_FM, MODE_DSB } mode;
	uint32_t frequency;
	uint8_t volume;
} rig_parameters_t;
extern rig_parameters_t p;

// status communicated from DSP to UI
typedef struct {
	uint32_t smeter;
} rig_status_t;
extern rig_status_t rs;
#define FFTLEN 256

#endif /* INC_RIG_H_ */
