/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * @file main.c
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


#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h>
#include <elf.h>
#include <libelf.h>
#include <gelf.h>

#define HTTPSERVER_IMPL
#include "httpserver.h"

#include "hashmap.h"
#include "vfs.h"
#include "mime.h"
#include "luaapp.h"
#include "utils.h"
#include "log.h"

#define VFS_EMBED_BASE_ADDR 0x80000000

static struct hashmap* g_vfs;
static struct hashmap* g_mime;
static struct lua_app* g_lua;
static int32_t g_http_callback;

static volatile char* g_emb_mark = "--$$NO_EMB$$--";

// ************************************************************************************
int32_t is_embedded() {
	if (g_emb_mark[4] == 'N' && g_emb_mark[5] == 'O') {
		return 0;
	} else {
		return 1;
	}
}

// ************************************************************************************
void handle_request(struct http_request_s* request) {
	char* query_path = NULL;

	// extract path from query
	if (1) {
		http_string_t str = hs_get_token_string(request, HSH_TOK_TARGET);
		if (str.buf) {
			int32_t ql = str.len;
			for(int32_t i=0;i<str.len;++i) {
				if (str.buf[i] == '?') {
					ql = i;
					break;
				}
			}
			query_path = strndup(str.buf, ql);
		}
	}

	// check file from vfs
	if (query_path) {
		struct vfs_buffer buf;
		vfs_get(g_vfs, query_path, &buf);
		if (buf.data) {
			char ext[32] = { 0 };
			extract_extension(query_path, ext, 32);
			const char* mime = mime_get(g_mime, ext);

			struct http_response_s* response = http_response_init();
			http_response_status(response, 200);
			if (mime) {
				http_response_header(response, "Content-Type", mime);
			} else {
				http_response_header(response, "Content-Type", "application/octet-stream");
			}
			http_response_body(response, buf.data, buf.len);
			http_respond(request, response);
			vfs_buffer_free(&buf);
			free(query_path);
			return;
		}
	}

	luaapp_process_http(g_lua, g_http_callback, request);
	free(query_path);
}

// ************************************************************************************
uint64_t align(uint64_t val) {
	uint64_t r = val % 4096;
	return val + (4096 - r);
}

// ************************************************************************************
int self_pack(char* dest) {
	int32_t fd = 0;
	uint64_t input_size = 0;
	char* input_buffer = NULL;
	uint64_t output_size = 0;
	char* output_buffer = NULL;

	uint64_t vfs_size = align(vfs_compute_size(g_vfs));
	char* vfs_buffer = malloc(vfs_size);
	uint64_t move_size = 4096;

	vfs_pack(g_vfs, vfs_buffer);

	// loading
	if (1) {
		fd = open("/proc/self/exe", O_RDONLY);
		if (fd < 0) {
			perror("open");
			return 1;
		}

		input_size = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);

		input_buffer = malloc(input_size);
		read_full(fd, input_buffer, input_size);
		close(fd);

		output_size = align(input_size) + vfs_size + move_size;
		output_buffer = malloc(output_size);
	}

	// changing
	if (1) {
		uint64_t vfs_offset = align(input_size + move_size);

		Elf64_Ehdr* hdr_in = (Elf64_Ehdr*)input_buffer;
		Elf64_Ehdr* hdr_out = (Elf64_Ehdr*)output_buffer;

		// move everything by $move_size
		memcpy(output_buffer + move_size, input_buffer, input_size);

		// restore orginal header after moving
		*hdr_out = *hdr_in;

		// fix segments offsets
		for(int32_t i=0;i<hdr_in->e_phnum;++i) {
			Elf64_Phdr* in = (Elf64_Phdr*)&input_buffer[hdr_in->e_phoff + i * hdr_in->e_phentsize];
			Elf64_Phdr* out = (Elf64_Phdr*)&output_buffer[hdr_out->e_phoff + i * hdr_out->e_phentsize];

			*out = *in;
			if (out->p_filesz > 0) {
				out->p_offset += move_size;
			}
		}

		// fix sessions offsets
		hdr_out->e_shoff += move_size;
		for(int32_t i=0;i<hdr_out->e_shnum;++i) {
			Elf64_Shdr* sec = (Elf64_Shdr*)&output_buffer[hdr_out->e_shoff + i * hdr_out->e_shentsize];

			if (sec->sh_offset > 0 && sec->sh_type != SHT_NOBITS) {
				sec->sh_offset += move_size;
			}
		}

		// add new segment
		if (1) {
			hdr_out->e_phnum += 1;

			Elf64_Phdr* phdr = (Elf64_Phdr*)&output_buffer[hdr_out->e_phoff + hdr_out->e_phentsize * (hdr_out->e_phnum - 1)];
			phdr->p_align = 0x1000;
			phdr->p_filesz = vfs_size;
			phdr->p_flags = PF_R;
			phdr->p_memsz = vfs_size;
			phdr->p_offset = vfs_offset;
			phdr->p_paddr = VFS_EMBED_BASE_ADDR;
			phdr->p_type = PT_LOAD;
			phdr->p_vaddr = VFS_EMBED_BASE_ADDR;
		}

		// change NO_EMB flag
		for(uint32_t i=0;i<output_size;++i) {
			if (strncmp(output_buffer + i,"--$$NO_EMB$$--", 14) == 0) {
				memcpy(output_buffer + i, "--$$___EMB$$--", 14);
			}
		}

		// save vfs data
		memcpy(output_buffer + vfs_offset, vfs_buffer, vfs_size);
	}

	// writing
	if (1) {
		fd = open(dest, O_WRONLY | O_CREAT | O_EXCL, 0755);
		if (fd < 0) {
			perror("open");
			return 1;
		}

		write_full(fd, output_buffer, output_size);
		close(fd);
	}

	free(input_buffer);
	free(output_buffer);
	free(vfs_buffer);
	return 0;
}

// ************************************************************************************
void print_usage(char* app_name) {
	if (is_embedded()) {
		printf("Usage:\n");
		printf("  ./%s -p port\n", app_name);
	} else {
		printf("Usage:\n");
		printf("  Run webserver from data_dir\n");
		printf("    %s -d data_dir -p port \n", app_name);
		printf("\n");
		printf("  Self-pack datadir and executable to output_path\n");
		printf("    %s -d data_dir -o output_path\n", app_name);
	}
}

// ************************************************************************************
int app_run(int port) {
	struct vfs_buffer buf;
	int32_t res = 0;

    if (port <= 0) {
    	log_error("Missing -p argument");
    	return 1;
    }

	// mime load
	if (1) {
		vfs_get(g_vfs, "/mime.types", &buf);
		if (!buf.data) {
			log_error("[VFS] Cannot locate /mime.types");
			return 1;
		}

		mime_load(&g_mime, &buf);
		vfs_buffer_free(&buf);
	}

	// lua init
	g_lua = luaapp_init(g_vfs);
	if (!g_lua) {
		log_error("[LUA] Cannot init lua");
		return 1;
	}

	// lua load /lib.lua
	if (1) {
		res = luaapp_runfile(g_lua, "/lib.lua");;
		if (res < 0) {
			log_error("[VFS] Cannot run /lib.lua");
			return 1;
		}
	}

	// lua load /main.lua
	if (1) {
		res = luaapp_runfile(g_lua, "/main.lua");;
		if (res < 0) {
			log_error("Cannot run /main.lua");
			return 1;
		}
	}

	// callback
	if (1) {
		g_http_callback = luaapp_refcallback(g_lua, "__httpHandle");
		if (g_http_callback < 0) {
			log_error("[LUA] Cannot ref __httpHandle function");
			return 1;
		}
	}

	struct http_server_s* server = http_server_init(port, handle_request);
	log_info("[NET] Started HTTP server on port %d", port);
	http_server_listen(server);

	return 0;
}

// ************************************************************************************
int main_standalone(int argc, char** argv) {
	int32_t res = 0;
	int32_t port = 0;
	char* data_path = NULL;
	char* pack_dest = NULL;

    int opt;
    while((opt = getopt(argc, argv, "p:d:o:h")) != -1) {
        switch(opt) {
            case 'p':
            	port = atoi(optarg);
                break;

            case 'd':
            	data_path = strdup(optarg);
                break;

            case 'o':
            	pack_dest = strdup(optarg);
            	break;

            case 'h':
            	print_usage(argv[0]);
            	return 0;

            default:    /* '?' */
                print_usage(argv[0]);
                return 1;
        }
    }

    if (!data_path) {
    	log_error("Missing -d argument");
    	print_usage(argv[0]);
    	return 1;
    }

	// vfs init
	res = vfs_init_fs(&g_vfs, data_path);
	if (res < 0) {
		log_error("[VFS] Cannot init VFS from data dir %s", data_path);
		return 1;
	}

    if (pack_dest) {
    	return self_pack(pack_dest);
    } else {
    	return app_run(port);
    }
}

// ************************************************************************************
int main_embedded(int argc, char** argv) {
	int32_t res = 0;
	int32_t port = 0;

    int opt;
    while((opt = getopt(argc, argv, "p:h")) != -1) {
        switch(opt) {
            case 'p':
            	port = atoi(optarg);
                break;

            case 'h':
            	print_usage(argv[0]);
            	return 0;

            default:    /* '?' */
                print_usage(argv[0]);
                return 1;
        }
    }


    // vfs init
    res = vfs_init_mem(&g_vfs, (void*)VFS_EMBED_BASE_ADDR);
	if (res < 0) {
		log_error("[VFS] Cannot init VFS from memory");
		return 1;
	}

   	return app_run(port);
}

// ************************************************************************************
int main(int argc, char** argv) {
	if (is_embedded()) {
		return main_embedded(argc, argv);
	} else {
		return main_standalone(argc, argv);
	}
}
