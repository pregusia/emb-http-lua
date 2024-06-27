/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * @file vfs.h
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

#ifndef VFS_H_
#define VFS_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct vfs_entry {
	char* vfs_path;
	char* fs_path;
	char* mem_data;
	uint32_t size;
};

struct vfs_buffer {
	char* data;
	size_t len;
	int32_t freeable;
};

struct hashmap;


void vfs_buffer_free(struct vfs_buffer* buf);
int32_t vfs_get(struct hashmap* vfs, const char* path, struct vfs_buffer* buf);
int32_t vfs_init_mem(struct hashmap** vfs, void* addr);
int32_t vfs_init_fs(struct hashmap** vfs, const char* path);
void vfs_free(struct hashmap* vfs);

uint32_t vfs_compute_size(struct hashmap* vfs);
int32_t vfs_pack(struct hashmap* vfs, char* dest);

#endif /* VFS_H_ */
