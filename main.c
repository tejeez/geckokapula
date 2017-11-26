/***************************************************************************//**
 * @file main.c
 * @brief Simple RAIL application which includes hal
 * @copyright Copyright 2017 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
#include "rail.h"
#include "hal_common.h"
#include "rail_config.h"
#include "pa.h"

#include "em_chip.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_timer.h"

#include "InitDevice.h"

#include <stdint.h>
#include "arm_math.h"
#include "arm_const_structs.h"

uint8_t nollaa[300] = {255,255,0};

void startrx() {
	RAIL_RfIdleExt(RAIL_IDLE, true);
	RAIL_ResetFifo(false, true);
	RAIL_SetRxFifoThreshold(100); //FIFO size is 512B
	RAIL_EnableRxFifoThreshold();
	RAIL_RxStart(0);
}

void starttx() {
	RAIL_RfIdleExt(RAIL_IDLE_ABORT, true);
	RAIL_ResetFifo(true, false);
	RAIL_SetTxFifoThreshold(100);
	RAIL_WriteTxFifo(nollaa, 300);
	RAIL_TxStart(0, NULL, NULL);
}

void transmit_something() {
	RAIL_TxData_t txstuff = { nollaa, 200 };
	RAIL_RfIdleExt(RAIL_IDLE_ABORT, true);
	RAIL_ResetFifo(true, false);
	RAIL_TxDataLoad(&txstuff);
	RAIL_TxStart(0, NULL, NULL);
}

void initRadio() {
  RAIL_Init_t railInitParams = {
    256,
    RADIO_CONFIG_XTAL_FREQUENCY,
    RAIL_CAL_ALL,
  };
  RADIO_PA_Init(&(RADIO_PAInit_t){
	    PA_SEL_2P4_HP,    /* Power Amplifier mode */
	    PA_VOLTMODE_VBAT, /* Power Amplifier vPA Voltage mode */
	    190,              /* Desired output power in dBm * 10 */
	    0,                /* Output power offset in dBm * 10 */
	    10,               /* Desired ramp time in us */
  });

  //halInit();
  RAIL_RfInit(&railInitParams);
  USART_Tx(USART0, '1');
  RAIL_RfIdleExt(RAIL_IDLE, true);
  USART_Tx(USART0, '2');

  RAIL_CalInit_t calInit = {
    RAIL_CAL_ALL,
    irCalConfig,
  };
  RAIL_CalInit(&calInit);
  USART_Tx(USART0, '3');

  RAIL_PacketLengthConfigFrameType(frameTypeConfigList[0]);
  USART_Tx(USART0, '4');
  if (RAIL_RadioConfig((void*)configList[0])) {
    //while (1) ;
	  USART_Tx(USART0, 'f');
  }
  USART_Tx(USART0, '5');

  RAIL_ChannelConfig(channelConfigs[0]);
  USART_Tx(USART0, '6');

  RAIL_DataConfig_t dataConfig = { TX_PACKET_DATA, RX_IQDATA_FILTLSB, FIFO_MODE /*PACKET_MODE*/, FIFO_MODE };
  RAIL_DataConfig(&dataConfig);
  USART_Tx(USART0, '7');
}

#define FFTLEN 128
const arm_cfft_instance_f32 *fftS = &arm_cfft_sR_f32_len128;
float fftbuf[2*FFTLEN];
volatile int fftbufp = 0;

#define RXBUFL 2
typedef int16_t iqsample_t[2];
iqsample_t rxbuf[RXBUFL];
void RAILCb_RxFifoAlmostFull(uint16_t bytesAvailable) {
	unsigned nread, i;
	static int psi=0, psq=0;
	GPIO_PortOutToggle(gpioPortF, 4);
	nread = RAIL_ReadRxFifo((uint8_t*)rxbuf, 4*RXBUFL);
	nread /= 4;
	int ssi=0, ssq=0, fm = 0;
	for(i=0; i<nread; i++) {
		int si=rxbuf[i][0], sq=rxbuf[i][1];
		int fi, fq;
		// multiply by conjugate
		fi = si * psi + sq * psq;
		fq = sq * psi - si * psq;
		/* Scale maximum absolute value to 0x7FFF.
		 * This can be done because FM demod doesn't care about amplitude.
		 */
		if(fi > 0x7FFF || fi < -0x7FFF || fq > 0x7FFF || fq < -0x7FFF) {
			fi /= 0x10000; fq /= 0x10000;
		}
		// very crude approximation...
		fm += 0x8000 * fq / ((fi>=0?fi:-fi) + (fq>=0?fq:-fq));

		psi = si; psq = sq;
		ssi += si; ssq += sq;
	}

	int fp = fftbufp;
	if(fp < 2*FFTLEN) {
		const float scaling = 1.0f / (RXBUFL*0x8000);
		fftbuf[fp]   = scaling*ssi;
		fftbuf[fp+1] = scaling*ssq;
		fftbufp = fp+2;
	}

	fm = (fm / 0x100) + 100;
	if(fm < 0) fm = 0;
	if(fm > 200) fm = 200;
	TIMER_CompareBufSet(TIMER0, 0, fm);
	//USART_Tx(USART0, 'r');
}

void RAILCb_TxFifoAlmostEmpty(uint16_t bytes) {
	GPIO_PortOutToggle(gpioPortF, 4);
	RAIL_WriteTxFifo(nollaa, 100);
	USART_Tx(USART0, 'e');
}

void display_loop();
void display_fft_line(float *data);


int main(void) {
	enter_DefaultMode_from_RESET();
	USART_Tx(USART0, 'a');
	initRadio();
 	USART_Tx(USART0, 'b');

 	TIMER_TopSet(TIMER0, 200);
 	TIMER_CompareBufSet(TIMER0, 0, 33);

	for(;;) {
		unsigned keyed = !GPIO_PinInGet(PTT_PORT, PTT_PIN);
		if(keyed && RAIL_RfStateGet() != RAIL_RF_STATE_TX) {
			USART_Tx(USART0, 'x');
			RAIL_RfIdleExt(RAIL_IDLE_ABORT, false);
			RAIL_TxToneStart(0);
		}
		if((!keyed) && RAIL_RfStateGet() != RAIL_RF_STATE_RX) {
			RAIL_TxToneStop();
			startrx();
		}

		if(fftbufp >= 2*FFTLEN) {
			arm_cfft_f32(fftS, fftbuf, 0, 1);
			display_fft_line(fftbuf);
			fftbufp = 0;
		}

		//USART_Tx(USART0, 'y');
		display_loop();
		GPIO_PortOutSet(gpioPortF, 5);
		GPIO_PortOutClear(gpioPortF, 5);
	}
	return 0;
}
