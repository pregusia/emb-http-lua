/*
 * utils.c
 *
 *  Created on: Jun 24, 2024
 *      Author: pregusia
 */

#include "utils.h"

#include <string.h>

uint32_t mem_read_u32(void** ptr) {
	uint32_t res = *((uint32_t*)*ptr);
	*ptr += 4;
	return res;
}

int32_t mem_read_i32(void** ptr) {
	int32_t res = *((int32_t*)*ptr);
	*ptr += 4;
	return res;
}

char* mem_read_buf(void** ptr, uint32_t* size) {
	*size = mem_read_u32(ptr);
	char* res = (char*)*ptr;
	*ptr += *size;
	return res;
}


void extract_extension(char* path, char* dest, int32_t dest_len) {
	int32_t len = strlen(path);
	int32_t dot_pos = -1;

	for(int32_t i=len;i>=0;--i) {
		if (path[i] == '.') {
			dot_pos = i;
			break;
		}
	}

	if (dot_pos > 0) {
		strncpy(dest, path + dot_pos + 1, dest_len);
	} else {
		dest[0] = 0;
	}
}


