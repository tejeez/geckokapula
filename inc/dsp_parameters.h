/*
 * dsp.h
 *
 *  Created on: Mar 31, 2018
 *      Author: oh2eat
 */

#ifndef INC_DSP_PARAMETERS_H_
#define INC_DSP_PARAMETERS_H_
#include "hw.h"

// see hw.h
#define PWM_IQ_FS_RATIO 3

#define TXBLOCKLEN 32
#define IQBLOCKLEN 64
#define PWMBLOCKLEN (PWM_IQ_FS_RATIO*IQBLOCKLEN)
#define PWMMAX TIMER0_PERIOD

typedef int16_t iqsample_t[2];

void dsp_rx(iqsample_t *input, uint8_t *output);
void dsp_tx(uint8_t *input, uint8_t *output);

void start_rx_dsp();
void start_tx_dsp();

#endif /* INC_DSP_PARAMETERS_H_ */
