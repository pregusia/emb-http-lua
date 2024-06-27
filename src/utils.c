/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * @file utils.c
 * @project emb-http-lua
 * @url https://github.com/pregusia/emb-http-lua
 *
 * MIT License
 *
 * Copyright (c) 2024 pregusia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "utils.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>

// ************************************************************************************
uint32_t mem_read_u32(void** ptr) {
	uint32_t res = *((uint32_t*)*ptr);
	*ptr += 4;
	return res;
}

// ************************************************************************************
int32_t mem_read_i32(void** ptr) {
	int32_t res = *((int32_t*)*ptr);
	*ptr += 4;
	return res;
}

// ************************************************************************************
uint8_t mem_read_u8(void** ptr) {
	uint8_t res = *((uint8_t*)*ptr);
	*ptr += 1;
	return res;
}

// ************************************************************************************
char* mem_read_buf(void** ptr, uint32_t* size) {
	*size = mem_read_u32(ptr);
	char* res = (char*)*ptr;
	*ptr += *size;
	return res;
}

// ************************************************************************************
void mem_write_u32(void** ptr, uint32_t val) {
	*((uint32_t*)*ptr) = val;
	*ptr += 4;
}

// ************************************************************************************
void mem_write_u8(void** ptr, uint8_t val) {
	*((uint8_t*)*ptr) = val;
	*ptr += 1;
}

// ************************************************************************************
void mem_write_buf(void** ptr, const void* buf, uint32_t size) {
	memcpy(*ptr, buf, size);
	*ptr += size;
}

// ************************************************************************************
void extract_extension(const char* path, char* dest, int32_t dest_len) {
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

// ************************************************************************************
int32_t read_full(int32_t fd, char* dest, uint32_t size) {
	uint32_t pos = 0;
	int32_t res = 0;
	char buf[4096] = { 0 };

	while(1) {
		res = read(fd, buf, sizeof(buf));
		if (res == 0) break;
		if (res < 0) {
			perror("read");
			return -1;
		}

		memcpy(dest + pos, buf, res);
		pos += res;
	}

	return pos;
}

// ************************************************************************************
int32_t write_full(int32_t fd, const char* src, uint32_t size) {
	uint32_t pos = 0;
	int32_t res = 0;

	while(pos < size) {
		res = write(fd, src + pos, size - pos);
		if (res == 0) break;
		if (res < 0) {
			perror("write");
			return -1;
		}

		pos += res;
	}

	return pos;
}
