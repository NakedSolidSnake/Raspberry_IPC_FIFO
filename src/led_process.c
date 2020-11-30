// C program to implement one side of FIFO 
// This side reads first, then reads 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <led.h>

#define _1ms    1000

int main() 
{ 
	int fd;
    int state;

	char * myfifo = "/tmp/myfifo";

    LED_t led =
    {
        .gpio.pin = 0,
        .gpio.eMode = eModeOutput
    };

    if (LED_init(&led))
        return EXIT_FAILURE;
	
	mkfifo(myfifo, 0666); 

	char buffer[2];

	while (1) 
	{ 
		
		fd = open(myfifo,O_RDONLY); 
		read(fd, buffer, 2);
		close(fd); 
        state = atoi(buffer); 		

        LED_set(&led, (eState_t)state);
	} 
	return 0; 
}
