/* SPDX-License-Identifier: MIT */

#ifndef INC_RAILTASK_H_
#define INC_RAILTASK_H_

#include "FreeRTOS.h"
#include "semphr.h"

void railtask_main(void *);
void railtask_rtos_init(void);

// Semaphore used to wake up RAIL task when it needs to do something.
extern xSemaphoreHandle railtask_sem;

#endif
