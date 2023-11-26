/* SPDX-License-Identifier: MIT */
#ifndef INC_DSP_MATH_H_
#define INC_DSP_MATH_H_

#include <stdint.h>

static inline uint32_t approx_angle(float y, float x)
{
	uint32_t angle = 0;
	if (x < 0.0f) {
		angle += 0x80000000UL;
		x = -x;
		y = -y;
	}
	if (y < 0.0f) {
		angle -= 0x40000000UL;
		float x_temp = x;
		x = -y;
		y = x_temp;
	}
	if (y > x) {
		angle += 0x20000000UL;
	}
	// TODO: approximate a bit better
	return angle;
}

#endif
