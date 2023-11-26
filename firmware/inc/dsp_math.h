/* SPDX-License-Identifier: MIT */
#ifndef INC_DSP_MATH_H_
#define INC_DSP_MATH_H_

#include <stdint.h>

static inline uint32_t approx_angle(float y, float x)
{
	uint32_t angle = 0;
	if (x < 0.0f) {
		x = -x;
		y = -y;
		angle += 0x80000000UL;
	}
	if (y < 0.0f) {
		float x_temp = x;
		x = -y;
		y =  x_temp;
		angle -= 0x40000000UL;
	}
	if (y > x) {
		float x_temp = x;
		x =  x      + y;
		y = -x_temp + y;
		angle += 0x20000000UL;
	}
	// Rotate by -pi/8
	float x_temp = x;
	const float c = 0.4142135623730951f;
	x =  x + c * y;
	y =  y - c * x_temp;
	angle += 0x10000000UL;

	//printf("%f %f\n", atan2f(y, x), y / x);

	// Prevent division by zero
	if (x != 0.0f && x != -0.0f) {
		angle += (uint32_t)(648060518.497f * y / x);
	}
	return angle;
}

#endif
