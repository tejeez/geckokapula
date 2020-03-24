#ifndef DSP_H_
#define DSP_H_
#include <stdint.h>

typedef int16_t iq_in_t[2];
typedef uint16_t audio_out_t;

typedef uint16_t audio_in_t;
typedef uint16_t fm_out_t;

#define AUDIO_MIN 0
#define AUDIO_MAX 200
#define AUDIO_MID 100

int dsp_fast_rx(iq_in_t *in, int in_len, audio_out_t *out, int out_len);
int dsp_fast_tx(audio_in_t *in, fm_out_t *out, int len);

#endif
