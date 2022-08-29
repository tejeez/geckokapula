/* SPDX-License-Identifier: MIT */

#include <string.h>
#include <stdint.h>

#define DEBUGBUFFER_SIZE 0x200

/* The debug buffer is somewhat compatible with SEGGER RTT,
 * so it can be read with the RTT reader script for OpenOCD at
 * https://gist.github.com/tejeez/ccdf3d03740bdffaf93b992b114aeb51
 *
 * Not tested yet with Segger software.
 *
 * We could also add a function which prints contents of the debug
 * buffer to an UART when called regularly from main loop,
 * allowing both UART and JLink based debug printing.
 */

struct debugbuffer {
	char acID[16];
	uint32_t MaxNumUpBuffers, MaxNumDownBuffers;
	char *sName, *pBuffer;
	uint32_t size, wrOff;
	volatile uint32_t rdOff;
	uint32_t Flags;
	char buffer[DEBUGBUFFER_SIZE];
};

struct debugbuffer debugbuffer;

void debug_init(void)
{
	memset(&debugbuffer, 0, (void*)debugbuffer.buffer - (void*)&debugbuffer);
	debugbuffer.pBuffer = debugbuffer.buffer;
	debugbuffer.wrOff = 0;
	debugbuffer.size = DEBUGBUFFER_SIZE;
	debugbuffer.MaxNumUpBuffers = 1;
	strcpy(debugbuffer.acID, " EGGER RTT");
	debugbuffer.acID[0] = 'S';
}

int _write(int file, const char *ptr, int len)
{
	if (!(file == 1 || file == 2))
		return -1;
	unsigned pos = debugbuffer.wrOff;
	int i;

	if (pos >= DEBUGBUFFER_SIZE)
		pos = 0;

	for (i = 0; i < len; i++) {
		debugbuffer.buffer[pos] = ptr[i];
		++pos;
		if(pos >= DEBUGBUFFER_SIZE)
			pos = 0;
	}
	debugbuffer.wrOff = pos;
	return len;
}
