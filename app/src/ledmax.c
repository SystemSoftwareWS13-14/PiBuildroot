#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>

#define LED_PATH "/dev/mygpio"

#define HZ 20
#define MAX_HZ 500
#define LED_MAX_COUNT 50

static volatile int led_fd;
static volatile int hz;

void *blink_led(void *data);
void sigfunc(int sig);

int main(int argc, char **argv)
{
	int *retval;
	pthread_t threads[1];
		
	signal(SIGINT, sigfunc);

	led_fd = open(LED_PATH, O_WRONLY);

	if(pthread_create(threads, NULL, blink_led, NULL))
		return -1;

	pthread_join(threads[0], (void**) &retval);

	return 0;
}

void *blink_led(void *data)
{
	int on, i;
	char buf[2];
	struct timespec wait = {
		.tv_sec = 0,
		.tv_nsec = 1000000000/HZ
	};
	
	on = 0;
	hz = HZ;
	
	while(1)
	{
		wait.tv_nsec = 1000000000/hz;
		for(i = 0; i < LED_MAX_COUNT; i++)
		{
			clock_nanosleep(CLOCK_REALTIME, 0, &wait, NULL);
		
			if(led_fd < 0)
			{
				perror("could not open LED-file");
				continue;
			}

			sprintf(buf, "%d", on);
			write(led_fd, buf, 2);

			if(on)
				on = 0;
			else
				on = 1;
		}
		hz += 5;
		
		if(hz > MAX_HZ)
			hz = HZ;
	}

	return 0;
}

void sigfunc(int sig)
{
	write(led_fd, "1", 2);
	close(led_fd);
	printf("\nMeasured frequency: %dHz.\n", hz);
	exit(0);
}
