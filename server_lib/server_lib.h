#ifndef _SERVER_LIB_H_
#define _SERVER_LIB_H_

#include "../general/general.h"
#include "../client_lib/client_lib.h"

typedef enum tServerState
{
    INICIO_RECEBIMENTO,
    RECEBE_TEXTO,
    RECEBE_ARQUIVO,
    FIM_PGMA
} tServerState;


typedef struct tServer
{
    tServerState estado;
    int socket;
    msgT *message;
    unsigned int sequencia_atual;
} tServer;

void recebeMensagemServerLoop(tServer *server);
void recebeMensagemTexto(tServer *server); 
void recebeMensagemArquivo(tServer *server);
#endif