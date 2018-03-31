/*
 * dsp.h
 *
 *  Created on: Mar 31, 2018
 *      Author: oh2eat
 */

#ifndef INC_DSP_PARAMETERS_H_
#define INC_DSP_PARAMETERS_H_

#define TXBLOCKLEN 1000
#define IQBLOCKLEN 8
#define PWMBLOCKLEN (4*IQBLOCKLEN)

typedef int16_t iqsample_t[2];

void dsp_rx(iqsample_t *input, uint8_t *output);
void dsp_tx(uint8_t *input, uint8_t *output);

void start_rx_dsp();
void start_tx_dsp();

#endif /* INC_DSP_PARAMETERS_H_ */
