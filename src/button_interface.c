#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <button_interface.h>

#define _1ms    1000

bool Button_Run(void *object, Button_Interface *button)
{
    int fd; 
    char buffer[2] = {0};
    int state = 0;

	// FIFO file path 
	char * fifo = "/tmp/read_fifo"; 

	mkfifo(fifo, 0666);

    if(button->Init(object) == false)
        return EXIT_FAILURE;

    while (true) 
	{ 
        while(true)
        {
            if(!button->Read(object)){
                usleep(_1ms * 100);
                state ^= 0x01;
                break;
            }else{
                usleep( _1ms );
            }
        }   

		fd = open(fifo, O_WRONLY);
        snprintf(buffer, sizeof(buffer), "%d", state);
		write(fd, buffer, strlen(buffer)+1); 
		close(fd);         	
	} 

    return false;
}
