/* SPDX-License-Identifier: MIT */

#include <string.h>
#include <stdint.h>

/* The debug buffer is somewhat compatible with SEGGER RTT,
 * so it can be read with at least some RTT readers.
 * Not tested yet with Segger software.
 */

#define DEBUGBUFFER_SIZE 0x200
#define DOWNBUFFER_SIZE 4

struct debugbuffer {
	char acID[16];
	uint32_t MaxNumUpBuffers, MaxNumDownBuffers;
	// Up buffer info (MCU to debugger)
	char *sName, *pBuffer;
	uint32_t size, wrOff;
	volatile uint32_t rdOff;
	uint32_t Flags;
	// Dummy down buffer info
	// because some RTT readers do not work without one
	char *sNameUp, *pBufferUp;
	uint32_t sizeUp, wrOffUp;
	volatile uint32_t rdOffUp;
	uint32_t FlagsUp;
	// Buffers
	char buffer[DEBUGBUFFER_SIZE];
	char bufferUp[DOWNBUFFER_SIZE];
};

/* Cortex-Debug RTT reader looks for a symbol named _SEGGER_RTT
 * so use that name for the struct. */
struct debugbuffer _SEGGER_RTT;

void debug_init(void)
{
	struct debugbuffer *b = &_SEGGER_RTT;
	memset(b, 0, (void*)b->buffer - (void*)b);
	b->sName = "Debug print";
	b->pBuffer = b->buffer;
	b->size = DEBUGBUFFER_SIZE;
	b->sNameUp = b->sName;
	b->pBufferUp = b->bufferUp;
	b->sizeUp = DOWNBUFFER_SIZE;
	b->MaxNumUpBuffers = 1;
	b->MaxNumDownBuffers = 1;
	strcpy(b->acID, " EGGER RTT");
	b->acID[0] = 'S';
}

int _write(int file, const char *ptr, int len)
{
	if (!(file == 1 || file == 2))
		return -1;
	struct debugbuffer *b = &_SEGGER_RTT;
	unsigned pos = b->wrOff;
	int i;

	if (pos >= DEBUGBUFFER_SIZE)
		pos = 0;

	for (i = 0; i < len; i++) {
		b->buffer[pos] = ptr[i];
		++pos;
		if(pos >= DEBUGBUFFER_SIZE)
			pos = 0;
	}
	b->wrOff = pos;
	return len;
}
