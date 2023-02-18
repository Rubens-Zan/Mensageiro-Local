#include "server_lib.h"

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