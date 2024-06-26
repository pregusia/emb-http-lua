/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * @file luaapp.h
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

#ifndef LUAAPP_H_
#define LUAAPP_H_

#include <lua.h>

struct lua_app {
	struct lua_State* state;
	struct hashmap* vfs;
};

struct http_request_s;

struct lua_app* luaapp_init(struct hashmap* vfs);
int32_t luaapp_runfile(struct lua_app* app, const char* path);
int32_t luaapp_refcallback(struct lua_app* app, const char* name);

int32_t luaapp_process_http(struct lua_app* app, int32_t callbackRef, struct http_request_s* req);

#endif /* LUAAPP_H_ */
