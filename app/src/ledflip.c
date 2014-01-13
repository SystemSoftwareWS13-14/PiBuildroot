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

#define DEV_PATH "/dev/mygpio"
#define LED_PATH DEV_PATH
#define BUTTON_PATH DEV_PATH
#define HZ 5
#define HZ_STEP 10

static volatile int led_fd;
static volatile int btn_fd;
static volatile int hz;

void *blink_led(void *data);
void *check_button(void *data);
void sigfunc(int sig);

int main(int argc, char **argv)
{
	int *retval;
	pthread_t threads[2];
		
	led_fd = open(LED_PATH, O_WRONLY);
	signal(SIGINT, sigfunc);

	if(pthread_create(threads, NULL, blink_led, NULL))
		return -1;
	if(pthread_create(&threads[1], NULL, check_button, NULL))
		return -1;

	pthread_join(threads[0], (void**) &retval);
	pthread_join(threads[1], (void**) &retval);

	return 0;
}

void *blink_led(void *data)
{
	int on;
	char buf[2];
	struct timespec wait = {
		.tv_sec = 0,
		.tv_nsec = 1000000000/HZ
	};

	hz = HZ;	
	on = 0;
	while(1)
	{
		wait.tv_nsec = 1000000000/hz;

		clock_nanosleep(CLOCK_REALTIME, 0, &wait, NULL);

		sprintf(buf, "%d", on);
		write(led_fd, buf, 2);

		if(on)
			on = 0;
		else
			on = 1;
	}
	return 0;
}

void *check_button(void *data)
{
	int pressed;
	char buffer[2];
	struct timespec wait = {
		.tv_sec = 0,
		.tv_nsec = 1000000
	};

	pressed = 0;

	while (1) {
		btn_fd = open(BUTTON_PATH, O_RDONLY);
		read(btn_fd, buffer, 2);
		close(btn_fd);

		if(strcmp(buffer, "0") == 0 && !pressed)
		{
			pressed = 1;
			printf("Pressed\n");
		}
		else if(strcmp(buffer, "1") == 0 && pressed)
		{
			pressed = 0;
			hz += HZ_STEP;
			printf("Released\n");
		}

		clock_nanosleep(CLOCK_REALTIME, 0, &wait, NULL);
	}
	return 0;
}

void sigfunc(int sig)
{
	write(led_fd, "1", 2);
	close(led_fd);

	exit(0);
}
