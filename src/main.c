
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#define HTTPSERVER_IMPL
#include "httpserver.h"

#include "hashmap.h"
#include "vfs.h"
#include "mime.h"
#include "luaapp.h"
#include "utils.h"

static struct hashmap* g_vfs;
static struct hashmap* g_mime;
static struct lua_app* g_lua;
static int32_t g_http_callback;


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
			char* mime = mime_get(g_mime, ext);

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
void print_usage(char* app_name) {
	printf("./%s -p port -d data_dir\n", app_name);
}

// ************************************************************************************
int main(int argc, char** argv) {
	int32_t res = 0;
	struct vfs_buffer buf;
	int32_t port = 0;
	char* data_path = NULL;

    int opt;
    while((opt = getopt(argc, argv, "p:d:")) != -1) {
        switch(opt) {
            case 'p':
            	port = atoi(optarg);
                break;

            case 'd':
            	data_path = strdup(optarg);
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
    	fprintf(stderr, "Missing -d argument\n");
    	return 1;
    }
    if (port <= 0) {
    	fprintf(stderr, "Missing -p argument\n");
    	return 1;
    }

	// vfs init
	res = vfs_init_fs(&g_vfs, data_path);
	if (res < 0) {
		fprintf(stderr, "Cannot create hashmap\n");
		return 1;
	}

	// mime load
	if (1) {
		vfs_get(g_vfs, "/mime.types", &buf);
		if (!buf.data) {
			fprintf(stderr, "Cannot locate /mime.types\n");
			return 1;
		}

		mime_load(&g_mime, &buf);
		vfs_buffer_free(&buf);
	}

	// lua init
	g_lua = luaapp_init(g_vfs);
	if (!g_lua) {
		fprintf(stderr, "Cannot init lua\n");
		return 1;
	}

	// lua load /lib.lua
	if (1) {
		res = luaapp_runfile(g_lua, "/lib.lua");;
		if (res < 0) {
			fprintf(stderr, "Cannot locate /lib.lua\n");
			return 1;
		}
	}

	// lua load /main.lua
	if (1) {
		res = luaapp_runfile(g_lua, "/main.lua");;
		if (res < 0) {
			fprintf(stderr, "Cannot locate /main.lua\n");
			return 1;
		}
	}

	// callback
	if (1) {
		g_http_callback = luaapp_refcallback(g_lua, "__httpHandle");
		if (g_http_callback < 0) {
			fprintf(stderr, "Cannot find __httpHandle function\n");
			return 1;
		}
	}

	struct http_server_s* server = http_server_init(port, handle_request);
	http_server_listen(server);

	return 0;
}
