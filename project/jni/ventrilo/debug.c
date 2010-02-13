#include <stdio.h>
#include <string.h>
#include <android/log.h>
#include "debug.h"

void print(uint8_t *data, uint32_t size, char *tag) {
	uint8_t buffer[64] = { 0 };
	uint32_t i;
	for(i = 0; i < size; i++) {
		sprintf(buffer, "%s%.2X ", buffer, data[i]);
		if(i && !(i % 16)) {
			__android_log_write(ANDROID_LOG_VERBOSE, tag, buffer);
			memset(buffer, 0, sizeof(buffer));
		}
	}
	__android_log_write(ANDROID_LOG_VERBOSE, tag, buffer);
}
