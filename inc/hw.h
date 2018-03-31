/*
 * hw.h
 *
 *  Created on: Mar 31, 2018
 *      Author: oh2eat
 */

#ifndef INC_HW_H_
#define INC_HW_H_
// some random hardware specific parameters

/* IQ samples arrive every 672 clock cycles.
 * Period 672/3=224 makes 3 PWM samples per IQ sample.
 */
#define TIMER0_PERIOD 224

#define DMA_CH_SYNTH 0
#define DMA_CH_PWM 1
#define DMA_CH_ADC 2
#define DMA_CH_DISPLAY 3

#define SYNTH_CHANNEL (*(volatile uint8_t*)0x40083038)

#endif /* INC_HW_H_ */
