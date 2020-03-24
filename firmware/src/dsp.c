/*
 * dsp.c
 *
 *  Created on: Mar 26, 2018
 *      Author: Tatu
 */

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

#define AUDIO_MAXLEN 32
#define IQ_MAXLEN (AUDIO_MAXLEN * 2)

extern rig_parameters_t p;

/* Waterfall FFT related things */
const arm_cfft_instance_f32 *fftS = &arm_cfft_sR_f32_len256;
#define SIGNALBUFLEN 512
int16_t signalbuf[2*SIGNALBUFLEN];
unsigned signalbufp = 0;
QueueHandle_t fft_queue;


/* TODO: Reimplement S-meter and squelch */

#if 0
#define RXBUFL 2

/* Process some IQ samples and return one audio sample.
 * The plan is to allow processing in bigger blocks, but for now
 * this is an intermediate step in the ongoing refactoring. */
static inline audio_out_t dsp_process_rx_sample(iq_in_t *rxbuf)
{
	unsigned nread, i;
	static int psi=0, psq=0;
	static int agc_level=0;
	static int audioout_prev=0, squelchlpf=0;
	int ssi=0, ssq=0, audioout = 0;
	static unsigned smeter_count = 0;
	static uint64_t smeter_acc = 0;
	static int audio_lpf = 0, audio_hpf = 0;

	nread = RXBUFL;
	for (i=0; i < nread; i++) {
		int si=rxbuf[i][1], sq=rxbuf[i][0]; // I and Q swapped
		int fi, fq;
		switch(p.mode) {
		case MODE_FM: {
			int fm;
			// multiply by conjugate
			fi = si * psi + sq * psq;
			fq = sq * psi - si * psq;
			/* Scale maximum absolute value to 0x7FFF.
			 * This can be done because FM demod doesn't care about amplitude.
			 */
			while(fi > 0x7FFF || fi < -0x7FFF || fq > 0x7FFF || fq < -0x7FFF) {
				fi /= 0x100; fq /= 0x100;
			}
			// very crude approximation...
			fm = 0x8000 * fq / ((fi>=0?fi:-fi) + (fq>=0?fq:-fq));
			audio_lpf += (fm*128 - audio_lpf)/16;
			audioout = audio_lpf/128;
			break; }
		case MODE_AM:
		case MODE_DSB: {
			int agc_1, agc_diff;
			int raw_audio;
			if(p.mode == MODE_AM)
				raw_audio = abs(si) + abs(sq);
			else
				raw_audio = si*2;
			audio_lpf += (raw_audio*64 - audio_lpf)/16;
			audio_hpf += (audio_lpf - audio_hpf) / 512; // DC block
			fi = (audio_lpf-audio_hpf)/128; // TODO: SSB filter

			// AGC
			agc_1 = abs(fi) * 0x100; // rectify
			agc_diff = agc_1 - agc_level;
			if(agc_diff > 0)
				agc_level += agc_diff/256;
			else
				agc_level += agc_diff/2048;

			audioout += 0x1000 * fi / (agc_level/0x100);

			break; }
		default: {
			audioout = 0;
			break;
		}
		}

		psi = si; psq = sq;
		ssi += si; ssq += sq;

		smeter_acc += si*si + sq*sq;
	}

	int fp = signalbufp;
	signalbuf[fp]   = ssi;
	signalbuf[fp+1] = ssq;
	fp+=2;
	if(fp >= SIGNALBUFLEN) fp = 0;
	signalbufp = fp;
	if (fp == 0 || fp == 171 || fp == 341)
		xSemaphoreGive(fft_sem);

	squelchlpf += (0x100*abs(audioout - audioout_prev) - squelchlpf) / 0x800;
	audioout_prev = audioout;

	if(squelchlpf < 0x2000*p.squelch) {
		audioout = (p.volume2 * audioout / 0x1000) + 100;
		if(audioout < 0) audioout = 0;
		if(audioout > 200) audioout = 200;
	} else {
		audioout = 100;
	}

	smeter_count += nread;
	if(smeter_count >= 0x4000) {
		rs.smeter = smeter_acc / 0x4000;
		smeter_acc = 0;
		smeter_count = 0;
	}
	return audioout;
}
#endif


// State of a biquad filter
struct biquad_state {
	float s1_i, s1_q, s2_i, s2_q;
};

// Coefficients of a biquad filter
struct biquad_coeff {
	float a0, a1, a2, b1, b2;
};

/* Apply a biquad filter to a complex signal with real coefficients,
 * i.e. run it separately for the I and Q parts.
 * Write output back to the same buffer.
 * The algorithm used is called transposed direct form II, or at least
 * I tried to follow how it was drawn at
 * https://www.earlevel.com/main/2003/02/28/biquads/ ...
 * But in some other sources, a and b are named the other way around
 * and the structure looks somehow different.
 * I'm a bit confused now and have to read more on those...
 *
 * The code could possibly be optimized by unrolling a couple of times
 * or by cascading multiple stages in a single loop.
 * We'll need some benchmarks to test such ideas.
 */
void biquad_filter(struct biquad_state *s, const struct biquad_coeff *c, iq_float *buf, unsigned len)
{
	unsigned i;
	const float a0 = c->a0, a1 = c->a1, a2 = c->a2, b1 = c->b1, b2 = c->b2;

	float
	s1_i = s->s1_i,
	s1_q = s->s1_q,
	s2_i = s->s2_i,
	s2_q = s->s2_q;

	for (i = 0; i < len; i++) {
		float in_i, in_q, out_i, out_q;
		in_i = buf[i][0];
		in_q = buf[i][1];
		out_i = s1_i + a0 * in_i;
		out_q = s1_q + a0 * in_q;
		s1_i  = s2_i + a1 * in_i + b1 * out_i;
		s1_q  = s2_q + a1 * in_q + b1 * out_q;
		s2_i  =        a2 * in_i + b2 * out_i;
		s2_q  =        a2 * in_q + b2 * out_q;
		buf[i][0] = out_i;
		buf[i][1] = out_q;
	}

	s->s1_i = s1_i;
	s->s1_q = s1_q;
	s->s1_i = s1_i;
	s->s2_q = s2_q;
}


/* Demodulator state */
struct demod {
	float fm_prev_i, fm_prev_q;
	float bfo_i, bfo_q;
	float bfofreq_i, bfofreq_q;
	float audio_lpf, audio_hpf, audio_po;
	float agc_amp;
	struct biquad_state bq1, bq2;
	enum rig_mode prev_mode;
};


static void demod_reset(struct demod *ds)
{
	ds->fm_prev_i = ds->fm_prev_q = 0;
	ds->audio_lpf = ds->audio_hpf = ds->audio_po = 0;
	ds->agc_amp = 0;
	ds->bfo_i = 1; ds->bfo_q = 0;
	//TODO reset biquads too
}


/* Average 2 samples and store them for waterfall FFT.
 * This is remnant from old code and could be done more
 * efficiently at some point. */
static void demod_store_2_samples(iq_in_t *s)
{
	int fp = signalbufp;
	signalbuf[fp]   = ((int32_t)s[0][1] + s[1][1]) / 2;
	signalbuf[fp+1] = ((int32_t)s[0][0] + s[1][0]) / 2;
	fp+=2;
	if(fp >= SIGNALBUFLEN) fp = 0;
	signalbufp = fp;
	if (fp == 0 || fp == 171 || fp == 341) {
		uint16_t msg = fp;
		if (!xQueueSend(fft_queue, &msg, 0)) {
			//++diag.fft_overflows;
		}
	}
}


/* Store samples for waterfall FFT. Remnant from old code. */
static void demod_store(iq_in_t *in, unsigned len)
{
	unsigned i;
	for (i = 0; i < len; i += 2)
		demod_store_2_samples(&in[i]);
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
 */
/*static inline*/ void demod_fm(struct demod *ds, iq_in_t *in, float *out, unsigned len)
{
	unsigned i;
	float s0i, s0q, s1i, s1q;
	s0i = ds->fm_prev_i;
	s0q = ds->fm_prev_q;
	for (i = 0; i < len; i+=2) {
		float fi, fq, fm;
		s1i = in[i][0];
		s1q = in[i][1];
		fi = s1i * s0i + s1q * s0q;
		fq = s1q * s0i - s1i * s0q;
		fm = fq / (fabsf(fi) + fabsf(fq));

		s0i = in[i+1][0];
		s0q = in[i+1][1];
		fi += s0i * s1i + s0q * s1q;
		fq += s0q * s1i - s0i * s1q;
		fm += fq / (fabsf(fi) + fabsf(fq));

		// Avoid NaN
		if (fm != fm)
			fm = 0;

		out[i/2] = fm;
	}
	ds->fm_prev_i = s0i;
	ds->fm_prev_q = s0q;
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
		ai = fabsf(in[i][0]);
		aq = fabsf(in[i][1]);
		o = (ai >= aq) ? (ai + aq * beta) : (aq + ai * beta);
		ai = fabsf(in[i+1][0]);
		aq = fabsf(in[i+1][1]);
		o += (ai >= aq) ? (ai + aq * beta) : (aq + ai * beta);
		out[i/2] = o;
	}
}


/* Demodulate DSB with input in floating point format.
 *
 * Multiply the signal by a beat-frequency oscillator and take
 * the real part of the result. Decimate it by 2.
 *
 * The oscillator is implemented by "rotating" a complex number on
 * each sample by multiplying it with a value on the unit circle.
 * The value is normalized once per block using formula from
 * https://dspguru.com/dsp/howtos/how-to-create-oscillators-in-software/
 *
 * The previous and next oscillator values alternate between variables
 * osc0 and osc1, and the loop is unrolled for 2 input samples.
 * */
void demod_dsb_f(struct demod *ds, iq_float *in, float *out, unsigned len)
{
	(void)ds;
	unsigned i;
	float osc1i, osc1q;
	float osc0i = ds->bfo_i, osc0q = ds->bfo_q;
	const float oscfi = ds->bfofreq_i, oscfq = ds->bfofreq_q;
	for (i = 0; i < len; i+=2) {
		float o;
		o = osc0i * in[i][0] - osc0q * in[i][1];
		osc1i = osc0i * oscfi - osc0q * oscfq;
		osc1q = osc0i * oscfq + osc0q * oscfi;

		o += osc1i * in[i+1][0] - osc1q * in[i+1][1];
		osc0i = osc1i * oscfi - osc1q * oscfq;
		osc0q = osc1i * oscfq + osc1q * oscfi;

		out[i/2] = o;
	}
	float ms = osc0i * osc0i + osc0q * osc0q;
	ms = (3.0f - ms) * 0.5f;
	ds->bfo_i = ms * osc0i;
	ds->bfo_q = ms * osc0q;
}


static const struct biquad_coeff biquad1_ssb = { };//TODO

/* Demodulate SSB.
 * The Weaver method is used.
 */
void demod_ssb(struct demod *ds, iq_in_t *in, float *out, unsigned len)
{
	iq_float buf[IQ_MAXLEN];
	unsigned i;

	/* Convert the buffer to float.
	 * TODO: also mix with the first oscillator here. */
	for (i = 0; i < len; i++) {
		buf[i][0] = in[i][0];
		buf[i][1] = in[i][1];
	}

	// Calculated for 1500 Hz at 57142 Hz sample rate
	ds->bfofreq_i = 0.98642885096843314f;
	ds->bfofreq_q = 0.16418928703510721f;

	//biquad_filter(&ds->bq1, &biquad1_ssb, buf, len);
	demod_dsb_f(ds, buf, out, len);
}



/* Apply some low-pass filtering to audio for de-emphasis
 * and some high-pass filtering for DC blocking.
 * Store the result in the same buffer.
 *
 * Also calculate the average amplitude which is used for AGC.
 * Average amplitude of differentiated signal is used for squelch.
 */
/*static inline*/ void demod_audio_filter(struct demod *ds, float *buf, unsigned len)
{
	unsigned i;
	const float lpf_a = 0.1f, hpf_a = 0.001f;
	float
	lpf = ds->audio_lpf,
	hpf = ds->audio_hpf,
	po  = ds->audio_po;
	float amp = 0, diff_amp = 0;
	for (i = 0; i < len; i++) {
		lpf += (buf[i] - lpf) * lpf_a;
		hpf += (lpf - hpf) * hpf_a;
		float o = lpf - hpf;
		buf[i] = o;

		amp += fabsf(o);
		diff_amp += fabsf(o - po);
		po = o;
	}
	ds->audio_lpf = lpf;
	ds->audio_hpf = hpf;
	ds->audio_po = po;

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

	demod_store(in, in_len);

	enum rig_mode mode = p.mode;
	/* Reset state after mode change */
	if (mode != demodstate.prev_mode) {
		demod_reset(&demodstate);
		demodstate.prev_mode = mode;
	}

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
	demod_audio_filter(&demodstate, audio, out_len);
	demod_convert_audio(audio, out, out_len, (10.0f * p.volume2) / demodstate.agc_amp);

	return out_len;
}


/* Process one transmit sample */
static inline fm_out_t dsp_process_tx_sample(audio_in_t audioin)
{
	int audioout;
	static int hpf, lpf, agc_level=0;

	// DC block / HPF:
	hpf += (audioin - hpf) / 4;
	audioin -= hpf;
	// lowpass
	lpf += (audioin - lpf) / 2;
	audioin = lpf;

	// Just copied AGC code from RX for now
	int agc_1, agc_diff;
	agc_1 = abs(audioin) * 0x100; // rectify
	agc_diff = agc_1 - agc_level;
	if(agc_diff > 0)
		agc_level += agc_diff/0x100;
	else
		agc_level += agc_diff/0x1000;

	audioout = 32 + 30 * audioin / (agc_level/0x100);
	if(audioout <= 0) audioout = 0;
	if(audioout >= 63) audioout = 63;
	return audioout;
}


/* Function to convert input audio to transmit frequency modulation */
int dsp_fast_tx(audio_in_t *in, fm_out_t *out, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		out[i] = dsp_process_tx_sample(in[i]);
	}
	return len;
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
