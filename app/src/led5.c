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

#define LED_PATH "/sys/class/gpio/gpio18/value"
#define BUTTON_PATH "/sys/class/gpio/gpio17/value"
#define HZ 5

static volatile int should_blink = 1;
static volatile int led_fd;

void *blink_led(void *data);
void *check_button(void *data);
void sigfunc(int sig);

int main(int argc, char **argv)
{
	int *retval;
	pthread_t threads[2];
		
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
	
	on = 0;
	while(1)
	{
		clock_nanosleep(CLOCK_REALTIME, 0, &wait, NULL);
		if(!should_blink)
			continue;

		led_fd = open(LED_PATH, O_WRONLY);
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
		close(led_fd);
	}
	return 0;
}

void *check_button(void *data)
{
	int fd, pressed;
	char buffer[2];
	struct timespec wait = {
		.tv_sec = 0,
		.tv_nsec = 10000000
	};

	pressed = 0;

	while (1) {

		fd = open(BUTTON_PATH, O_RDONLY);
		if(fd < 0)
		{
			perror("could not open BUTTON-file!");
			continue;
		}

		read(fd, buffer, 1);

		if(strcmp(buffer, "0") == 0 && !pressed)
		{
			pressed = 1;
			printf("Button pressed!\n");
		}
		if(strcmp(buffer, "1") == 0 && pressed)
		{
			pressed = 0;
			printf("Button released!\n");
			should_blink = !should_blink;
		}

		close(fd);
		clock_nanosleep(CLOCK_REALTIME, 0, &wait, NULL);
	}
	return 0;
}

void sigfunc(int sig)
{
	int fd;

	fd = open(LED_PATH, O_WRONLY);
	if(fd < 0)
	{
		perror("could not open");
		fd = led_fd;
	}

	write(fd, "1", 2);

	exit(0);
}
