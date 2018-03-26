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

rig_parameters_t p = {0,1,0, 0, 2400000000, 3 };
rig_status_t rs = {0};

void startrx() {
	RAIL_RfIdleExt(RAIL_IDLE, true);
	RAIL_ResetFifo(false, true);
	RAIL_SetRxFifoThreshold(10); //FIFO size is 512B
	RAIL_EnableRxFifoThreshold();
	RAIL_RxStart(p.channel);
}

RAIL_ChannelConfigEntry_t channelconfigs[] = {{ 0, 20, 2000, 2395000000 }};
const RAIL_ChannelConfig_t channelConfig = { channelconfigs, 1 };
void config_channel() {

	RAIL_RfIdleExt(RAIL_IDLE_ABORT, true);

	channelconfigs[0].baseFrequency = p.frequency;
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

extern int testnumber;
char rail_watchdog = 0;
void rail_task() {
	initRadio();
	uint32_t modulation_test=0;
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
		if(/*1 || */keyed) {
			//RAIL_DebugFrequencyOverride(p.frequency + 100*modulation_test);
			//*(uint32_t*)((void*)0x4008302c) = modulation_test;
			//*(uint32_t*)((void*)0x40083030) = modulation_test;
			/**(uint32_t*)((void*)0x4008303c) = modulation_test;
			*(uint32_t*)((void*)0x40083050) = modulation_test;
			modulation_test += 12345;*/

			//*(uint8_t*)(0x40083000 + 44) = modulation_test; // this transmits 2.2 GHz?

			modulation_test++;
			/*extern void SYNTH_ChannelSet(int, int);
			SYNTH_ChannelSet(modulation_test&15, modulation_test&15);*/

			// there registers are set by SYNTH_ChannelSet
			*(uint32_t*)(uint8_t*)(0x40083000 + 56) = modulation_test;
			//*(uint32_t*)(uint8_t*)(0x40084008) = 128;
		}
		if((!keyed) && (RAIL_RfStateGet() != RAIL_RF_STATE_RX || p.channel_changed)) {
			p.channel_changed = 0;
			RAIL_TxToneStop();
			startrx();
		}
		testnumber++; // to see if RAIL has stuck in some function
		vTaskDelay(2);
	}
}
