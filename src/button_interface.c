#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <button_interface.h>

#define _1ms    1000

static void wait_press(void *object, Button_Interface *button)
{
    while (true)
    {
        if (!button->Read(object))
        {
            usleep(_1ms * 100);
            break;
        }
        else
        {
            usleep(_1ms);
        }
    }
}

bool Button_Run(void *object, Button_Interface *button)
{
    int fd; 
    char buffer[2] = {0};
    int state = 0;

	// FIFO file path 
	char *fifo = "/tmp/fifo_ipc"; 

	mkfifo(fifo, 0666);

    if(button->Init(object) == false)
        return EXIT_FAILURE;

    while (true) 
	{ 
        wait_press(object, button);

        state ^= 0x01;
		fd = open(fifo, O_WRONLY);
        snprintf(buffer, sizeof(buffer), "%d", state);
		write(fd, buffer, strlen(buffer)+1); 
		close(fd);         	
	} 

    return false;
}
