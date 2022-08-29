/* SPDX-License-Identifier: MIT */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include <stdint.h>

int display_init(void);
int display_ready(void);

void display_area(int x1,int y1,int x2,int y2);
void display_start(void);
void display_end(void);
void display_pixel(uint8_t r, uint8_t g, uint8_t b);
void display_transfer(const uint8_t *dmadata, int dmalen);
void display_scroll(unsigned y);
void display_backlight(int b);

#endif /* INC_DISPLAY_H_ */
