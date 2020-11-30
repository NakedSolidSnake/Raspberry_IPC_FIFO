// C program to implement one side of FIFO 
// This side writes first, then reads 
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <button.h>

#define _1ms    1000

static Button_t button = {
        .gpio.pin = 7,
        .gpio.eMode = eModeInput,
        .ePullMode = ePullModePullUp,
        .eIntEdge = eIntEdgeFalling,
        .cb = NULL
    };

int main() 
{ 
	int fd; 
    char buffer[2] = {0};
    int state = 0;

	// FIFO file path 
	char * myfifo = "/tmp/myfifo"; 

	// Creating the named file(FIFO) 
	// mkfifo(<pathname>, <permission>) 
	mkfifo(myfifo, 0666);

    if(Button_init(&button))
        return EXIT_FAILURE;
	
	while (1) 
	{ 
		// Open FIFO for write only 

        while(1)
        {
            if(!Button_read(&button)){
                usleep(_1ms * 40);
                while(!Button_read(&button));
                usleep(_1ms * 40);
                state ^= 0x01;
                break;
            }else{
                usleep( _1ms );
            }
        }   

		fd = open(myfifo, O_WRONLY);
        snprintf(buffer, sizeof(buffer), "%d", state);
		write(fd, buffer, strlen(buffer)+1); 
		close(fd);         	
	} 
	return 0; 
} 
