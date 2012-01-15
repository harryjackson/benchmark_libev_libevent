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
#include "bench.h"

int main(int argc, char **argv)
{
	int c;
	conf.clients = 1;
	conf.pipes = 100;
	conf.events = conf.pipes;
	while ((c = getopt(argc, argv, "c:e:hl:f:")) != -1) {
		switch (c) {
		case 'c':
			conf.clients = atoi(optarg);
			break;
		case 'h':
			usage("", 0);
			break;
		case 'l':
			conf.library = optarg;
			break;
		case 'f':
			conf.pipes = atoi(optarg);
			break;
		case 'e':
			conf.events = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Illegal argument \"%c\"\n", c);
			usage
			    ("Must specify the correct arguments or we blow up\n",
			     0);
			exit(1);
		}
	}
    RUN_LIBEVENT = 0;
    if(strcmp(conf.library, "libevent") == 0) {
        RUN_LIBEVENT = 1;
    }

	if ( (strcmp(conf.library, "libev") != 0) && (strcmp(conf.library, "libevent") != 0))
		usage("Please specify a library to test ie -l [libev|libevent]\n", -1);

	if (conf.events < 1)
		usage("We need at least one event. Accurate measurement will need a lot of events ie millions\n", -1);

	conf.tmpevents = conf.events;

	int sbs = conf.pipes * 2 * sizeof(int);
	bobj.pipes = malloc(sbs);	/* XXX We need to specify a hard limit or at least warn of the need for one */
	assert(bobj.pipes != NULL);

	if (RUN_LIBEVENT == 1) {
        bobj.base = event_base_new();
        create_pipes();
        bench_libev();
    } else {
		int sbw = conf.pipes * 2 * sizeof(struct ev_io);
        bobj.watchers = malloc(sbw);
        assert(bobj.watchers != NULL);
        assert((conf.pipes * 2 * sizeof(int)) / sizeof(int) == sbw / sizeof(struct ev_io));
        bobj.evloop = ev_default_loop(0);
        create_pipes();
        bench_libev();
	}
	return 0;
}

void libevent_read_cb(evutil_socket_t fd, short FLAGS, void *arg)
{
    int rpos = (int) arg;
    return general_read_cb(fd, rpos);
}

void read_cb(struct ev_loop *loop, struct ev_io *w, int flags)
{

    int rpos = (int)w->data;
    return general_read_cb(w->fd, rpos);
}

void general_read_cb(int fd, int rpos) 
{
	res.tot_events++;
	res.tot_read_cb++;
	int rec = 0;
	rec = read(fd, msg, 1);
	if (rec > 0) {
		res.tot_reads++;
		res.tot_bytes += rec;
	} else if (rec < 0) {
		perror("Just called read and got:");
        exit(0);
	} else if(rec == 0) {
        res.tot_read0++;
        exit(0);
    }
    else {
        perror("WTF - ?");
        abort();
    } 
    
    if ( res.tot_events >= conf.events ) {
		breakloop = 1;
        if(RUN_LIBEVENT == 1) {
            event_base_loopbreak(bobj.base); 
        }
        else {
            ev_break(bobj.evloop, EVBREAK_ALL);
        }
        longjmp(quit_buf, 1);
	}
    if( bobj.pipes[rpos] == STDERR_FILENO) {
        perror("bobj.pipes[rpos] == STDERR_FILENO");
        abort();
    }
    int r = write(bobj.pipes[rpos + 1], "d", 1);
	if (r < 0 && errno != EWOULDBLOCK) {
        printf("tried write(fd=%i, d, 1)\n", fd + 1);
		printf("got error %i\n", errno);
		exit(0);
	}
    res.tot_fired++;
    res.tot_writes++;
}

void create_pipes()
{
	int i = 0;
	const int count = 2 * conf.pipes;
	for (i = 0; i < count; i += 2) {
		int rpos = i;	/* Location of read pipe */
		int wpos = i + 1;	/* Location of write pipe */
		if (pipe(&bobj.pipes[rpos]) != 0) {
			printf("Error with call to pipe: %s",
			       strerror(errno));
			exit(0);
		}
		if (fcntl(bobj.pipes[rpos], F_SETFL, O_NONBLOCK) != 0) {
			printf("Error with fcntl call on pipe: %s",
			       strerror(errno));
			exit(0);
		}
		if (fcntl(bobj.pipes[wpos], F_SETFL, O_NONBLOCK) != 0) {
			printf("Error with fcntl call on pipe: %s\n",
			       strerror(errno));
			exit(0);
		}
	    if (RUN_LIBEVENT == 1) {
            struct event *ev;
            ev = event_new(bobj.base, bobj.pipes[rpos], compat_EV_READ("libevent")|EV_PERSIST, libevent_read_cb, (void *)rpos);
            event_add(ev, NULL);
	    }
        else {
            ev_io_init(&bobj.watchers[rpos], read_cb, bobj.pipes[rpos], compat_EV_READ("libev"));
            ev_io_start(bobj.evloop, &bobj.watchers[rpos]);
            bobj.watchers[rpos].data = (void *) rpos;
        }
    }
}


void bench_libev()
{
    int i = 0;
    int sp = conf.pipes / conf.clients;
    if(sp % 2 == 0) {
        sp--;
    }
    if(sp <= 1) {
       sp = 2;
    }
    int cc = 0; /* Client Count */
    int pipe_ar_len = 2 * conf.pipes; 
    for (i = 0, cc = 0 ; (cc < conf.clients); i += sp, cc++) {
        if(i >= pipe_ar_len) {
            i = 0;
        }
        if(bobj.pipes[i + 1] == STDERR_FILENO) {
            abort();
        }
        write(bobj.pipes[i + 1], "c", 1);
        res.tot_fired++;
        res.tot_writes++;
    }
    if ( res.tot_fired != conf.clients) {
        printf("\n    Total Clients active not matching supplied arguments\n");
        printf("\n\n    res.tot_fired=%li  conf.clients=%i sp=%i i=%i\n", res.tot_fired, conf.clients, sp, i);
    }
	struct timer *tim = get_timer();
	start_timer(tim);
    if ( setjmp(quit_buf) == 0 ) {
        if(RUN_LIBEVENT == 1) {
            while (breakloop != 1) {
                event_base_dispatch(bobj.base);
            }
        }
        else {
            while (breakloop != 1) {
                ev_run(bobj.evloop, EVRUN_ONCE | EVRUN_NOWAIT);
            }
        }
    }
    stop_timer(tim);
    double tt = time_taken(tim);
	res.tot_time = tt;
	print_stats();

    free(tim->start);
    free(tim->stop);
    free(tim->timezone);
    free(tim);

}

void print_stats()
{
	double time_per_millev = (res.tot_time / res.tot_events * 1000000);
    fprintf(stdout,"lib=%s,files=%i,clients=%i,tot_events=%li,tot_fired=%li,"
                    "tot_reads=%li,tot_read_cb=%li,tot_writes=%li,tot_r0=%li,tot_bytes=%li,tot_time=%.10f,time_per_millev=%.10f\n",
	       conf.library,
	       conf.pipes,
	       conf.clients,
	       res.tot_events,
           res.tot_fired,
           res.tot_reads,
           res.tot_read_cb,
           res.tot_writes,
           res.tot_read0,
           res.tot_bytes,
           res.tot_time, time_per_millev);
}

void usage(char *msg, int c)
{
	if (c == -1)
		printf("\n\n    Possible cause: %s\n\n", msg);

    printf("\n    command -a 10 -n 1000 -w 1000000 -l libev\n"
           "        -c: Clients.\n"
           "        -e: Number of events to run the test for.\n"
           "        -f: Files. How many pipes are created, note we create twice this many because there is a read and write pipe\n"
           "        -h: Prints this usage.\n"
           "        -l: Library to bench ie [libev|libevent].\n\n");
	exit(EXIT_FAILURE);
}

double time_taken(struct timer *t)
{
	return ((t->stop->tv_sec + t->stop->tv_usec * 1e-6) -
		(t->start->tv_sec + t->start->tv_usec * 1e-6));
}

double stop_timer(struct timer *t)
{
	gettimeofday(t->stop, t->timezone);
	return t->stop->tv_sec + t->stop->tv_usec * 1e-6;
}

double start_timer(struct timer *t)
{
	gettimeofday(t->start, t->timezone);
	return t->start->tv_sec + t->start->tv_usec * 1e-6;
}

struct timer *get_timer()
{
	struct timer *t = malloc(sizeof(struct timer));
	assert(t != NULL);
	int s = sizeof(struct timeval);
	t->start = malloc(s);
	assert(t->start != NULL);
	t->stop = malloc(s);
	assert(t->stop != NULL);
	t->timezone = malloc(sizeof(struct timezone));
	assert(t->timezone != NULL);
	return t;
}

int compat_EV_READ(char *str) 
{
    if(strcmp(str,"libevent") == 0) {
        return 0x02;
    }
    return 0x01;
}

int compat_EVLOOP_NONBLOCK(char *str) 
{
    if(strcmp(str,"libevent") == 0) {
        return 2;
    }
    return 1;
}
