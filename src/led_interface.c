#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <led_interface.h>

#define _1ms    1000

bool LED_Run(void *object, LED_Interface *led)
{
	int fd;
    int state;
	char buffer[2];

	char *fifo = "/tmp/fifo_ipc";

	mkfifo(fifo, 0666);

	if (led->Init(object) == false)
        return EXIT_FAILURE;
	
	while (true) 
	{ 
		fd = open(fifo,O_RDONLY); 
		read(fd, buffer, 2);
		close(fd); 
        state = atoi(buffer); 		
		led->Set(object, state);
	} 
	return false;
}
