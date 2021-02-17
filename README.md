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
Declaramos variáveis para armazenar o PID dos processos _button_process_ e _led_process_, e variáveis para armazenar o estado da chamada feita pelo _exec_
```c
int pid_button, pid_led;
int button_status, led_status;
```
Aqui clonamos o processo usando _fork_ e chamamos o _exec_ com o _button_process_ como parâmetro para que o clone mude o seu contexto para _button_process_
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
Aqui clonamos o processo usando _fork_ e chamamos o _exec_ com o _led_process_ como parâmetro para que o clone mude o seu contexto para _led_process_
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
Configuramos o descritor do botão como entrada, com _pullup_ habilitado 
```c
static Button_t button = {
    .gpio.pin = 7,
    .gpio.eMode = eModeInput,
    .ePullMode = ePullModePullUp,
    .eIntEdge = eIntEdgeFalling,
    .cb = NULL
};
```
Aqui criamos variáveis para receber o _handle_ do arquivo, um buffer para escrevermos o estado do botão, que servirá como base para a alteração do estado do LED, e uma para manter
```c
int fd; 
char buffer[2] = {0};
int state = 0;
```
Definimos o caminho onde o arquivo será criado, e por fim criamos o arquivo _FIFO_ com permissão de escrita e leitura para o usuário, grupo e outros
```c
char * myfifo = "/tmp/myfifo"; 
mkfifo(myfifo, 0666);
```
```c
if(Button_init(&button))
    return EXIT_FAILURE;
```
Nesse loop é aguardado o pressionamento do botão para que a alteração do estado seja efetivado e o loop interrompido
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
Nesse trecho, abrimos o arquivo da FIFO e adquirimos o handle do arquivo, formatamos o buffer com o valor presente na variável _state_ e escrevemos na FIFO, e fechamos o handle
```c
fd = open(myfifo, O_WRONLY);
snprintf(buffer, sizeof(buffer), "%d", state);
write(fd, buffer, strlen(buffer)+1); 
close(fd);         	
```
## led_process.c
Aqui criamos variáveis para receber o handle da FIFO, um buffer que será usado para ler o valor contido na FIFO, uma variável para guardar o estado, uma string com o caminho da FIFO e por fim o descritor de LED
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
Inicializamos o LED com o descritor previamente configurado
```c
if (LED_init(&led))
    return EXIT_FAILURE;
```
Garantimos que a FIFO foi criada
```c	
mkfifo(myfifo, 0666); 
```
Aqui a aplicação fica realizando _pooling_ lendo o conteúdo do arquivo e aplicando em LED
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
OBS: Para compilar e testar o projeto é necessário instalar a biblioteca de hardware necessária para resolver as dependências de configuração de GPIO da Raspberry Pi.
Para compilar execute os comandos abaixo
```bash
$ git clone https://github.com/NakedSolidSnake/Raspberry_IPC_FIFO
$ cd Raspberry_IPC_FIFO
$ mkdir build && cd build
$ cmake ..
$ make
```
## Executando
Para executar a aplicação execute o processo launch_processes para lançar os processos button_process e led_process
```bash
$ cd bin
$ ./launch_processes
```
## Matando os processos
Para matar os processos criados execute o script kill_process.sh
```bash
$ cd bin
$ ./kill_process.sh
```
## Conclusão
_FIFO_ é um IPC um pouco mais versátil do que [Pipes](https://github.com/NakedSolidSnake/Raspberry_IPC_Pipe), permite que diversos processos se comuniquem entre si através desse recurso, porém pode se tornar um pouco mais complicado quando se necessita entregar as mensagens para um processo específico, necessitando a implementação de filtros por parte da aplicação que consome o conteúdo.

## Referências
* [Linux Programming Interface](https://www.amazon.com.br/dp/B004OEJMZM/ref=dp-kindle-redirect?_encoding=UTF8&btkr=1)
* [fork e exec](https://github.com/NakedSolidSnake/Raspberry_fork_exec)
* [lib hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware)
