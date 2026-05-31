#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include "./src/radio.c"

// Define Variables;
stream_info si;char st=0;

// Print Title;
static void print_title(char *t)
{
	printf("Now Playing: %s.\n",t);st=0;
};

// Free & Exit;
static void shutdown(int s)
{
	if(*si.thr!=0)
	{
		pthread_cancel(*si.thr);destroy();
	};

	exit(0);
};

// Main Loop;
int main(int q,char **r)
{
	if(q<2)
	{
		write(0,"No URL Passed.\n",15);return 1;
	};

	// Install Handlers;
	signal(SIGINT,shutdown);signal(SIGTERM,shutdown);

	// Start Radio Stream (Restart On Error);
	long int g;si.url=*(r+1);si.title_callback=print_title;

	while(st<10)
	{
		pthread_create(&*si.thr,0,stream,(void*)(&si));
		
		pthread_join(*si.thr,(void**)&g);st+=1;
	};

	return 0;
};