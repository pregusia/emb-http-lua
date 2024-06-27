/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * @file log.c
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


#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// ************************************************************************************
void build_date(char* out, int size) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	strftime(out, size, "%Y-%m-%d %H:%M:%S", t);
}

// ************************************************************************************
void log_info(const char* fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);

    char date_str[128] = { 0 };
    build_date(date_str, sizeof(date_str) - 1);

    fprintf(stderr, "[%s] [INFO] ", date_str);
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");

    va_end(argptr);
}

// ************************************************************************************
void log_error(const char* fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);

    char date_str[128] = { 0 };
    build_date(date_str, sizeof(date_str) - 1);

    fprintf(stderr, "[%s] [ERROR] ", date_str);
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");

    va_end(argptr);
}

