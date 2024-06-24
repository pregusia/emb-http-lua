/*
 * vfs.h
 *
 *  Created on: Jun 24, 2024
 *      Author: pregusia
 */

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
int32_t vfs_get(struct hashmap* vfs, char* path, struct vfs_buffer* buf);
int32_t vfs_init_mem(struct hashmap** vfs, void* addr, size_t len);
int32_t vfs_init_fs(struct hashmap** vfs, char* path);
void vfs_free(struct hashmap* vfs);


#endif /* VFS_H_ */
