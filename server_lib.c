#include "server_lib.h"
#include "cliente.h"
#include "error-handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <locale.h>
// #include <curses.h>
#include <wchar.h>
#include <termios.h>

#include <dirent.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ESC 27
// #define ENTER 13
#define ENTER 10

int recebe_mensagem(int soquete, msgT *mensagem, int timeout)
{
    while (1)
    {
        // cuida do timeout
        struct pollfd fds;

        fds.fd = soquete;
        fds.events = POLLIN;

        int retorno_poll = poll(&fds, 1, TIMEOUT);

        if (timeout)
        {
            if (retorno_poll == 0)
                return TIMEOUT_RETURN;
            else if (retorno_poll < 0)
                return 0;
        }

        if (recv(soquete, mensagem, sizeof(msgT), 0) < 0)
        {
            return 0;
        }
        else if (retorno_poll > 0)
        {
            // if retorno_pull > 0 entao recebeu alguma mensagem, senao continua
            printf("recebeu marc: %s tipo %d e sequencia %d dados: %s\n\n", mensagem->marc_inicio, mensagem->tipo, mensagem->sequencia, mensagem->dados);
            return 1; // adicionar?
        }
    }
}

int recebeMensagemServerLoop(int soquete, msgT *mensagem)
{

    while (1)
    {
        // 0=DESLIGADO, POIS NAO DEVE DAR TIMEOUT QUANDO VAI RECEBER A PRIMEIRA MENSAGEM
        int retorno_func = recebe_mensagem(soquete, mensagem, 0);
        if (retorno_func == 1)
            printf("retorno_func %s \n", mensagem->tipo);
        // if (retorno_func == 0)
        //     perror("Erro ao receber mensagem no recebe_retorno");
        // else if (retorno_func == TIMEOUT_RETURN)
        // {
        //     perror("Timeout");
        //     continue;
        // }

        // if (mensagem->marc_inicio == MARC_INICIO)
        // {
        //     if (testa_paridade(mensagem))
        //         return mensagem->tipo;
        //     else
        //         mandaRetorno(1,soquete);
        // }
        // else
        //     mandaRetorno(0,soquete);
    }
}

int mandaRetorno(int isAck, int soquete)
{
    char *msg = "";
    msgT mensagem;
    // if (isAck){
    //     initMessage(&mensagem, 0, sequencia_global, NACK, msg);
    //     if( ! sendMessage (soquete, &mensagem)) {
    //         perror("Erro ao mandar nack");
    //     }

    // }else {
    //     initMessage(&mensagem, 0, sequencia_global, ACK, msg);
    //     if( ! sendMessage (soquete, &mensagem)) {
    //         perror("Erro ao mandar ack");
    //     }
    // }
}