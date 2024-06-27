/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * @file luaapp.c
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

#include "luaapp.h"
#include "vfs.h"
#include "httpserver.h"
#include "log.h"

#include <stdio.h>

#include <lualib.h>
#include <lauxlib.h>

// ************************************************************************************
struct lua_app* luaapp_init(struct hashmap* vfs) {
	struct lua_app* res = (struct lua_app*)malloc(sizeof(struct lua_app));

	res->vfs = vfs;
	res->state = luaL_newstate();

	if (!res->state) {
		log_error("[LUA] Unable to create lua engine");
		return NULL;
	}

	luaL_openlibs(res->state);

	return res;
}

// ************************************************************************************
void luaapp_dump_stack(struct lua_app* app) {
    int32_t top = lua_gettop(app->state);

    printf("** STACK DUMP ** (top=%d)\n", top);
    for(int32_t i = 1; i <= top; i++) {
    	printf("[%d]", i);

    	int32_t t = lua_type(app->state, i);
    	switch (t) {
    		case LUA_TSTRING:  /* strings */
    			printf(" [string \"%s\"]", lua_tostring(app->state, i));
    			break;

			case LUA_TBOOLEAN:  /* booleans */
				if (lua_toboolean(app->state, i)) {
					printf(" true");
				} else {
					printf(" false");
				}
				break;

			case LUA_TNUMBER:  /* numbers */
				printf(" %.2f", lua_tonumber(app->state, i));
				break;

			case LUA_TFUNCTION:
				printf(" [function]");
				break;

			default:  /* other values */
				printf(" %s", lua_typename(app->state, t));
				break;
    	}

    	printf("\n");
    }
}

// ************************************************************************************
const char* luaapp_pop_string(struct lua_app* app) {
	if (!app) return NULL;
	lua_pop(app->state,1);
	const char* str = lua_tostring(app->state, 0);
	if (!str || !strlen(str)) return "";
	return str;
}

// ************************************************************************************
int32_t luaapp_pop_i32(struct lua_app* app) {
	if (!app) return 0;
	lua_pop(app->state,1);
	return lua_tonumber(app->state, 0);
}

// ************************************************************************************
int32_t luaapp_runfile(struct lua_app* app, const char* path) {
	if (!app) return -1;
	if (!path) return -1;
	if (!app->vfs) return -1;

	struct vfs_buffer buf;
	int32_t ret = 0;

	vfs_get(app->vfs, path, &buf);

	if (!buf.data) return -1;

	ret = luaL_loadbuffer(app->state, buf.data, buf.len, path);
	if (ret != 0) {
		const char* err = luaapp_pop_string(app);
		log_error("[LUA] Cannot run file %s", path);

		if (err[0] != 0) {
			log_error("[LUA] %s", err);
		}

		vfs_buffer_free(&buf);
		return -1;
	}

	// check that it is loaded as a function
	if (lua_isfunction(app->state, -1) == 0) {
		vfs_buffer_free(&buf);
		return -1;
	}

	// execute it
	ret = lua_pcall(app->state, 0, 0, 0);
	if (ret != 0) {
		const char* err = luaapp_pop_string(app);
		log_error("[LUA] Cannot run file %s", path);

		if (err[0] != 0) {
			log_error("[LUA] %s", err);
		}

		vfs_buffer_free(&buf);
		return -1;
	}

	vfs_buffer_free(&buf);
	return 0;
}

// ************************************************************************************
int32_t luaapp_refcallback(struct lua_app* app, const char* name) {
	if (!app) return LUA_NOREF;
	if (!name) return LUA_NOREF;
	if (name[0] == 0) return LUA_NOREF;

	// get current event function pointer
	lua_getglobal(app->state, name);
	if (lua_isfunction(app->state, -1) == 0) {
		lua_pop(app->state, 1);
		return LUA_NOREF;
	}

    return luaL_ref(app->state, LUA_REGISTRYINDEX);
}

// ************************************************************************************
void luaapp_parse_query(struct lua_app* app, const char* query, int32_t len) {
	char* query_copy = strndup(query, len);
	char* p = NULL;

	while((p = strsep(&query_copy, "&\n"))) {
		char *name = strtok(p, "=");
		char* value = NULL;

		if (name && (value = strtok(NULL, "="))) {
			// TODO: if value is numeric, pushnumber
			// TODO: query params with same name will be replaced

			lua_pushstring(app->state, name);
			lua_pushstring(app->state, value);
			lua_settable(app->state, -3);
		}
	}

	free(query_copy);
}

// ************************************************************************************
void luaapp_push_request(struct lua_app* app, struct http_request_s* request) {
	http_string_t str;

	lua_newtable(app->state);
	lua_getglobal(app->state, "HTTPRequest"); // TODO: optimize string lookup using ref?
	lua_setmetatable(app->state, -2);

	// request.method = xx
	if (1) {
		str = http_request_method(request);
		if (str.buf) {
			lua_pushstring(app->state, "method");
			lua_pushlstring(app->state, str.buf, str.len);
			lua_settable(app->state, -3);
		}
	}

	// request.path + request.query + request.queryParams
	if (1) {
		str = hs_get_token_string(request, HSH_TOK_TARGET);
		if (str.buf) {

			int32_t query_pos = -1;
			for(int32_t i=0;i<str.len;++i) {
				if (str.buf[i] == '?') {
					query_pos = i;
					break;
				}
			}

			if (query_pos > 0) {
				// path
				lua_pushstring(app->state, "path");
				lua_pushlstring(app->state, str.buf, query_pos);
				lua_settable(app->state, -3);

				// queryParams
				lua_pushstring(app->state, "queryParams");
				lua_newtable(app->state);
				luaapp_parse_query(app, str.buf + query_pos + 1, str.len - query_pos - 1);
				lua_settable(app->state, -3);

			} else {
				// path
				lua_pushstring(app->state, "path");
				lua_pushlstring(app->state, str.buf, str.len);
				lua_settable(app->state, -3);

				// queryParams
				lua_pushstring(app->state, "queryParams");
				lua_newtable(app->state);
				lua_settable(app->state, -3);
			}
		}
	}

	// request.headers
	if (1) {
		lua_pushstring(app->state, "headers");
		lua_newtable(app->state);

		// TODO: headers with same name will be replaced

		int32_t iter = 0;
		http_string_t key, val;
		while(http_request_iterate_headers(request, &key, &val, &iter)) {
			lua_pushlstring(app->state, key.buf, key.len);
			lua_pushlstring(app->state, val.buf, val.len);
			lua_settable(app->state, -3);
		}
		lua_settable(app->state, -3);
	}

	// TODO: request content
	// TODO: contentJson?
}

// ************************************************************************************
void luaapp_push_response(struct lua_app* app) {
	lua_newtable(app->state);
	lua_getglobal(app->state, "HTTPResponse"); // TODO: optimize string lookup using ref?
	lua_setmetatable(app->state, -2);

	// response.headers
	if (1) {
		lua_pushstring(app->state, "headers");
		lua_newtable(app->state);
		lua_settable(app->state, -3);
	}

	// response.content
	if (1) {
		lua_pushstring(app->state, "content");
		lua_pushstring(app->state, "");
		lua_settable(app->state, -3);
	}

	// response.code
	if (1) {
		lua_pushstring(app->state, "code");
		lua_pushnumber(app->state, 200);
		lua_settable(app->state, -3);
	}
}

// ************************************************************************************
void luaapp_pop_response(struct lua_app* app, struct http_request_s* request) {
	struct http_response_s* response = http_response_init();
	int32_t has_content_type = 0;

	// code
	if (1) {
		lua_pushstring(app->state, "code");
		lua_gettable(app->state, -2);
		int32_t code = luaapp_pop_i32(app);
		http_response_status(response, code);
	}

	// headers
	if (1) {
		lua_pushstring(app->state, "headers");
		lua_gettable(app->state, -2);

		if (lua_istable(app->state, -1)) {
			int32_t t = lua_gettop(app->state);
			lua_pushnil(app->state);  /* first key */
			while (lua_next(app->state, t) != 0) {
				const char* key = lua_tostring(app->state, -2);
				const char* value = luaapp_pop_string(app);

				if (strcasecmp(key, "content-type") == 0) {
					has_content_type = 1;
				}

				http_response_header(response, key, value);
			}
		}

		lua_pop(app->state, 1);
	}

	if (!has_content_type) {
		http_response_header(response, "Content-Type", "text/plain");
	}

	// content
	if (1) {
		lua_pushstring(app->state, "content");
		lua_gettable(app->state, -2);
		const char* content = luaapp_pop_string(app);
		http_response_body(response, content, strlen(content));
	}

	lua_pop(app->state, 1);

	// TODO: contentJson processing?

	http_respond(request, response);
}


// ************************************************************************************
int32_t luaapp_process_http(struct lua_app* app, int32_t callbackRef, struct http_request_s* request) {
	if (!app) return -1;
	if (!request) return -1;

	// first response
	luaapp_push_response(app);

	// callback
	lua_rawgeti(app->state, LUA_REGISTRYINDEX, callbackRef);

	// request
	luaapp_push_request(app, request);

	// response dup
	lua_pushvalue(app->state, -3);

	// call
	lua_pcall(app->state, 2, 0, 0);

	// read response
	luaapp_pop_response(app, request);

	return 0;
}
