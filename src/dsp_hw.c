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
#include "em_bus.h"

#include "InitDevice.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// rig
#include "rig.h"
#include "hw.h"
#include "dsp_parameters.h"

#if 0
#define interrupt_debugc debugc
#else
#define interrupt_debugc(...)
#endif

static iqsample_t iqbuffer[IQBLOCKLEN];
static uint8_t pwmbuffer1[2*PWMBLOCKLEN], *const pwmbuffer2 = pwmbuffer1 + PWMBLOCKLEN;
static uint16_t adcbuffer1[2*TXBLOCKLEN], *const adcbuffer2 = adcbuffer1 + TXBLOCKLEN;
static uint8_t synthbuffer1[2*TXBLOCKLEN], *const synthbuffer2 = synthbuffer1 + TXBLOCKLEN;

#define PING 0
#define PONG 1
static volatile char dma_adc_phase, dma_pwm_phase;

#define LDMA_DESCRIPTOR_LINKREL_P2M_HALF(src, dest, count, linkjmp) \
  {                                                                 \
    .xfer =                                                         \
    {                                                               \
      .structType   = ldmaCtrlStructTypeXfer,                       \
      .structReq    = 0,                                            \
      .xferCnt      = (count) - 1,                                  \
      .byteSwap     = 0,                                            \
      .blockSize    = ldmaCtrlBlockSizeUnit1,                       \
      .doneIfs      = 1,                                            \
      .reqMode      = ldmaCtrlReqModeBlock,                         \
      .decLoopCnt   = 0,                                            \
      .ignoreSrec   = 0,                                            \
      .srcInc       = ldmaCtrlSrcIncNone,                           \
      .size         = ldmaCtrlSizeHalf,                             \
      .dstInc       = ldmaCtrlDstIncTwo,                            \
      .srcAddrMode  = ldmaCtrlSrcAddrModeAbs,                       \
      .dstAddrMode  = ldmaCtrlDstAddrModeAbs,                       \
      .srcAddr      = (uint32_t)(src),                              \
      .dstAddr      = (uint32_t)(dest),                             \
      .linkMode     = ldmaLinkModeRel,                              \
      .link         = 1,                                            \
      .linkAddr     = (linkjmp) * 4                                 \
    }                                                               \
  }

void dsp_init() {
	extern char rail_initialized;
	while(!rail_initialized) vTaskDelay(20);
	start_tx_dsp();
	start_rx_dsp();
}

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
#if 1
	static const LDMA_TransferCfg_t pwmtrigger =
	LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_TIMER0_CC2);
	static const LDMA_Descriptor_t pwmLoop[] = {
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(pwmbuffer1, &TIMER0->CC[0].CCVB, PWMBLOCKLEN, 1),
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(pwmbuffer2, &TIMER0->CC[0].CCVB, PWMBLOCKLEN, -1)
	};

	//LDMA_StopTransfer(DMA_CH_PWM);
	dma_pwm_phase = PING;
 	LDMA_StartTransfer(DMA_CH_PWM, &pwmtrigger, pwmLoop);
#endif
#if 0
	NVIC_SetPriority(TIMER0_IRQn, 3); // TODO: irq priorities in header or something
	NVIC_EnableIRQ(TIMER0_IRQn);
	TIMER_IntEnable(TIMER0, TIMER_IF_OF);
#endif
}

#if 0
static volatile unsigned dspbufindex = 0;
void TIMER0_Handler() {
	int b = dspbufindex;
	TIMER0->CC[0] = pwmbuffer1[b];
	b++;
	if(b >= PWMBLOCKLEN) b = 0;
	dspbufindex = b;
}
#endif

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
	LDMA_DESCRIPTOR_LINKREL_SYNC(1, 0, 0, 0, 1),
	LDMA_DESCRIPTOR_LINKREL_P2M_HALF(&ADC0->SINGLEDATA, adcbuffer1, TXBLOCKLEN, 1),
	LDMA_DESCRIPTOR_LINKREL_P2M_HALF(&ADC0->SINGLEDATA, adcbuffer2, TXBLOCKLEN, -2)
	};

	static const LDMA_Descriptor_t synthLoop[] = {
	LDMA_DESCRIPTOR_LINKREL_SYNC(0, 1, 1, 1, 1),
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(synthbuffer1, &SYNTH_CHANNEL, TXBLOCKLEN, 1),
	LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(synthbuffer2, &SYNTH_CHANNEL, TXBLOCKLEN, -2)
	};

	// stop ADC first to ensure the DMAs start at the same sample
	ADC_Reset(ADC0);

	/*LDMA_StopTransfer(DMA_CH_ADC);
	LDMA_StopTransfer(DMA_CH_SYNTH);*/

	//vTaskDelay(1); // Ensure ADC has time to stop. Maybe not necessary.

	dma_adc_phase = PING;
	LDMA_StartTransfer(DMA_CH_ADC,   &adctrigger, adcLoop);
	LDMA_StartTransfer(DMA_CH_SYNTH, &adctrigger, synthLoop);
	LDMA_IntDisable(1<<DMA_CH_SYNTH); // don't need interrupts from both

	ADC0_enter_DefaultMode_from_RESET();
	ADC_Start(ADC0, adcStartSingle);
}

char can_read_rail_fifo = 0;
volatile int rail_bytes=0;
void LDMA_IRQHandler() {
	extern int testnumber;
	uint32_t pending = LDMA_IntGetEnabled();
	static const uint32_t masks[] = {
			1<<DMA_CH_DISPLAY, 1<<DMA_CH_SYNTH, 1<<DMA_CH_PWM, 1<<DMA_CH_ADC
	};
	int i;
	//debugc('.');
	for(i=0; i<4; i++) {
		uint32_t m = masks[i] /*1<<i*/;
		if(pending & m) LDMA->IFC = m;
	}
	/* Is there a way to check which half of the ping-pong
	 * transfer just completed? Now we just count it from start
	 * and hope it stays in sync...
	 * (apparently it doesn't)
	 */
	if(pending & (1<<DMA_CH_PWM)) {
		//BUS_RegMaskedClear(LDMA->CHDONE, 1<<DMA_CH_PWM);
		uint8_t *srcaddr = (uint8_t*)LDMA->CH[DMA_CH_PWM].SRC;
		//testnumber++;
#if 1
		int ba = RAIL_GetRxFifoBytesAvailable();
		if(!can_read_rail_fifo) {
			// start reading when enough is available
			if(ba >
			   (IQBLOCKLEN+10)*sizeof(iqsample_t))
				can_read_rail_fifo = 1;
		}
		rail_bytes = ba;
		if(can_read_rail_fifo)
			RAIL_ReadRxFifo((uint8_t*)iqbuffer, IQBLOCKLEN*sizeof(iqsample_t));
		if(/*dma_pwm_phase == PING*/srcaddr >= pwmbuffer2 && srcaddr < pwmbuffer2+PWMBLOCKLEN) {
			interrupt_debugc('p');
			dsp_rx(iqbuffer, pwmbuffer1);
			//dma_pwm_phase = PONG;
		} else {
			interrupt_debugc('P');
			dsp_rx(iqbuffer, pwmbuffer2);
			//dma_pwm_phase = PING;
		}
#endif
	}
	if(pending & (1<<DMA_CH_ADC)) {
		//BUS_RegMaskedClear(LDMA->CHDONE, 1<<DMA_CH_ADC);
		uint16_t *dstaddr = (uint16_t*)LDMA->CH[DMA_CH_ADC].DST;
		uint8_t *srcaddr = (uint8_t*)LDMA->CH[DMA_CH_SYNTH].SRC;
		//testnumber+=10;
#if 1
		//if(/*dma_adc_phase == PING*/ dstaddr >= adcbuffer2 && dstaddr < adcbuffer2+TXBLOCKLEN) {
		if(srcaddr >= synthbuffer2 && srcaddr < synthbuffer2+TXBLOCKLEN) {
			interrupt_debugc('a');
			dsp_tx(adcbuffer1, synthbuffer1);
			//dma_adc_phase = PONG;
		} else {
			interrupt_debugc('A');
			dsp_tx(adcbuffer2, synthbuffer2);
			//dma_adc_phase = PING;
		}
#endif
	}
}

void RAILCb_RxFifoAlmostFull(uint16_t bytesAvailable) {
	/* This is not used now because reads are timed by
	 * the audio output DMA interrupt.
	 * It, however, seems necessary to read some samples here
	 * because otherwise the program gets stuck for some reason.
	 * This should get called rarely in normal operation!
	 */
	interrupt_debugc('?');
	uint8_t dummybuffer[8];
	int i;
	for(i=0; i<128/8; i++)
		RAIL_ReadRxFifo(dummybuffer, 8);
}

/* For DMA debugging. Should make a triangle wave */
void dsp_rx_testsignals() {
	int i;
	for(i=0; i<PWMBLOCKLEN; i++) {
		int a = PWMMAX * i / PWMBLOCKLEN;
		pwmbuffer1[i] = a;
		pwmbuffer2[i] = PWMMAX - a;
	}
}

#include <stdio.h>
void debugputs(char*);
void dsp_buf_debug() {
	char o[30];
	uint16_t *dstaddr;
	uint8_t *srcaddr;
	portDISABLE_INTERRUPTS();
	dstaddr = (uint16_t*)LDMA->CH[DMA_CH_ADC].DST;
	srcaddr = (uint8_t*)LDMA->CH[DMA_CH_SYNTH].SRC;
	portENABLE_INTERRUPTS();
	int adcpos = dstaddr - adcbuffer1, synthpos = srcaddr - synthbuffer1;
	snprintf(o, 30, "%4d %4d %4d  %5d\n",
			adcpos, synthpos, (synthpos-adcpos) & (TXBLOCKLEN-1),
			rail_bytes);
	debugputs(o);
}
