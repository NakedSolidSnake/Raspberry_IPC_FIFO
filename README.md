<p align="center">
  <img src="https://media3.giphy.com/media/l0HlLRLbo6I1ZksqQ/source.gif"/>
</p>

# FIFO
## Tópicos
* [Introdução](#introdução)
* [Criando FIFO](#criando-fifo)
* [Implementação](#implementação)
* [launch_processes.c](#launch_processesc)
* [button_interface.c](#button_interfacec)
* [led_interface.c](#led_interfacec)
* [Compilando, Executando e Matando os processos](#compilando-executando-e-matando-os-processos)
* [Compilando](#compilando)
* [Clonando o projeto](#clonando-o-projeto)
* [Selecionando o modo](#selecionando-o-modo)
* [Executando](#executando)
* [Interagindo com o exemplo](#interagindo-com-o-exemplo)
* [MODO PC](#modo-pc-1)
* [MODO RASPBERRY](#modo-raspberry-1)
* [Matando os processos](#matando-os-processos)
* [Conclusão](#conclusão)
* [Referências](#referências)

## Introdução
FIFO é um IPC similar ao [PIPE](https://github.com/NakedSolidSnake/Raspberry_IPC_Pipe), mas diferente de PIPE possui um nome no filesystem, ou seja, é um arquivo como qualquer outro presente no filesystem, e pode ser manipulado com as funções de _open_, _read_, _write_ e _close_. Uma caracteristica desse IPC é que qualquer processo interessado nos dados que trafegam nesse IPC pode consumir esses dados.

## Criando FIFO
Para criar uma FIFO usa-se a _system call_ 
```c
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode);
```
Onde _pathname_ representa o caminho do arquivo a ser criado, e _mode_ representa as permissões do arquivo, e retorna 0 em caso de sucesso e -1 em caso de falha na criação do arquivo.

## Implementação
Como descrito em [PIPE](https://github.com/NakedSolidSnake/Raspberry_IPC_Pipe) será apresentando uma implementação de uma aplicação cliente-servidor.
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
## button_interface.c

Aqui criamos variáveis para receber o _handle_ do arquivo, um buffer para escrevermos o estado do botão, que servirá como base para a alteração do estado do LED, e uma para manter
```c
int fd; 
char buffer[2] = {0};
int state = 0;
```
Definimos o caminho onde o arquivo será criado, e por fim criamos o arquivo _FIFO_ com permissão de escrita e leitura para o usuário, grupo e outros
```c
char *fifo = "/tmp/fifo_ipc";
mkfifo(myfifo, 0666);
```
Inicializamos o botão com o descritor previamente configurado
```c
if(button->Init(object) == false)
    return EXIT_FAILURE;
```
Nesse loop é aguardado o pressionamento do botão para que a alteração do estado seja efetivado e o loop interrompido
```c
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
```
Nesse trecho, abrimos o arquivo da FIFO e adquirimos o handle do arquivo, formatamos o buffer com o valor presente na variável _state_ e escrevemos na FIFO, e fechamos o handle
```c
fd = open(fifo, O_WRONLY);
snprintf(buffer, sizeof(buffer), "%d", state);
write(fd, buffer, strlen(buffer)+1); 
close(fd);         	
```
## led_interface.c
Aqui criamos variáveis para receber o handle da FIFO, um buffer que será usado para ler o valor contido na FIFO, uma variável para guardar o estado, uma string com o caminho da FIFO
```c
int fd;
int state;
char buffer[2];

char *fifo = "/tmp/fifo_ipc";

```
Inicializamos a interface LED com o descritor previamente configurado
```c
if (led->Init(object) == false)
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

  fd = open(fifo,O_RDONLY); 
  read(fd, buffer, 2);
  close(fd); 
  state = atoi(buffer); 		

  led->Set(object, (uint8_t)state);
} 
```
## Compilando, Executando e Matando os processos
Para compilar e testar o projeto é necessário instalar a biblioteca de [hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware) necessária para resolver as dependências de configuração de GPIO da Raspberry Pi.

## Compilando
Para faciliar a execução do exemplo, o exemplo proposto foi criado baseado em uma interface, onde é possível selecionar se usará o hardware da Raspberry Pi 3, ou se a interação com o exemplo vai ser através de input feito por FIFO e o output visualizado através de LOG.

### Clonando o projeto
Pra obter uma cópia do projeto execute os comandos a seguir:

```bash
$ git clone https://github.com/NakedSolidSnake/Raspberry_IPC_FIFO
$ cd Raspberry_IPC_FIFO
$ mkdir build && cd build
```

### Selecionando o modo
Para selecionar o modo devemos passar para o cmake uma variável de ambiente chamada de ARCH, e pode-se passar os seguintes valores, PC ou RASPBERRY, para o caso de PC o exemplo terá sua interface preenchida com os sources presentes na pasta src/platform/pc, que permite a interação com o exemplo através de FIFO e LOG, caso seja RASPBERRY usará os GPIO's descritos no [artigo](https://github.com/NakedSolidSnake/Raspberry_lib_hardware#testando-a-instala%C3%A7%C3%A3o-e-as-conex%C3%B5es-de-hardware).

#### Modo PC
```bash
$ cmake -DARCH=PC ..
$ make
```

#### Modo RASPBERRY
```bash
$ cmake -DARCH=RASPBERRY ..
$ make
```

## Executando
Para executar a aplicação execute o processo _*launch_processes*_ para lançar os processos *button_process* e *led_process* que foram determinados de acordo com o modo selecionado.

```bash
$ cd bin
$ .launch_processes
```

Uma vez executado podemos verificar se os processos estão rodando atráves do comando 
```bash
$ ps -ef | grep _process
```

O output 
```bash
pi        5922     1  2 10:40 pts/1    00:00:00 ./button_process
pi        5923     1  0 10:40 pts/1    00:00:00 ./led_process
```
## Interagindo com o exemplo
Dependendo do modo de compilação selecionado a interação com o exemplo acontece de forma diferente

### MODO PC
Para o modo PC, precisamos abrir um terminal e monitorar os LOG's
```bash
$ sudo tail -f /var/log/syslog | grep LED
```

Dessa forma o terminal irá apresentar somente os LOG's referente ao exemplo.

Para simular o botão, o processo em modo PC cria uma FIFO para permitir enviar comandos para a aplicação, dessa forma todas as vezes que for enviado o número 0 irá logar no terminal onde foi configurado para o monitoramento, segue o exemplo
```bash
$ echo "0" > /tmp/input_fifo
```

Output do LOG quando enviado o comando algumas vezez
```bash
Apr  4 05:56:03 cssouza-Latitude-5490 LED FIFO[27328]: LED Status: On
Apr  4 05:56:05 cssouza-Latitude-5490 LED FIFO[27328]: LED Status: Off
Apr  4 05:56:07 cssouza-Latitude-5490 LED FIFO[27328]: LED Status: On
Apr  4 05:56:20 cssouza-Latitude-5490 LED FIFO[27328]: LED Status: Off
Apr  4 05:56:21 cssouza-Latitude-5490 LED FIFO[27328]: LED Status: On
Apr  4 05:56:22 cssouza-Latitude-5490 LED FIFO[27328]: LED Status: Off
Apr  4 05:56:23 cssouza-Latitude-5490 LED FIFO[27328]: LED Status: On
```

### MODO RASPBERRY
Para o modo RASPBERRY a cada vez que o botão for pressionado irá alternar o estado do LED.

## Matando os processos
Para matar os processos criados execute o script kill_process.sh
```bash
$ cd bin
$ ./kill_process.sh
```
## Conclusão
_FIFO_ é um IPC um pouco mais versátil do que [PIPE](https://github.com/NakedSolidSnake/Raspberry_IPC_Pipe), permite que diversos processos se comuniquem entre si através desse recurso, porém pode se tornar um pouco mais complicado quando se necessita entregar as mensagens para um processo específico, necessitando a implementação de filtros por parte da aplicação que consome o conteúdo.

## Referências
* [Mark Mitchell, Jeffrey Oldham, and Alex Samuel - Advanced Linux Programming](https://www.amazon.com.br/Advanced-Linux-Programming-CodeSourcery-LLC/dp/0735710430)
* [fork, exec e daemon](https://github.com/NakedSolidSnake/Raspberry_fork_exec_daemon)
* [biblioteca hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware)
