/* SPDX-License-Identifier: MIT */

#include <stdlib.h>

// CMSIS
#include "arm_math.h"
#include "arm_const_structs.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// rig
#include "rig.h"
#include "ui.h"
#include "ui_parameters.h"

#include "dsp.h"
#include <math.h>

#define AUDIO_MAXLEN 32
#define IQ_MAXLEN (AUDIO_MAXLEN * 2)

extern rig_parameters_t p;

/* Waterfall FFT related things */
const arm_cfft_instance_f32 *fftS = &arm_cfft_sR_f32_len256;
#define SIGNALBUFLEN 512
int16_t signalbuf[2*SIGNALBUFLEN];
QueueHandle_t fft_queue;


// State of a biquad filter
struct biquad_state {
	float s1_i, s1_q, s2_i, s2_q;
};

// Coefficients of a biquad filter
struct biquad_coeff {
	float a1, a2, b0, b1, b2;
};

/* Apply a biquad filter to a complex signal with real coefficients,
 * i.e. run it separately for the I and Q parts.
 * Write output back to the same buffer.
 * The algorithm used is called transposed direct form II, as shown at
 * https://www.dsprelated.com/freebooks/filters/Transposed_Direct_Forms.html
 *
 * The code could possibly be optimized by unrolling a couple of times
 * or by cascading multiple stages in a single loop.
 * We'll need some benchmarks to test such ideas.
 */
void biquad_filter(struct biquad_state *s, const struct biquad_coeff *c, iq_float_t *buf, unsigned len)
{
	unsigned i;
	const float a1 = -c->a1, a2 = -c->a2, b0 = c->b0, b1 = c->b1, b2 = c->b2;

	float
	s1_i = s->s1_i,
	s1_q = s->s1_q,
	s2_i = s->s2_i,
	s2_q = s->s2_q;

	for (i = 0; i < len; i++) {
		float in_i, in_q, out_i, out_q;
		in_i = buf[i].i;
		in_q = buf[i].q;
		out_i = s1_i + b0 * in_i;
		out_q = s1_q + b0 * in_q;
		s1_i  = s2_i + b1 * in_i + a1 * out_i;
		s1_q  = s2_q + b1 * in_q + a1 * out_q;
		s2_i  =        b2 * in_i + a2 * out_i;
		s2_q  =        b2 * in_q + a2 * out_q;
		buf[i].i = out_i;
		buf[i].q = out_q;
	}

	s->s1_i = s1_i;
	s->s1_q = s1_q;
	s->s2_i = s2_i;
	s->s2_q = s2_q;
}


/* Demodulator state */
struct demod {
	// Audio gain parameter
	float audiogain;

	/* Phase of the digital down-converter,
	 * i.e. first oscillator used in SSB demodulation */
	float ddc_i, ddc_q;
	// Frequency of the digital down-converter
	float ddcfreq_i, ddcfreq_q;

	// Phase of the second oscillator in SSB demodulation
	float bfo_i, bfo_q;
	// Frequency of the second oscillator in SSB demodulation
	float bfofreq_i, bfofreq_q;

	// Previous sample stored by FM demodulator
	float fm_prev_i, fm_prev_q;

	// Audio filter state
	float audio_lpf, audio_hpf, audio_po;

	// AGC state
	float agc_amp;

	// Squelch state
	float diff_avg, squelch;

	// S-meter state
	uint64_t smeter_acc;
	unsigned smeter_count;

	unsigned signalbufp;

	// Biquad filter states, used in SSB demodulation
	struct biquad_state bq1, bq2;

	enum rig_mode prev_mode;
};


static void demod_reset(struct demod *ds)
{
	ds->fm_prev_i = ds->fm_prev_q = 0;
	ds->audio_lpf = ds->audio_hpf = ds->audio_po = 0;
	ds->agc_amp = 0;
	ds->diff_avg = 0;
	ds->bfo_i = 1; ds->bfo_q = 0;
	ds->ddc_i = 1; ds->ddc_q = 0;
	memset(&ds->bq1, 0, sizeof(ds->bq1));
	memset(&ds->bq2, 0, sizeof(ds->bq2));
}


/* Store samples for waterfall FFT, decimating by 2.
 * Also calculate total signal power for S-meter. */
void demod_store(struct demod *ds, iq_in_t *in, unsigned len)
{
	unsigned i, fp = ds->signalbufp;
	uint64_t acc = ds->smeter_acc;
	for (i = 0; i < len; i += 2) {
		int32_t s0i, s0q, s1i, s1q;
		s0i = in[i].i;
		s0q = in[i].q;
		s1i = in[i+1].i;
		s1q = in[i+1].q;
		signalbuf[fp] = s0i + s1i;
		signalbuf[fp+1] = s0q + s1q;
		acc += s0i * s0i + s0q * s0q;
		acc += s1i * s1i + s1q * s1q;
		fp = (fp + 2) & (SIGNALBUFLEN-2);
		if (fp == 0 || fp == 171*2 || fp == 341*2) {
			uint16_t msg = fp;
			if (!xQueueSend(fft_queue, &msg, 0)) {
				//++diag.fft_overflows;
			}
		}
	}
	if((ds->smeter_count += len) >= 0x4000) {
		/* Update S-meter value on display */
		rs.smeter = acc / 0x4000;
		acc = 0;
		ds->smeter_count = 0;

		display_ev.text_changed = 1;
		xSemaphoreGive(display_sem);
	}
	ds->signalbufp = fp;
	ds->smeter_acc = acc;
}



/* FM demodulate a buffer.
 * Each I/Q sample is multiplied by the conjugate of the previous sample,
 * giving a value whose complex argument is proportional to the frequency.
 *
 * Instead of actually calculating the argument, a very crude approximation
 * for small values is used instead, but it sounds "good enough" since
 * the input signal is somewhat oversampled.
 *
 * The multiplication results in numbers with a big dynamic range, so
 * floating point math is used.
 *
 * The loop is unrolled two times, so we can nicely reuse the previous
 * sample values already loaded and converted without storing them in
 * another variable.
 * Also, the audio output gets decimated by two by just
 * "integrate and dump". Again, sounds good enough given the oversampling.
 *
 * Average amplitude of differentiated signal is used for squelch.
 */
/*static inline*/ void demod_fm(struct demod *ds, iq_in_t *in, float *out, unsigned len)
{
	unsigned i;
	float s0i, s0q, s1i, s1q;
	s0i = ds->fm_prev_i;
	s0q = ds->fm_prev_q;

	float prev_fm = ds->audio_po, diff_amp = 0;

	for (i = 0; i < len; i+=2) {
		float fi, fq, fm;
		s1i = in[i].i;
		s1q = in[i].q;
		fi = s1i * s0i + s1q * s0q;
		fq = s1q * s0i - s1i * s0q;
		fm = fq / (fabsf(fi) + fabsf(fq));

		s0i = in[i+1].i;
		s0q = in[i+1].q;
		fi += s0i * s1i + s0q * s1q;
		fq += s0q * s1i - s0i * s1q;
		fm += fq / (fabsf(fi) + fabsf(fq));

		// Avoid NaN
		if (fm != fm)
			fm = 0;

		out[i/2] = fm;
		diff_amp += fabsf(fm - prev_fm);
		prev_fm = fm;
	}
	ds->fm_prev_i = s0i;
	ds->fm_prev_q = s0q;

	ds->audio_po = prev_fm;
	float diff_avg = ds->diff_avg;
	if (diff_avg != diff_avg) diff_avg = 0;
	ds->diff_avg = diff_avg + (diff_amp - diff_avg) * .02f;
}


/* Demodulate AM.
 * Again, output audio is decimated by 2.
 *
 * An approximation explained here is used:
 * https://dspguru.com/dsp/tricks/magnitude-estimator/
 */
/*static inline*/ void demod_am(struct demod *ds, iq_in_t *in, float *out, unsigned len)
{
	(void)ds;
	unsigned i;
	const float beta = 0.4142f;
	for (i = 0; i < len; i+=2) {
		float ai, aq, o;
		ai = fabsf((float)in[i].i);
		aq = fabsf((float)in[i].q);
		o = (ai >= aq) ? (ai + aq * beta) : (aq + ai * beta);
		ai = fabsf((float)in[i+1].i);
		aq = fabsf((float)in[i+1].q);
		o += (ai >= aq) ? (ai + aq * beta) : (aq + ai * beta);
		out[i/2] = o;
	}
}


/* Digital down-conversion.
 * This is the first mixer in the Weaver method SSB demodulator.
 *
 * Multiply the signal by a complex oscillator
 * and decimate the result by 2.
 *
 * The oscillator is implemented by "rotating" a complex number on
 * each sample by multiplying it with a value on the unit circle.
 * The value is normalized once per block using formula from
 * https://dspguru.com/dsp/howtos/how-to-create-oscillators-in-software/
 *
 * The previous and next oscillator values alternate between variables
 * osc0 and osc1, and the loop is unrolled for 2 input samples.
 * */
void demod_ddc(struct demod *ds, iq_in_t *in, iq_float_t *out, unsigned len)
{
	(void)ds;
	unsigned i;
	float osc1i, osc1q;
	float osc0i = ds->ddc_i, osc0q = ds->ddc_q;
	const float oscfi = ds->ddcfreq_i, oscfq = ds->ddcfreq_q;
	len /= 2;
	for (i = 0; i < len; i++) {
		float oi, oq, ii, iq;
		ii = in->i;
		iq = in->q;
		in++;
		oi    = osc0i * ii    - osc0q * iq;
		oq    = osc0i * iq    + osc0q * ii;

		osc1i = osc0i * oscfi - osc0q * oscfq;
		osc1q = osc0i * oscfq + osc0q * oscfi;

		ii = in->i;
		iq = in->q;
		in++;
		oi   += osc1i * ii    - osc1q * iq;
		oq   += osc1i * iq    + osc1q * ii;

		osc0i = osc1i * oscfi - osc1q * oscfq;
		osc0q = osc1i * oscfq + osc1q * oscfi;

		out[i].i = oi;
		out[i].q = oq;
	}
	float ms = osc0i * osc0i + osc0q * osc0q;
	ms = (3.0f - ms) * 0.5f;
	ds->ddc_i = ms * osc0i;
	ds->ddc_q = ms * osc0q;
}


/* Demodulate DSB with input in floating point format.
 * This is the second mixer in the Weaver SSB demodulator.
 *
 * Multiply the signal by a beat-frequency oscillator and take
 * the real part of the result.
 *
 * The oscillator is implemented by "rotating" a complex number on
 * each sample by multiplying it with a value on the unit circle.
 * The value is normalized once per block using formula from
 * https://dspguru.com/dsp/howtos/how-to-create-oscillators-in-software/
 *
 * The previous and next oscillator values alternate between variables
 * osc0 and osc1, and the loop is unrolled for 2 input samples.
 * */
void demod_dsb_f(struct demod *ds, iq_float_t *in, float *out, unsigned len)
{
	(void)ds;
	unsigned i;
	float osc1i, osc1q;
	float osc0i = ds->bfo_i, osc0q = ds->bfo_q;
	const float oscfi = ds->bfofreq_i, oscfq = ds->bfofreq_q;
	for (i = 0; i < len; i+=2) {
		out[i] = osc0i * in[i].i - osc0q * in[i].q;
		osc1i = osc0i * oscfi - osc0q * oscfq;
		osc1q = osc0i * oscfq + osc0q * oscfi;

		out[i+1] = osc1i * in[i+1].i - osc1q * in[i+1].q;
		osc0i = osc1i * oscfi - osc1q * oscfq;
		osc0q = osc1i * oscfq + osc1q * oscfi;
	}
	float ms = osc0i * osc0i + osc0q * osc0q;
	ms = (3.0f - ms) * 0.5f;
	ds->bfo_i = ms * osc0i;
	ds->bfo_q = ms * osc0q;
}


/* Coefficients from https://arachnoid.com/BiQuadDesigner/
 * with: 1000 Hz, sample rate 24000 Hz, Q 0.8.
 * Now the same coefficients are used for two stages, but a better
 * response could be obtained by designing a proper 2-stage filter. */
static const struct biquad_coeff biquad1_ssb = {
	.a1 = -1.66286366f,
	.a2 =  0.72152314f,
	.b0 =  0.01466487f,
	.b1 =  0.02932974f,
	.b2 =  0.01466487f
};

/* Demodulate SSB.
 * The Weaver method is used.
 */
void demod_ssb(struct demod *ds, iq_in_t *in, float *out, unsigned len)
{
	iq_float_t buf[IQ_MAXLEN];

	demod_ddc(ds, in, buf, len);
	len /= 2;
	biquad_filter(&ds->bq1, &biquad1_ssb, buf, len);
	biquad_filter(&ds->bq2, &biquad1_ssb, buf, len);
	demod_dsb_f(ds, buf, out, len);
}



/* Apply some low-pass filtering to audio for de-emphasis
 * and some high-pass filtering for DC blocking.
 * Store the result in the same buffer.
 *
 * Also calculate the average amplitude which is used for AGC.
 */
/*static inline*/ void demod_audio_filter(struct demod *ds, float *buf, unsigned len)
{
	unsigned i;
	const float lpf_a = 0.1f, hpf_a = 0.001f;
	float
	lpf = ds->audio_lpf,
	hpf = ds->audio_hpf;
	float amp = 0;
	for (i = 0; i < len; i++) {
		lpf += (buf[i] - lpf) * lpf_a;
		hpf += (lpf - hpf) * hpf_a;
		float o = lpf - hpf;
		buf[i] = o;

		amp += fabsf(o);
	}
	ds->audio_lpf = lpf;
	ds->audio_hpf = hpf;

	/* Update AGC values once per block, so most of the AGC code
	 * runs at a lower sample rate. */
	const float agc_attack = 0.1f, agc_decay = 0.01f;
	float agc_amp = ds->agc_amp;
	// Avoid NaN
	if (agc_amp != agc_amp)
		agc_amp = 0;

	float d = amp - agc_amp;
	if (d >= 0)
		ds->agc_amp = agc_amp + d * agc_attack;
	else
		ds->agc_amp = agc_amp + d * agc_decay;
}


/*static inline*/ void demod_convert_audio(float *in, audio_out_t *out, unsigned len, float gain)
{
	unsigned i;
	for (i = 0; i < len; i++) {
		float f = gain * in[i] + (float)AUDIO_MID;
		audio_out_t o;
		if (f <= (float)AUDIO_MIN)
			o = AUDIO_MIN;
		else if(f >= (float)AUDIO_MAX)
			o = AUDIO_MAX;
		else
			o = (audio_out_t)f;
		out[i] = o;
	}
}


struct demod demodstate;

/* Function to convert received IQ to output audio */
int dsp_fast_rx(iq_in_t *in, int in_len, audio_out_t *out, int out_len)
{
	if (out_len * 2 != in_len || out_len > AUDIO_MAXLEN)
		return 0;

	demod_store(&demodstate, in, in_len);

	enum rig_mode mode = p.mode;
	float audio[AUDIO_MAXLEN];
	switch(mode) {
	case MODE_FM:
		demod_fm(&demodstate, in, audio, in_len);
		break;
	case MODE_AM:
		demod_am(&demodstate, in, audio, in_len);
		break;
	case MODE_DSB:
		demod_ssb(&demodstate, in, audio, in_len);
		break;
	/*case MODE_SSB:
		demod_ssb(&demodstate, in, audio, in_len);
		break;*/
	default:
		break;
	}

	if (demodstate.diff_avg < demodstate.squelch) {
		// Squelch open
		demod_audio_filter(&demodstate, audio, out_len);
		demod_convert_audio(audio, out, out_len, demodstate.audiogain / demodstate.agc_amp);
	} else {
		// Squelch closed
		int i;
		for (i = 0; i < out_len; i++)
			out[i] = AUDIO_MID;
	}

	return out_len;
}


void dsp_update_params(void)
{
	const float bfo_hz = 1200.0f;
	float f;

	f = (6.2831853f * 2.0f / RX_IQ_FS) * bfo_hz;
	demodstate.bfofreq_i = cosf(f);
	demodstate.bfofreq_q = sinf(f);

	f = (-6.2831853f / RX_IQ_FS) * ((float)p.offset_freq + bfo_hz);
	demodstate.ddcfreq_i = cosf(f);
	demodstate.ddcfreq_q = sinf(f);

	unsigned vola = p.volume;
	demodstate.audiogain = ((vola&1) ? (3<<(vola/2)) : (2<<(vola/2))) * 10.0f;

	demodstate.squelch = 1.0f * p.squelch;

	enum rig_mode mode = p.mode;
	/* Reset state after mode change */
	if (mode != demodstate.prev_mode) {
		demod_reset(&demodstate);
		demodstate.prev_mode = mode;
	}
}


/* Modulator state */
struct modstate {
	float lpf, hpf, agc_amp;
};

struct modstate modstate;


/* Function to convert input audio to transmit frequency modulation */
static int dsp_fast_tx_fm(audio_in_t *in, fm_out_t *out, int len)
{
	float hpf = modstate.hpf, lpf = modstate.lpf;
	float agc_amp = modstate.agc_amp;

	int i;
	for (i = 0; i < len; i++) {
		float audioin = in[i];
		// DC block, highpass for pre-emphasis
		hpf += (audioin - hpf) * .1f;
		audioin -= hpf;
		// lowpass
		lpf += (audioin - lpf) * .2f;

		float amp = fabsf(audioin);

		const float agc_attack = 0.01f, agc_decay = 0.001f;
		// Avoid NaN, clamp to a minimum value
		if (agc_amp != agc_amp || agc_amp < .01f)
			agc_amp = .01f;

		float d = amp - agc_amp;
		if (d >= 0)
			agc_amp = agc_amp + d * agc_attack;
		else
			agc_amp = agc_amp + d * agc_decay;

		int32_t audioout = audioin * (50.0f / agc_amp) + 32.0f;
		if(audioout <= 6)  audioout = 6;
		if(audioout >= 57) audioout = 57;
		out[i] = audioout;
	}

	modstate.lpf = lpf;
	modstate.hpf = hpf;
	modstate.agc_amp = agc_amp;
	return len;
}


/* Function to convert input audio to transmit frequency modulation */
int dsp_fast_tx(audio_in_t *in, fm_out_t *out, int len)
{
	if (p.mode == MODE_FM) {
		dsp_fast_tx_fm(in, out, len);
	} else {
		// Transmit unmodulated carrier on other modes
		int i;
		for (i = 0; i < len; i++) {
			out[i] = 32;
		}
	}
}


static void calculate_waterfall_line(unsigned sbp)
{
	extern uint8_t displaybuf2[3*(FFT_BIN2-FFT_BIN1)];
	unsigned i;
	float mag_avg = 0;

	/* These are static because so such big arrays would not be allocated from stack.
	 * Not sure if this is a good idea.
	 * If averaging were not used, mag could actually reuse fftdata
	 * with some changes to indexing.
	 */
	static float fftdata[2*FFTLEN], mag[FFTLEN];
	static uint8_t averages = 0;

	/* sbp is the message received from the fast DSP task,
	 * containing the index of the latest sample written by it.
	 * Take one FFT worth of previous samples before it. */
	sbp -= 2*FFTLEN;

	for(i=0; i<2*FFTLEN; i+=2) {
		sbp &= SIGNALBUFLEN-1;
		fftdata[i]   = signalbuf[sbp];
		fftdata[i+1] = signalbuf[sbp+1];
		sbp += 2;
	}

	arm_cfft_f32(fftS, fftdata, 0, 1);

	if(averages == 0)
		for(i=0;i<FFTLEN;i++) mag[i] = 0;
	for(i=0;i<FFTLEN;i++) {
		float fft_i = fftdata[2*i], fft_q = fftdata[2*i+1];
		mag_avg +=
		mag[i ^ (FFTLEN/2)] += fft_i*fft_i + fft_q*fft_q;
	}
	averages++;
	if(averages < p.waterfall_averages)
		return;
	averages = 0;
	mag_avg = (130.0f*FFTLEN) / mag_avg;

	uint8_t *bufp = displaybuf2;
	for(i=FFT_BIN1;i<FFT_BIN2;i++) {
		unsigned v = mag[i] * mag_avg;
		if(v < 0x100) {  // black to blue
			bufp[0] = v / 2;
			bufp[1] = 0;
			bufp[2] = v;
		} else if(v < 0x200) { // blue to yellow
			bufp[0] = v / 2;
			bufp[1] = v - 0x100;
			bufp[2] = 0x1FF - v;
		} else if(v < 0x300) { // yellow to white
			bufp[0] = 0xFF;
			bufp[1] = 0xFF;
			bufp[2] = v - 0x200;
		} else { // white
			bufp[0] = 0xFF;
			bufp[1] = 0xFF;
			bufp[2] = 0xFF;
		}
		bufp += 3;
	}

	display_ev.waterfall_line = 1;
	xSemaphoreGive(display_sem);
}


/* A task for DSP operations that can take a longer time */
void slow_dsp_task(void *arg) {
	(void)arg;
	for(;;) {
		uint16_t msg;
		if (xQueueReceive(fft_queue, &msg, portMAX_DELAY)) {
			calculate_waterfall_line(msg);
		}
	}
}


void slow_dsp_rtos_init(void)
{
	fft_queue = xQueueCreate(1, sizeof(uint16_t));
}
