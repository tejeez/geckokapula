# SPDX-License-Identifier: MIT

AUDIO_IN=audio_in.wav

LIBS=-lm
CFLAGS=-Wall -Wextra -DDSP_TEST -I. -I../inc

all: fm_out_audio.wav fm_out_ssb.raw

fm_out_audio.wav: fm_out.raw Makefile
	sox -t raw -e signed-integer -b 16 -c 1 -r 24000 "$<" -t wav "$@" highpass -1 100 gain 60 lowpass -1 300

fm_out.raw: ${AUDIO_IN} dsp_tx_test Makefile
	sox "$<" -t raw -e unsigned-integer -b 16 -c 1 - rate 24000 | ./dsp_tx_test FM "$@" iq_out.raw

fm_out_ssb.raw: ${AUDIO_IN} dsp_tx_test Makefile
	sox "$<" -t raw -e unsigned-integer -b 16 -c 1 - rate 24000 | ./dsp_tx_test USB "$@" iq_out_ssb.raw

dsp_tx_test: dsp_tx_test.c ../src/dsp.c ../inc/*.h Makefile
	${CC} -o "$@" dsp_tx_test.c ../src/dsp.c ${CFLAGS} ${LIBS}
