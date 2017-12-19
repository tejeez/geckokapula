/*
 * ui_hw.h
 * Some hardware-specific parts of UI code
 *
 *  Created on: Dec 3, 2017
 *      Author: Tatu
 */

#ifndef INC_UI_HW_H_
#define INC_UI_HW_H_

#include "em_timer.h"
#include "em_gpio.h"
#include "InitDevice.h"

static inline unsigned get_encoder_position() {
	return TIMER_CounterGet(TIMER1);
}
static inline unsigned get_encoder_button() {
	return GPIO_PinInGet(ENCP_PORT, ENCP_PIN) == 0;
}
static inline unsigned get_ptt() {
	return GPIO_PinInGet(PTT_PORT, PTT_PIN) == 0;
}

#endif /* INC_UI_HW_H_ */
