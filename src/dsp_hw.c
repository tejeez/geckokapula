/*
 * dsp_hw.c
 *
 *  Created on: Mar 31, 2018
 *      Author: oh2eat
 */

// emlib
#include "rail.h"
#include "em_timer.h"
#include "em_adc.h"
#include "em_ldma.h"

#include "InitDevice.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// rig
#include "rig.h"
#include "hw.h"
#include "dsp_parameters.h"

static iqsample_t iqbuffer[IQBLOCKLEN];
static uint8_t pwmbuffer1[PWMBLOCKLEN], pwmbuffer2[PWMBLOCKLEN];
static uint8_t adcbuffer1[TXBLOCKLEN], adcbuffer2[TXBLOCKLEN];
static uint8_t synthbuffer1[TXBLOCKLEN], synthbuffer2[TXBLOCKLEN];

#define PING 0
#define PONG 1
static volatile char dma_adc_phase, dma_pwm_phase;

void start_rx_dsp() {
	/* RX chain reads I/Q samples from RAIL FIFO
	 * and writes samples to PWM.
	 * PWM is fed by ping-pong DMA and takes a new sample
	 * on each PWM cycle.
	 * When transfer of a half buffer to PWM is ready,
	 * the DSP code shall first read samples from RAIL FIFO,
	 * process them and write the result in the PWM buffer.
	 * DMA is kept running during TX so it's possible to
	 * play a sidetone when needed.
	 */
	static const LDMA_TransferCfg_t pwmtrigger =
 	LDMA_TRANSFER_CFG_PERIPHERAL(
 			//ldmaPeripheralSignal_TIMER0_UFOF
 			/* Trigger from ADC0 only for initial test until
 			 * I find out how to trigger on each PWM cycle.
 			 */
 			ldmaPeripheralSignal_ADC0_SINGLE
			);
	static const LDMA_Descriptor_t pwmLoop[] = {
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(pwmbuffer1, &TIMER0->CC[0], 500, 1),
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(pwmbuffer2, &TIMER0->CC[0], 500, -1)
	/*LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(pwmbuffer1, &TIMER0->CC[0], PWMBLOCKLEN, 1),
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(pwmbuffer2, &TIMER0->CC[0], PWMBLOCKLEN, -1)*/
	};

	//LDMA_StopTransfer(DMA_CH_PWM);
	dma_pwm_phase = PING;
 	LDMA_StartTransfer(DMA_CH_PWM, &pwmtrigger, pwmLoop);
}

void start_tx_dsp() {
	/* TX chain reads samples from microphone ADC
	 * and writes samples to frequency synthesizer.
	 * They are using ping-pong DMA buffers working
	 * at the same rate and started nearly at the same time.
	 * When ADC has written the first half input buffer,
	 * the DSP code shall start processing it and
	 * writing result in first half output buffer.
	 * The same happens for the second half and repeats.
	 * */
	/*NVIC_SetPriority(ADC0_IRQn, 3); // TODO: irq priorities in header or something
	NVIC_EnableIRQ(ADC0_IRQn);
	ADC_IntEnable(ADC0, ADC_IF_SINGLE);*/

 	static const LDMA_TransferCfg_t adctrigger =
 	LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_ADC0_SINGLE);

	static const LDMA_Descriptor_t adcLoop[] = {
	LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&ADC0->SINGLEDATA, adcbuffer1, TXBLOCKLEN, 1),
	LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&ADC0->SINGLEDATA, adcbuffer2, TXBLOCKLEN, -1)
	};

	static const LDMA_Descriptor_t synthLoop[] = {
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(synthbuffer1, &SYNTH_CHANNEL, TXBLOCKLEN, 1),
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(synthbuffer2, &SYNTH_CHANNEL, TXBLOCKLEN, -1)
	};

	// stop ADC first to ensure the DMAs start at the same sample
	ADC_Reset(ADC0);

	//LDMA_StopTransfer(DMA_CH_ADC);
	//LDMA_StopTransfer(DMA_CH_SYNTH);

	dma_adc_phase = PING;
	LDMA_StartTransfer(DMA_CH_ADC,   &adctrigger, adcLoop);
 	//LDMA_StartTransfer(DMA_CH_SYNTH, &adctrigger, synthLoop);
 	//LDMA_IntEnable(DMA_CH_ADC);
 	LDMA_IntDisable(DMA_CH_SYNTH); // don't need interrupts from both

 	ADC0_enter_DefaultMode_from_RESET();
	ADC_Start(ADC0, adcStartSingle);
}

void LDMA_IRQHandler() {
	extern int testnumber;
	uint32_t pending = LDMA_IntGetEnabled();
	if(pending & (1<<DMA_CH_SYNTH)) { // not used
		LDMA->IFC = 1<<DMA_CH_SYNTH;
	}
	if(pending & (1<<DMA_CH_ADC)) {
		//testnumber+=10;
		/* Is there a way to check which half of the ping-pong
		 * transfer just completed? Now we just count it from start
		 * and hope it stays in sync...
		 */
		//testnumber++;
#if 0
		if(dma_adc_phase == PING) {
			dsp_tx(adcbuffer1, synthbuffer1);
			dma_adc_phase = PONG;
		} else {
			dsp_tx(adcbuffer2, synthbuffer2);
			dma_adc_phase = PING;
		}
#endif
		LDMA->IFC = 1<<DMA_CH_ADC;
	}
	if(pending & (1<<DMA_CH_PWM)) {
		testnumber++;
		//RAIL_ReadRxFifo((uint8_t*)iqbuffer, IQBLOCKLEN*sizeof(iqsample_t));
#if 0
		if(dma_pwm_phase == PING) {
			dsp_rx(iqbuffer, pwmbuffer1);
			dma_pwm_phase = PONG;
		} else {
			dsp_rx(iqbuffer, pwmbuffer2);
			dma_pwm_phase = PING;
		}
#endif
		LDMA->IFC = 1<<DMA_CH_PWM;
	}
	if(pending & (1<<DMA_CH_DISPLAY)) {
		//testnumber+=100;
		// TODO: wake up display task if becomes necessary
		LDMA->IFC = 1<<DMA_CH_DISPLAY;
	}
}

void RAILCb_RxFifoAlmostFull(uint16_t bytesAvailable) {
	// not used now because reads are timed by the audio output DMA interrupt
}

