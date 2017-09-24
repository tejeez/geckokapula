/***************************************************************************//**
 * @file main.c
 * @brief Simple RAIL application which includes hal
 * @copyright Copyright 2017 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
#include "rail.h"
#include "hal_common.h"
#include "rail_config.h"

#include "em_chip.h"
#include "em_usart.h"
#include "em_gpio.h"

#include "InitDevice.h"

void startrx() {
  RAIL_RfIdleExt(RAIL_IDLE, true);
  RAIL_ResetFifo(false, true);
  RAIL_SetRxFifoThreshold(100); //FIFO size is 512B
  RAIL_EnableRxFifoThreshold();
  RAIL_RxStart(0);
}

void initRadio() {
  RAIL_Init_t railInitParams = {
    256,
    RADIO_CONFIG_XTAL_FREQUENCY,
    RAIL_CAL_ALL,
  };
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

  RAIL_DataConfig_t dataConfig = { TX_PACKET_DATA, RX_IQDATA_FILTLSB, FIFO_MODE, FIFO_MODE };
  RAIL_DataConfig(&dataConfig);
  USART_Tx(USART0, '7');
}

void RAILCb_RxFifoAlmostFull(uint16_t bytesAvailable) {
	uint8_t rxbuf[32];
	//USART_Tx(USART0, bytesAvailable);
	GPIO_PortOutToggle(gpioPortF, 4);
	RAIL_ReadRxFifo(rxbuf, 32);
}

void display_loop();

int main(void) {
	enter_DefaultMode_from_RESET();
	USART_Tx(USART0, 'a');
	initRadio();
	USART_Tx(USART0, 'b');

	startrx();
	USART_Tx(USART0, 'c');

	for(;;) {
		display_loop();
		volatile unsigned a;
		for(a=0; a<10000; a++);
	}
	return 0;
}
