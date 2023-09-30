/* SPDX-License-Identifier: MIT */
#ifndef INC_DSP_DRIVER_H_
#define INC_DSP_DRIVER_H_

#include "rail.h"

void dsp_hw_init(void);
void dsp_rtos_init(void);
void fast_dsp_task(void *);
int start_rx_dsp(RAIL_Handle_t rail);
int start_tx_dsp(RAIL_Handle_t rail);

#endif
