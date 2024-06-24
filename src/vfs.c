/*
 * vfs.c
 *
 *  Created on: Jun 24, 2024
 *      Author: pregusia
 */

#include "vfs.h"
#include "hashmap.h"
#include "utils.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <error.h>
#include <stdio.h>
#include <string.h>

// ************************************************************************************
uint64_t vfs_entry_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct vfs_entry* entry = item;
    return hashmap_sip(entry->vfs_path, strlen(entry->vfs_path), seed0, seed1);
}

// ************************************************************************************
int32_t vfs_entry_compare(const void *a, const void *b, void *udata) {
    const struct vfs_entry* a_entry = a;
    const struct vfs_entry* b_entry = b;
    return strcmp(a_entry->vfs_path, b_entry->vfs_path);
}


// ************************************************************************************
void vfs_buffer_free(struct vfs_buffer* buf) {
	if (buf->freeable) {
		free(buf->data);
		buf->data = NULL;
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
int32_t vfs_buffer_get(struct vfs_entry* e, struct vfs_buffer* buf) {
	if (!e) return -1;
	if (!buf) return -1;

	if (e->fs_path) {
		// in-fs

		int32_t fd = open(e->fs_path, O_RDONLY);
		if (fd < 0) {
			perror("open");
			return -1;
		}

		buf->freeable = 1;
		buf->len = e->size;
		buf->data = malloc(e->size);

		read_full(fd, buf->data, e->size);
		close(fd);
		return 1;
	} else {
		// in-memory
		buf->data = e->mem_data;
		buf->len = e->size;
		buf->freeable = 0;
		return 1;
	}
}

// ************************************************************************************
int32_t vfs_get(struct hashmap* vfs, char* path, struct vfs_buffer* buf) {
	struct vfs_entry q;
	q.vfs_path = path;

	buf->data = NULL;
	buf->freeable = 0;
	buf->len = 0;

	struct vfs_entry* res = hashmap_get(vfs, &q);
	if (res) {
		return vfs_buffer_get(res, buf);
	} else {
		return -1;
	}
}

// ************************************************************************************
int32_t vfs_init_mem(struct hashmap** vfs, void* addr, size_t len) {
	*vfs = hashmap_new(sizeof(struct vfs_entry), 0, 0, 0, vfs_entry_hash, vfs_entry_compare, NULL, NULL);

	void* ptr = addr;
	uint32_t entries_num = mem_read_u32(&ptr);
	for(uint32_t i=0;i<entries_num;++i) {
		uint32_t path_size;
		struct vfs_entry e;

		e.vfs_path = mem_read_buf(ptr, &path_size);
		e.mem_data = mem_read_buf(ptr, &e.size);

		hashmap_set(vfs, &e);
	}

	return 0;
}

// ************************************************************************************
int32_t vfs_fill_file(struct hashmap* vfs, char* vfs_path, char* fs_path) {
	int32_t fd = open(fs_path, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	off_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	struct vfs_entry e;
	e.fs_path = strdup(fs_path);
	e.vfs_path = strdup(vfs_path);
	e.size = size;

	hashmap_set(vfs, &e);

	close(fd);
	return 0;
}

// ************************************************************************************
int32_t vfs_fill_fs(struct hashmap* vfs, char* vfs_path, char* fs_path) {
	struct dirent *de = NULL;
	DIR* d = NULL;
	int32_t res = 0;

	d = opendir(fs_path);
	if (!d) {
		perror("opendir");
		return -1;
	}

	while( (de = readdir(d)) != NULL) {
		if (strcmp(de->d_name,".") == 0) continue;
		if (strcmp(de->d_name,"..") == 0) continue;

		char abs_path_fs[4096] = { 0 };
		char abs_path_vfs[4096] = { 0 };

		sprintf(abs_path_fs, "%s/%s", fs_path, de->d_name);
		sprintf(abs_path_vfs, "%s/%s", vfs_path, de->d_name);

		//printf("%s %s\n", abs_path_fs, abs_path_vfs);

		struct stat s;
		res = stat(abs_path_fs, &s);
		if (res < 0) {
			perror("stat");
			return -1;
		}

		if (S_ISREG(s.st_mode)) {
			res = vfs_fill_file(vfs, abs_path_vfs, abs_path_fs);
			if (res < 0) return res;
		}
		if (S_ISDIR(s.st_mode)) {
			res = vfs_fill_fs(vfs, abs_path_vfs, abs_path_fs);
			if (res < 0) return res;
		}
	}

	closedir(d);
	return 0;
}

// ************************************************************************************
int32_t vfs_init_fs(struct hashmap** vfs, char* fs_path) {
	*vfs = hashmap_new(sizeof(struct vfs_entry), 0, 0, 0, vfs_entry_hash, vfs_entry_compare, NULL, NULL);

	char* vfs_path = "";
	int32_t res = 0;

	res = vfs_fill_fs(*vfs, vfs_path, fs_path);
	if (res < 0) {
		hashmap_free(*vfs);
		*vfs = NULL;
		return -1;
	}

	return 0;
}

// ************************************************************************************
void vfs_free(struct hashmap* vfs) {
	hashmap_free(vfs);
}



