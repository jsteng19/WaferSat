/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Modified by varun-iyer for ChibiOS/MCU usage
 *
 *
 */

#include "string.h"
#include "chprintf.h"
#include "sd.h"
#include "hal.h"
#include "log.h"
#include "ff.h"

static struct {
  mutex_t mtx;
  FIL *fp;
  int level;
  int quiet;
} L;


static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

void log_init(void) {
	chMtxObjectInit(&(L.mtx));
#if LOG_MEM
	FRESULT mk_err = 1;
	int test_no = 0;
	// Find unique directory
	while(mk_err && test_no < 100000) {
		chsnprintf(log_dirname, MAX_FILENAME, "test%d", test_no);
		mk_err = f_mkdir(log_dirname);
		test_no++;
	}
	if(mk_err != 0) {
		LOG_ERR_LED();
		// return mk_err;
	}
	
	char log_filename[MAX_FILENAME];
	chsnprintf(log_filename, MAX_FILENAME, "%s/%s", log_dirname, LOG_FILENAME);
 
	FRESULT f_err;
	f_err = f_open(&log_file, log_filename, FA_CREATE_NEW);
	if(f_err) {
		LOG_ERR_LED();
		//TODO error handling system
		// return f_err;
	}
	f_err = f_close(&log_file);
	if(f_err) {
		LOG_ERR_LED();
		// return f_err;
	}
	f_err = f_open(&log_file, log_filename, FA_WRITE);
	if(f_err) {
		LOG_ERR_LED();
		// return f_err;
	}
#endif /* LOG_MEM */
#if LOG_SERIAL
	sdStart(&LOG_SD, &LOG_CONF);
#endif /* LOG_SERIAL */
}

void log_set_udata(void *udata) {
  L.udata = udata;
}


void log_set_fp(FILE *fp) {
  L.fp = fp;
}


void log_set_level(int level) {
  L.level = level;
}


void log_set_quiet(int enable) {
  L.quiet = enable ? 1 : 0;
}


void log_log(int level, const char *file, int line, const char *fmt, ...) {
  if (level < L.level) {
    return;
  }

  /* Acquire lock */
  chMtxLock(&(L.mtx));

  /* Get current time */
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  /* Log to stderr */
  if (!L.quiet) {
    va_list args;
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(
      stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
      buf, level_colors[level], level_names[level], file, line);
#else
    fprintf(stderr, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
#endif
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
  }

  /* Log to file */
  if (L.fp) {
    va_list args;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
    va_start(args, fmt);
    vfprintf(L.fp, fmt, args);
    va_end(args);
    fprintf(L.fp, "\n");
    fflush(L.fp);
  }

  /* Release lock */
  chMtxLock(&(L.mtx));
}
