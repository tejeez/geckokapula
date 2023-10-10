# SPDX-License-Identifier: MIT

AUDIO_IN=audio_in.wav

CFLAGS=-Wall -Wextra -DDSP_TEST -I. -I../inc

test_dsp_tx: ${AUDIO_IN} dsp_tx_test Makefile
	sox "${AUDIO_IN}" -t raw -e unsigned-integer -b 16 -c 1 - rate 24000 | ./dsp_tx_test fm_out.raw

dsp_tx_test: dsp_tx_test.c ../src/dsp.c ../inc/dsp.h Makefile
	${CC} dsp_tx_test.c ../src/dsp.c -o "$@" ${LIBS} ${CFLAGS} -lm
