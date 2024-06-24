/*
 * mime.h
 *
 *  Created on: Jun 24, 2024
 *      Author: pregusia
 */

#ifndef MIME_H_
#define MIME_H_

#include <stdint.h>

struct vfs_buffer;
struct hashmap;

struct mime_entry {
	char* extension;
	char* mime_type;
};

int32_t mime_load(struct hashmap** map, struct vfs_buffer* buf);
void mime_free(struct hashmap* map);
char* mime_get(struct hashmap* map, char* ext);


#endif /* MIME_H_ */
