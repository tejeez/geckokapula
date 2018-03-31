/*
 * rail.c
 *
 *  Created on: Dec 19, 2017
 *      Author: Tatu
 */
// RAIL
#include "rail.h"
#include "rail_config.h"
#include "pa.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// rig
#include "rig.h"
#include "dsp_parameters.h"

rig_parameters_t p = {MIDDLECHANNEL,1,0, 0, 2400000000, 1, 3, 5 };
rig_status_t rs = {1};

void startrx() {
	RAIL_RfIdleExt(RAIL_IDLE, true);
	RAIL_ResetFifo(false, true);
	RAIL_SetRxFifoThreshold(400); //FIFO size is 512B
	RAIL_EnableRxFifoThreshold();
	RAIL_RxStart(p.channel);
}

RAIL_ChannelConfigEntry_t channelconfigs[] = {{ 0, 63, CHANNELSPACING, 2395000000 }};
const RAIL_ChannelConfig_t channelConfig = { channelconfigs, 1 };
void config_channel() {

	RAIL_RfIdleExt(RAIL_IDLE_ABORT, true);

	channelconfigs[0].baseFrequency = p.frequency - MIDDLECHANNEL*CHANNELSPACING;
	RAIL_ChannelConfig(&channelConfig);

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
  RAIL_RfIdleExt(RAIL_IDLE, true);

  RAIL_CalInit_t calInit = {
    RAIL_CAL_ALL,
    irCalConfig,
  };
  RAIL_CalInit(&calInit);

  RAIL_PacketLengthConfigFrameType(frameTypeConfigList[0]);
  if (RAIL_RadioConfig((void*)configList[0])) {
    //failed
  }

  RAIL_ChannelConfig(channelConfigs[0]);

  RAIL_DataConfig_t dataConfig = { TX_PACKET_DATA, RX_IQDATA_FILTLSB, FIFO_MODE /*PACKET_MODE*/, FIFO_MODE };
  RAIL_DataConfig(&dataConfig);
}

void RAILCb_TxFifoAlmostEmpty(uint16_t bytes) {
}

/* Skip RAIL asserts to extend the tuning range.
 * Needs linker parameter --wrap=RAILInt_Assert */
void __wrap_RAILInt_Assert() { }

extern int testnumber;
char rail_watchdog = 0, rail_initialized = 0;
void rail_task() {
	initRadio();
	rail_initialized = 1;
	for(;;) {
		unsigned keyed = p.keyed;
		rail_watchdog = 0;
		if(p.channel_changed) {
			config_channel();
		}
		if(keyed && (RAIL_RfStateGet() != RAIL_RF_STATE_TX || p.channel_changed)) {
			p.channel_changed = 0;
			RAIL_RfIdleExt(RAIL_IDLE_ABORT, false);
			RAIL_TxToneStart(p.channel);
			//RAIL_DebugModeSet(1);
		}
		if((!keyed) && (RAIL_RfStateGet() != RAIL_RF_STATE_RX || p.channel_changed)) {
			p.channel_changed = 0;
			RAIL_TxToneStop();
			startrx();
		}
		//testnumber++; // to see if RAIL has stuck in some function
		vTaskDelay(2);
	}
}
