/*
 * rail.c
 *
 *  Created on: Dec 19, 2017
 *      Author: Tatu
 */
// RAIL
#ifndef DISABLE_RAIL
#include "rail.h"
#include "rail_config.h"
#endif

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// rig
#include "rig.h"

#include <stdio.h>

#define CHANNELSPACING 147 // 38.4 MHz / 2^18
#define MIDDLECHANNEL 32

rig_parameters_t p = {MIDDLECHANNEL,1,0, 0, 2400000000, 11, 96, 5, 29 };
rig_status_t rs = {0};

#ifndef DISABLE_RAIL
RAIL_Handle_t rail;

int start_rx_dsp(RAIL_Handle_t rail);
int start_tx_dsp(RAIL_Handle_t rail);


RAIL_ChannelConfigEntryAttr_t generated_entryAttr = {
  { 0xFFFFFFFFUL }
};

RAIL_ChannelConfigEntry_t channelconfig_entry[] = {
	{
		.phyConfigDeltaAdd = NULL,
		.baseFrequency = 2395000000UL,
		.channelSpacing = CHANNELSPACING,
		.physicalChannelOffset = 0,
		.channelNumberStart = 0,
		.channelNumberEnd = 63,
		.maxPower = RAIL_TX_POWER_MAX,
		.attr = &generated_entryAttr
	}
};

extern const uint32_t generated[];
const RAIL_ChannelConfig_t channelConfig = {
	generated,
	NULL,
	channelconfig_entry,
	1,
	0
};


void config_channel() {
	unsigned r;
	RAIL_Idle(rail, RAIL_IDLE_ABORT, true);

	channelconfig_entry[0].baseFrequency = p.frequency - MIDDLECHANNEL*CHANNELSPACING;
	r = RAIL_ConfigChannels(rail, &channelConfig, NULL);
	printf("RAIL_ConfigChannels (2): %u\n", r);
}


void rail_callback(RAIL_Handle_t rail, RAIL_Events_t events);

#if 0
void RxFifoAlmostFull_callback(RAIL_Handle_t rail);
void rail_callback(RAIL_Handle_t rail, RAIL_Events_t events)
{
	/* In RAIL 2, all events use this same callback.
	 * Check flags and call the old callback function. */
	if (events & RAIL_EVENT_RX_FIFO_ALMOST_FULL) {
		RxFifoAlmostFull_callback(rail);
	}
}
#endif

static RAIL_Config_t railCfg = {
	.eventsCallback = &rail_callback,
};

void initRadio() {
	unsigned r;
	rail = RAIL_Init(&railCfg, NULL);
	r = RAIL_ConfigCal(rail, RAIL_CAL_ALL);
	printf("RAIL_ConfigCal: %u\n", r);
	r = RAIL_ConfigChannels(rail, &channelConfig, NULL);
	printf("RAIL_ConfigChannels (1): %u\n", r);

	RAIL_TxPowerConfig_t txPowerConfig = {
		.mode = RAIL_TX_POWER_MODE_2P4GIG_HP,
		.voltage = 3300,
		.rampTime = 10,
	};
	r = RAIL_ConfigTxPower(rail, &txPowerConfig);
	printf("RAIL_ConfigTxPower: %u\n", r);
	r = RAIL_SetTxPower(rail, RAIL_TX_POWER_LEVEL_HP_MAX);
	printf("RAIL_SetTxPower: %u\n", r);

	r = RAIL_ConfigEvents(rail, RAIL_EVENTS_ALL, RAIL_EVENT_RX_FIFO_ALMOST_FULL);
	printf("RAIL_ConfigEvents: %u\n", r);

	RAIL_DataConfig_t dataConfig = { TX_PACKET_DATA, RX_IQDATA_FILTLSB, FIFO_MODE /*PACKET_MODE*/, FIFO_MODE };
	r = RAIL_ConfigData(rail, &dataConfig);
	printf("RAIL_ConfigData: %u\n", r);
}


/* Skip RAIL asserts to extend the tuning range.
 * Needs linker parameter --wrap=RAILInt_Assert */
void __wrap_RAILInt_Assert() { }

extern int testnumber;
char rail_watchdog = 0;
void rail_task() {
	initRadio();
	for(;;) {
		unsigned keyed = p.keyed;
		rail_watchdog = 0;
		/* Changing channel configuration here seems to break something,
		 * so don't call it for now. Maybe events or data have to be
		 * reconfigured after it. */
		if(0 && p.channel_changed) {
			config_channel();
		}

		unsigned r;
		RAIL_RadioState_t rs = RAIL_GetRadioState(rail);
		printf("RAIL_GetRadioState: %08x\n", rs);
		if(keyed && ((rs & RAIL_RF_STATE_TX) == 0 || p.channel_changed)) {
			p.channel_changed = 0;
			RAIL_Idle(rail, RAIL_IDLE_ABORT, false);
			r = RAIL_StartTxStream(rail, p.channel, RAIL_STREAM_CARRIER_WAVE);
			printf("RAIL_StartTxStream: %u\n", r);
			start_tx_dsp(rail);
		}
		if((!keyed) && ((rs & RAIL_RF_STATE_RX) == 0 || p.channel_changed)) {
			p.channel_changed = 0;
			if (rs & RAIL_RF_STATE_TX)
				RAIL_StopTxStream(rail);
			RAIL_Idle(rail, RAIL_IDLE_ABORT, true);
			start_rx_dsp(rail);
		}
		//testnumber++; // to see if RAIL has stuck in some function

		// TODO: receive commands from queue instead of polling with delay
		vTaskDelay(200);
	}
}
#else
char rail_watchdog = 0;
void rail_task(void)
{
	for (;;)
		vTaskDelay(1000);
}
#endif
