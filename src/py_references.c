#include <stdio.h>
#include <pthread.h>
#include "./radio.c"

stream_info si;

// Shutdown;
void stream_stop()
{
	if(*si.thr!=0)
	{
		pthread_cancel(*si.thr);destroy();
	};
};

// Get State;
char stream_state()
{
	if(*si.thr!=0)
	{
		return 1;
	};

	return 0;
};

// Song Title Callback;
typedef void (*cb)(const char*);cb cc;

static void title_callback(char *t)
{
	cc(t);
};

// Start;
void stream_start(cb c,char *r)
{
	long int g;

	if(r!=0)
	{
		cc=c;si.url=r;si.title_callback=title_callback;

		pthread_create(&*si.thr,0,stream,(void*)(&si));pthread_join(*si.thr,(void**)&g);
	};
};