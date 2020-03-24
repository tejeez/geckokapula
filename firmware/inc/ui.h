/*
 * ui.h
 *
 *  Created on: Nov 30, 2017
 *      Author: Tatu
 */

#ifndef INC_UI_H_
#define INC_UI_H_

/* Before including this, include at least
 * FreeRTOS.h and semphr.h */

void ui_check_buttons(void);
void ui_control_backlight(void);

void display_task(void *arg);
void ui_rtos_init(void);

struct display_ev {
	char text_changed;
	char waterfall_line;
};

/* Display event flags. Tells the display driver
 * that it may have to do something.
 * After setting a flag here, raise display_sem. */
extern volatile struct display_ev display_ev;

/* Semaphore to wake up the display task.
 * Given after setting a display event flag. */
SemaphoreHandle_t display_sem;

#endif /* INC_UI_H_ */
