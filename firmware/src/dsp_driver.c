#include "em_timer.h"
#include "em_adc.h"

#include "rail.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "dsp.h"

/* Signal flow in RX
 * -----------------
 * Audio output is a PWM output driven by a timer.
 * I/Q input is read from the RAIL RX FIFO.
 * Audio output sample rate should have a known relationship
 * to the I/Q input sample rate.
 * To ensure the input and output streams keep synchronized, they
 * are driven by the same interrupt handler, which is the
 * RAIL RX FIFO event callback handler.
 * Every time a block of I/Q samples is received, one audio sample
 * is updated to the PWM value. Thus, the I/Q sample rate should
 * be an integer multiple of the audio sample rate.
 *
 * To avoid jitter and aliasing effects, the audio sample rate
 * should be synchronized to the PWM frequency, so that each
 * audio sample affects a constant number of PWM cycles.
 * Some attention should be paid to the effect of interrupt
 * timing jitter in updating the PWM value.
 * (This part is not done yet.)
 *
 * Software interfaces
 * -------------------
 * The callback handler interfaces to the fast DSP task
 * through a ring buffer and an RTOS queue.
 * Each queue item contains the number of samples to process
 * and pointers to input and output buffers.
 * The fast DSP task calls dsp_process_rx with these buffers
 * to do the actual signal processing.
 * The fast DSP task can further defer processing to the
 * slow DSP task by buffering data and sending messages
 * using an RTOS queue.
 */


/* Ratio of I/Q input and audio output sample rates.
 * This determines the number of samples to read
 * at a time from the RAIL FIFO. */
#define RX_SAMPLE_RATIO 2

/* How many samples to process at a time in the DSP task.
 * This is the number of audio samples, so number of I/Q
 * samples is RX_SAMPLE_RATIO times the value. */
#define RX_DSP_BLOCK 16

/* Size of the RX ring buffer.
 * Should be a multiple ot RX_DSP_BLOCK. */
#define RX_BUF_SIZE (RX_DSP_BLOCK*2)


audio_out_t buf_audio_out[RX_BUF_SIZE];
iq_in_t buf_iq_in[RX_BUF_SIZE * RX_SAMPLE_RATIO];
unsigned rx_buf_p = 0;

struct fast_dsp_rx_msg {
	iq_in_t *in;
	audio_out_t *out;
	uint16_t in_len, out_len;
};

QueueHandle_t fast_dsp_rx_q;

void rail_callback(RAIL_Handle_t rail, RAIL_Events_t events)
{
	BaseType_t yield = 0;
	if (events & RAIL_EVENT_RX_FIFO_ALMOST_FULL) {
		unsigned p = rx_buf_p;
		TIMER_CompareBufSet(TIMER0, 0, buf_audio_out[p]);
		RAIL_ReadRxFifo(rail, (uint8_t*)(buf_iq_in + p * RX_SAMPLE_RATIO), sizeof(iq_in_t) * RX_SAMPLE_RATIO);

		if (p % RX_DSP_BLOCK == RX_DSP_BLOCK - 1) {
			// Index of the first sample in the latest received block
			unsigned fp = p - (RX_DSP_BLOCK - 1);

			struct fast_dsp_rx_msg msg = {
				buf_iq_in + fp * RX_SAMPLE_RATIO,
				buf_audio_out + fp,
				RX_DSP_BLOCK * RX_SAMPLE_RATIO,
				RX_DSP_BLOCK
			};
			xQueueSendFromISR(fast_dsp_rx_q, &msg, &yield);
		}

		++p;
		if (p >= RX_BUF_SIZE)
			p = 0;
		rx_buf_p = p;
	}
	portYIELD_FROM_ISR(yield);
}


void fast_dsp_task(void *arg)
{
	(void)arg;
	for (;;) {
		struct fast_dsp_rx_msg msg;
		if (xQueueReceive(fast_dsp_rx_q, &msg, portMAX_DELAY)) {
			dsp_process_rx(msg.in, msg.in_len, msg.out, msg.out_len);
		}
	}
}


/* Create the RTOS objects needed by DSP.
 * Called before starting the scheduler. */
void dsp_rtos_init(void)
{
	fast_dsp_rx_q = xQueueCreate(1, sizeof(struct fast_dsp_rx_msg));
}
