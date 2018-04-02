/*
 * dsp.h
 *
 *  Created on: Mar 31, 2018
 *      Author: oh2eat
 */

#ifndef INC_DSP_PARAMETERS_H_
#define INC_DSP_PARAMETERS_H_

// see hw.h:
#define PWM_IQ_FS_RATIO 3
#define PWMMAX 224

// block sizes for DSP interrupts:
#define TXBLOCKLEN 64
#define IQBLOCKLEN 32
#define PWMBLOCKLEN (PWM_IQ_FS_RATIO*IQBLOCKLEN)

// frequency synthesizer:
#define CHANNELSPACING 147 // 38.4 MHz / 2^18
#define MIDDLECHANNEL 32

typedef int16_t iqsample_t[2];

void dsp_rx(iqsample_t *input, uint8_t *output);
void dsp_tx(uint16_t *input, uint8_t *output);

void start_rx_dsp();
void start_tx_dsp();
void dsp_init();

#endif /* INC_DSP_PARAMETERS_H_ */
