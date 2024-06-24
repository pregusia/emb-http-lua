/*
 * luaapp.h
 *
 *  Created on: Jun 24, 2024
 *      Author: pregusia
 */

#ifndef LUAAPP_H_
#define LUAAPP_H_

#include <lua.h>

struct lua_app {
	struct lua_State* state;
	struct hashmap* vfs;
};

struct http_request_s;

struct lua_app* luaapp_init(struct hashmap* vfs);
int32_t luaapp_runfile(struct lua_app* app, char* path);
int32_t luaapp_refcallback(struct lua_app* app, char* name);

int32_t luaapp_process_http(struct lua_app* app, int32_t callbackRef, struct http_request_s* req);

#endif /* LUAAPP_H_ */
