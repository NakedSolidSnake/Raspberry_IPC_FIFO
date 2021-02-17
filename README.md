<p align="center">
  <img src="https://media3.giphy.com/media/l0HlLRLbo6I1ZksqQ/source.gif"/>
</p>

# FIFO
## Tópicos
* [Introdução](#Introdução) 
* [Criando FIFO](#criando-fifo)
* [Implementação](#Implementação)
* [launch_processes.c](#launch_processesc)
* [button_process.c](#button_processc)
* [led_process.c](#led_processc)
* [Compilando](#compilando)
* [Executando](#executando)
* [Matando os processos](#matando-os-processos)
* [Conclusão](#Conclusão)
* [Referências](#Referências)

## Introdução
FIFO é um IPC similar à [Pipes](https://github.com/NakedSolidSnake/Raspberry_IPC_Pipe), mas diferente de [Pipes](https://github.com/NakedSolidSnake/Raspberry_IPC_Pipe) possui um nome no filesystem, ou seja, é um arquivo como qualquer outro presente no filesystem, e pode ser manipulado com as funções de _open_, _read_, _write_ e _close_. Uma caracteristica desse IPC é que qualquer processo interessado nos dados que trafegam nesse IPC pode consumir esses dados.

## Criando FIFO
Para criar uma FIFO usa-se a _system call_ 
```c
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode);
```
Onde _pathname_ representa o caminho do arquivo a ser criado, e _mode_ representa as permissões do arquivo, e retorna 0 em caso de sucesso e -1 em caso de falha na criação do arquivo.

## Implementação
Como descrito em [Pipes](https://github.com/NakedSolidSnake/Raspberry_IPC_Pipe) será apresentando uma implementação de uma aplicação cliente-servidor.
## launch_processes.c
```c
int pid_button, pid_led;
int button_status, led_status;
```
```c
pid_button = fork();

if(pid_button == 0)
{
    //start button process
    char *args[] = {"./button_process", NULL};
    button_status = execvp(args[0], args);
    printf("Error to start button process, status = %d\n", button_status);
    abort();
}   
    
```
```c
pid_led = fork();

if(pid_led == 0)
{
    //Start led process
    char *args[] = {"./led_process", NULL};
    led_status = execvp(args[0], args);
    printf("Error to start led process, status = %d\n", led_status);
    abort();
}
```
## button_process.c
```c
static Button_t button = {
    .gpio.pin = 7,
    .gpio.eMode = eModeInput,
    .ePullMode = ePullModePullUp,
    .eIntEdge = eIntEdgeFalling,
    .cb = NULL
};
```

```c
int fd; 
char buffer[2] = {0};
int state = 0;
```
```c
char * myfifo = "/tmp/myfifo"; 
mkfifo(myfifo, 0666);
```
```c
if(Button_init(&button))
    return EXIT_FAILURE;
```
```c
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
```
```c
fd = open(myfifo, O_WRONLY);
snprintf(buffer, sizeof(buffer), "%d", state);
write(fd, buffer, strlen(buffer)+1); 
close(fd);         	
```
## led_process.c
```c
int fd;
int state;
char buffer[2];

char * myfifo = "/tmp/myfifo";

LED_t led =
{
    .gpio.pin = 0,
    .gpio.eMode = eModeOutput
};
```
```c
if (LED_init(&led))
    return EXIT_FAILURE;
```
```c	
mkfifo(myfifo, 0666); 
```
```c
while (1) 
{ 

  fd = open(myfifo,O_RDONLY); 
  read(fd, buffer, 2);
  close(fd); 
      state = atoi(buffer); 		

      LED_set(&led, (eState_t)state);
} 
```
## Compilando
## Executando
## Matando os processos
## Conclusão
## Referências
