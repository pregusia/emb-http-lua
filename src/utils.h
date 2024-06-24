/*
 * utils.h
 *
 *  Created on: Jun 24, 2024
 *      Author: pregusia
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

uint32_t mem_read_u32(void** ptr);
int32_t mem_read_i32(void** ptr);
char* mem_read_buf(void** ptr, uint32_t* size);

void extract_extension(char* path, char* dest, int32_t dest_len);

#endif /* UTILS_H_ */
