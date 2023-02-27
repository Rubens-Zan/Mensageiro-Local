#ifndef _SERVER_LIB_H_
#define _SERVER_LIB_H_

#include "../general/general.h"
#include "../client_lib/client_lib.h"
#include <math.h>

typedef enum tServerState
{
    INICIO_RECEBIMENTO,
    RECEBE_TEXTO,
    RECEBE_ARQUIVO,
    FIM_PGMA,
    RECEBE_ARQUIVO_PARAESPERA
} tServerState;


typedef struct tServer
{
    tServerState estado;
    int socket;
    msgT *message;
    unsigned int sequencia_atual;
} tServer;

/**
 * @brief Função para receber a primeira mensagem no servidor em loop
 * @param soquete
 * @param mensagem - Mensagem que vai receber
 */
void recebeMensagemServerLoop(tServer *server);

/**
 * @brief - Função para efetuar o recebimento da mensagem de texto
 *
 * @param server - Holds all the content to be received
 */
void recebeMensagemTexto(tServer *server); 

/**
 *
 * @brief - Função para recebimento de arquivos
 *
 * @param server - Holds all the content to be received
 */
void recebeMensagemArquivo(tServer *server);

void recebeMensagemArquivo_PARAESPERA(tServer *server); 

#endif