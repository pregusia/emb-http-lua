/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * @file utils.h
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

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

uint32_t mem_read_u32(void** ptr);
int32_t mem_read_i32(void** ptr);
uint8_t mem_read_u8(void** ptr);
char* mem_read_buf(void** ptr, uint32_t* size);

void mem_write_u8(void** ptr, uint8_t val);
void mem_write_u32(void** ptr, uint32_t val);
void mem_write_buf(void** ptr, const void* buf, uint32_t size);

int32_t read_full(int32_t fd, char* dest, uint32_t size);
int32_t write_full(int32_t fd, const char* src, uint32_t size);

void extract_extension(const char* path, char* dest, int32_t dest_len);

#endif /* UTILS_H_ */
