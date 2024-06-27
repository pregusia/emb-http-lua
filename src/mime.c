/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * @file mime.c
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

#include "mime.h"
#include "hashmap.h"
#include "vfs.h"

// ************************************************************************************
uint64_t mime_entry_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct mime_entry* entry = item;
    return hashmap_sip(entry->extension, strlen(entry->extension), seed0, seed1);
}

// ************************************************************************************
int32_t mime_entry_compare(const void *a, const void *b, void *udata) {
    const struct mime_entry* a_entry = a;
    const struct mime_entry* b_entry = b;
    return strcmp(a_entry->extension, b_entry->extension);
}

// ************************************************************************************
int32_t mime_load(struct hashmap** map, struct vfs_buffer* buf) {
	if (!buf) return -1;
	if (!map) return -1;

	*map = hashmap_new(sizeof(struct mime_entry), 0, 0, 0, mime_entry_hash, mime_entry_compare, NULL, NULL);

	char* data = strndup(buf->data, buf->len);
	char* saveptr1 = NULL;
	char* saveptr2 = NULL;
	char* line = strtok_r(data, "\n", &saveptr1);

	while(line) {
		if (line[0] != 0 && line[0] != '#') {
			char* ext = strtok_r(line, "\t ", &saveptr2);
			char* mime = strtok_r(NULL, "\t ", &saveptr2);

			if (ext && mime) {

				struct mime_entry e;
				e.extension = strdup(ext);
				e.mime_type = strdup(mime);

				hashmap_set(*map, &e);
			}

		}
		line = strtok_r(NULL, "\n", &saveptr1);
	}

	free(data);
	return 0;
}

// ************************************************************************************
void mime_free(struct hashmap* map) {
	if (map) {
		hashmap_free(map);
	}
}

// ************************************************************************************
const char* mime_get(struct hashmap* map, const char* ext) {
	if (!map) return NULL;
	if (!ext) return NULL;
	if (ext[0] == 0) return NULL;

	struct mime_entry q;
	q.extension = (char*)ext;

	const struct mime_entry* res = hashmap_get(map, &q);
	if (res) {
		return res->mime_type;
	} else {
		return NULL;
	}
}


