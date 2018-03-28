/*
 * dsp.c
 *
 *  Created on: Mar 26, 2018
 *      Author: Tatu
 */

#include <stdlib.h>

// CMSIS
#include "arm_math.h"
#include "arm_const_structs.h"

// emlib
#include "rail.h"
#include "em_timer.h"
#include "em_adc.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// rig
#include "rig.h"
#include "ui.h"


#define RXBUFL 2
typedef int16_t iqsample_t[2];
iqsample_t rxbuf[RXBUFL];

const arm_cfft_instance_f32 *fftS = &arm_cfft_sR_f32_len256;
float fftbuf[2*FFTLEN];
volatile int fftbufp = 0;


extern rig_parameters_t p;

/* Interrupt for DSP operations that take a short time and need low latency */
void RAILCb_RxFifoAlmostFull(uint16_t bytesAvailable) {
	unsigned nread, i;
	static int psi=0, psq=0;
	static int agc_level=0;
	nread = RAIL_ReadRxFifo((uint8_t*)rxbuf, 4*RXBUFL);
	nread /= 4;
	int ssi=0, ssq=0, audioout = 0;
	static unsigned smeter_count = 0;
	static uint64_t smeter_acc = 0;
	static int audio_lpf = 0, audio_hpf = 0;
	int vola = p.volume;
 	for(i=0; i<nread; i++) {
		int si=rxbuf[i][0], sq=rxbuf[i][1];
		int fi, fq;
		switch(p.mode) {
		case MODE_FM: {
			int fm;
			// multiply by conjugate
			fi = si * psi + sq * psq;
			fq = sq * psi - si * psq;
			/* Scale maximum absolute value to 0x7FFF.
			 * This can be done because FM demod doesn't care about amplitude.
			 */
			while(fi > 0x7FFF || fi < -0x7FFF || fq > 0x7FFF || fq < -0x7FFF) {
				fi /= 0x100; fq /= 0x100;
			}
			// very crude approximation...
			fm = 0x8000 * fq / ((fi>=0?fi:-fi) + (fq>=0?fq:-fq));
			audio_lpf += (fm*128 - audio_lpf)/16;
			audioout = audio_lpf/128;
			break; }
		case MODE_AM:
		case MODE_DSB: {
			int agc_1, agc_diff;
			int raw_audio;
			if(p.mode == MODE_AM)
				raw_audio = abs(si) + abs(sq);
			else
				raw_audio = si*2;
			audio_lpf += (raw_audio*64 - audio_lpf)/16;
			audio_hpf += (audio_lpf - audio_hpf) / 512; // DC block
			fi = (audio_lpf-audio_hpf)/128; // TODO: SSB filter

			// AGC
			agc_1 = abs(fi) * 0x100; // rectify
			agc_diff = agc_1 - agc_level;
			if(agc_diff > 0)
				agc_level += agc_diff/256;
			else
				agc_level += agc_diff/2048;

			audioout += 0x1000 * fi / (agc_level/0x100);

			break; }
		default: {
			audioout = 0;
			break;
		}
		}

		psi = si; psq = sq;
		ssi += si; ssq += sq;

		smeter_acc += si*si + sq*sq;
	}

	int fp = fftbufp;
	if(fp < 2*FFTLEN) {
		const float scaling = 1.0f / (RXBUFL*0x8000);
		fftbuf[fp]   = scaling*ssi;
		fftbuf[fp+1] = scaling*ssq;
		fftbufp = fp+2;
	}

	audioout = (vola * audioout / 0x400) + 100;
	if(audioout < 0) audioout = 0;
	if(audioout > 200) audioout = 200;
	TIMER_CompareBufSet(TIMER0, 0, audioout);
	//USART_Tx(USART0, 'r');

	smeter_count += nread;
	if(smeter_count >= 0x4000) {
		rs.smeter = smeter_acc / 0x4000;
		smeter_acc = 0;
		smeter_count = 0;
	}

}

inline void synth_set_channel(int ch) {
	*(uint32_t*)(uint8_t*)(0x40083000 + 56) = ch;
}

extern int testnumber;
void ADC0_IRQHandler() {
	int audioout;
	static int hpf, lpf, agc_level=0;
	testnumber++;

	int audioin = ADC0->SINGLEDATA;
	ADC_IntClear(ADC0, ADC_IF_SINGLE);

	if(!p.keyed) {
		synth_set_channel(p.channel);
		return;
	}

	// DC block:
	hpf += (audioin - hpf) / 32;
	audioin -= hpf;
	// lowpass
	lpf += (audioin - lpf) / 2;
	audioin = lpf;

	// Just copied AGC code from RX for now
	int agc_1, agc_diff;
	agc_1 = abs(audioin) * 0x100; // rectify
	agc_diff = agc_1 - agc_level;
	if(agc_diff > 0)
		agc_level += agc_diff/0x100;
	else
		agc_level += agc_diff/0x1000;

	audioout = 32 + 20 * audioin / (agc_level/0x100);
	if(audioout <= 0) audioout = 0;
	if(audioout >= 63) audioout = 63;

	synth_set_channel(audioout);
}


/* A task for DSP operations that can take a longer time */
void dsp_task() {
	NVIC_SetPriority(ADC0_IRQn, 3); // TODO: irq priorities in header or something
	NVIC_EnableIRQ(ADC0_IRQn);
	ADC_IntEnable(ADC0, ADC_IF_SINGLE);
 	ADC_Start(ADC0, adcStartSingle);

	for(;;) {
		// TODO: semaphore?
		if(fftbufp >= 2*FFTLEN) {
			arm_cfft_f32(fftS, fftbuf, 0, 1);
			ui_fft_line(fftbuf);
			fftbufp = 0;
		}
		vTaskDelay(2);
	}
}

