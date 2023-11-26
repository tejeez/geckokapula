/* SPDX-License-Identifier: MIT */
// Test with:
// gcc test_dsp_math.c -o test_dsp_math -O2 -lm -Wall -Wextra && ./test_dsp_math

#include <math.h>
#include <stdio.h>
#include "../inc/dsp_math.h"

int main(void)
{
	float angle;
	for (angle = 0; angle < (float)(M_PI*2); angle += (float)(M_PI/32)) {
		float x = cosf(angle), y = sinf(angle);
		uint32_t exact = (uint32_t)(atan2f(y, x) * 6.8356528e+08f);
		uint32_t approx = approx_angle(y,x);
		printf("%10.7f %8x %8x %10d\n",
			angle, exact, approx,
			(int)((int32_t)(approx - exact))
		);
	}
	return 0;
}
