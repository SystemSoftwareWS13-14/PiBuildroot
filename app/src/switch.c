#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define PIN_FILE "/sys/class/gpio/gpio17/value"

int main(int argc, char **argv)
{
	int fd;
	int pressed = 0;
	int counter = 0;
	char buffer[1];

	printf("Opening %s...\n", PIN_FILE);

	while (1) {
		fd = open(PIN_FILE, O_RDONLY);
		if(fd < 0)
		{
			perror("Could not open gpio-file!");
			return 1;
		}

		read(fd, buffer, 1);
		if(strcmp(buffer, "0") == 0 && !pressed)
		{
			pressed = 1;
			++counter;
			printf("Button pressed! Count: %d\n", counter);
		}
		if(strcmp(buffer, "1") == 0 && pressed)
		{
			pressed = 0;
			printf("Button released!\n");
		}

		close(fd);
	}

	return 0;
}
