#ifndef DSP_H_
#define DSP_H_
#include <stdint.h>

typedef int16_t iq_in_t[2];
typedef uint32_t audio_out_t;

void dsp_process_rx(iq_in_t *in, int in_len, audio_out_t *out, int out_len);

#endif
