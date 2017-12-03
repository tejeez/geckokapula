/*
 * rig.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Tatu
 */

#ifndef INC_RIG_H_
#define INC_RIG_H_

// parameters
typedef struct {
	int channel;
	int channel_changed;
	enum { MODE_FM, MODE_DSB } mode;
	uint32_t frequency;
	uint32_t step;
} rig_parameters_t;
extern rig_parameters_t p;

#endif /* INC_RIG_H_ */
