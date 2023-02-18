#ifndef _CLIENT_LIB_H_
#define _CLIENT_LIB_H_

#include "../general/general.h"
#include "../server_lib/server_lib.h"

// #define ENTER 13
#define ENTER 10
#define ESC 27
#define TIMEOUT 1000 * 3
#define TIMEOUT_RETURN 2
#define BUFFER_GIGANTE 65536

typedef enum tState
{
    INICIO,
    ENVIA_TEXTO,
    ENVIA_MENSAGEM,
    ENVIA_ARQUIVO,
    FIM_PROGRAMA
} tState;

typedef struct tCliente
{
    tState estado;
    char *fileName;
    msgT *message;
} tCliente;

void state_init(tCliente *client);
void state_create_message(int soquete, tCliente *client);
void state_send_file(int soquete, FILE *arq);

void state_end(tCliente *client);
int recebe_mensagem(int soquete, msgT *mensagem, int timeout);
int recebeMensagemServerLoop(int soquete, msgT *mensagem);
void getStringAsBinary(bit *messageS, unsigned int *s, unsigned int tam, unsigned int binaryTam);
void incrementaSequencia();

#endif