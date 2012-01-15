/*
 * Copyright 2012, Harry Jackson <harry@hjackson.org>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#ifndef BENCH_LIBEV_LIBEVENT
#define BENCH_LIBEV_LIBEVENT
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#define EV_COMPAT3 0
#include <ev.h>
#include <event2/event.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <setjmp.h>


extern char *optarg;
static char msg[10];
static int RUN_LIBEVENT;
static jmp_buf quit_buf;
static int breakloop = 0;

static struct config {
	int clients;
	int pipes;
	int events;
	int tmpevents;
	char *library;
} conf;

static struct bench_obj {
	int *pipes;		/* This holds our filehandles for any libs */
	struct event_base *base;	/* libevent */
	struct ev_io *watchers;	/* libev */
	struct ev_loop *evloop;	/* libev */
} bobj;

static struct bench_results {
	long int tot_fired;
	long int tot_events;
	long int tot_reads;
	long int tot_writes;
	long int tot_write_cb;
	long int tot_bytes;
	long int tot_read0;
	long int tot_read_cb;
	double tot_time;
} res;

struct timer {
	struct timeval *start;
	struct timeval *stop;
	struct timezone *timezone;
};

double start_timer(struct timer *t);
double stop_timer(struct timer *t);
double time_taken(struct timer *t);
int check_pending();
int compat_EVLOOP_NONBLOCK(char *str);
int compat_EV_READ(char *str);
struct timer *get_timer();
void bench_libev();
void bench_libevent();
void create_pipes();
void *fire_read(void *ptr);
void general_read_cb(int fd, int rpos); 
void libevent_read_cb(evutil_socket_t fd, short FLAGS, void *arg);
void malloc_check(void *n, const char *msg);
void print_stats();
void read_cb(struct ev_loop *loop, struct ev_io *w, int flags);
void usage(char *msg, int c);

#endif
