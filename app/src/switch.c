#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PIN gpio4

int main(int argc, char **argv)
{
	int fd = open("/sys/class/gpio/PIN/direction", O_RDONLY);
	int counter = 0;
	char buffer[2];

	while (1) {
		read(fd, buffer, 2);
		printf("Read from gpio: %d -> %s\n", ++counter, buffer);
	}

	close(fd);
	return 0;
}
