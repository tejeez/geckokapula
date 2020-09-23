#include "em_system.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "rail.h"
#include "rail_config.h"

RAIL_Handle_t rail;

struct callbackdata {
	RAIL_Time_t time;
	RAIL_Handle_t handle;
	RAIL_Events_t events;
};
struct callbackdata last_callback;

void rail_callback(RAIL_Handle_t rail, RAIL_Events_t events)
{
	last_callback.time = RAIL_GetTime();
	last_callback.handle = rail;
	last_callback.events = events;
}

static RAIL_Config_t railCfg = {
	.eventsCallback = rail_callback,
};

/* Return values for all calls, so that they are easier
 * to check with a debugger. */
struct retvals {
	RAIL_Status_t ConfigCal, ConfigTxPower, SetTxPower, ConfigEvents;
	RAIL_Status_t ConfigChannels, ConfigData;
	uint16_t SetRxFifoThreshold;
	RAIL_Status_t StartRx;
	RAIL_Status_t DelayUs1, DelayUs2;
	RAIL_RadioState_t GetRadioState;
	uint16_t fifo1, fifo2, fifo3;
	int done;
};
struct retvals ret;

void experiments(void)
{
	/* Basic initialization */
	rail = RAIL_Init(&railCfg, NULL);
	ret.ConfigCal = RAIL_ConfigCal(rail, RAIL_CAL_ALL);

	RAIL_TxPowerConfig_t txPowerConfig = {
		.mode = RAIL_TX_POWER_MODE_2P4GIG_HP,
		.voltage = 3300,
		.rampTime = 10,
	};
	ret.ConfigTxPower = RAIL_ConfigTxPower(rail, &txPowerConfig);
	ret.SetTxPower = RAIL_SetTxPower(rail, RAIL_TX_POWER_LEVEL_HP_MAX);
	ret.ConfigEvents = RAIL_ConfigEvents(rail, RAIL_EVENTS_ALL, RAIL_EVENT_RX_FIFO_ALMOST_FULL);


	/* Channel configuration.
	 * Unlike v2 firmware, this now uses the generated config as is */
	RAIL_Idle(rail, RAIL_IDLE_ABORT, true);
	ret.ConfigChannels = RAIL_ConfigChannels(rail, channelConfigs[0], NULL);
	RAIL_DataConfig_t dataConfig = { TX_PACKET_DATA, RX_IQDATA_FILTLSB, FIFO_MODE, FIFO_MODE };
	ret.ConfigData = RAIL_ConfigData(rail, &dataConfig);

	/* Start reception. */
	ret.SetRxFifoThreshold = RAIL_SetRxFifoThreshold(rail, 64);
	ret.StartRx = RAIL_StartRx(rail, 0, NULL);

	/* Check whether the FIFO fills up at all */
	ret.fifo1 = RAIL_GetRxFifoBytesAvailable(rail);
	ret.DelayUs1 = RAIL_DelayUs(10000);
	ret.fifo2 = RAIL_GetRxFifoBytesAvailable(rail);

	/* To make sure above code has run */
	ret.done = 123;

	for(;;)
	{
		ret.fifo3 = RAIL_GetRxFifoBytesAvailable(rail);
		ret.GetRadioState = RAIL_GetRadioState(rail);
	}

}

void init(void)
{
	/* Initialization code, copied from v2 firmware and simplified */
    CHIP_Init();
	CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_EXTERNAL_CLOCK;
	CMU_HFXOInit(&hfxoInit);
	SystemHFXOClockSet(38400000);
	CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
	CMU_HFXOAutostartEnable(0, false, false);
}

int main()
{
	init();
	experiments();
	for(;;);
}
