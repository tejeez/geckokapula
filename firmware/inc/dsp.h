#ifndef DSP_H_
#define DSP_H_
#include <stdint.h>

/* Format of the I/Q samples read from RAIL FIFO */
typedef struct {
	int16_t q, i;
} iq_in_t;

/* Format of the samples in audio output buffer */
typedef uint16_t audio_out_t;

/* Format of the samples in audio input buffer */
typedef uint16_t audio_in_t;

/* Format of the samples in FM modulation output buffer */
typedef uint16_t fm_out_t;

/* Format of I/Q buffers used in some signal processing functions */
typedef struct {
	float i, q;
} iq_float_t;

#define AUDIO_MIN 0
#define AUDIO_MAX 200
#define AUDIO_MID 100

// I/Q sample rate
#define RX_IQ_FS 57142

int dsp_fast_rx(iq_in_t *in, int in_len, audio_out_t *out, int out_len);
int dsp_fast_tx(audio_in_t *in, fm_out_t *out, int len);
void dsp_update_params(void);

#endif
