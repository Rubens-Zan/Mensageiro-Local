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
#define MAX_TENTATIVAS 2

typedef enum tState
{
    INICIO,
    ENVIA_TEXTO,
    ENVIA_MENSAGEM,
    ENVIA_ARQUIVO,
    FIM_PROGRAMA
} tClientState;

typedef struct tCliente
{
    tClientState estado;
    char *fileName;
    msgT *message;
    int socket;
    unsigned int sequencia_atual;
} tCliente;

void state_init(tCliente *client);
void state_create_message(int soquete, tCliente *client);
void state_send_file(int soquete, tCliente *client);

void state_end(tCliente *client);
typesMessage recebeRetorno(int soquete, msgT *mensagem, int *contador, int seq_num);
void getStringAsBinary(bit *messageS, unsigned int *s, unsigned int tam, unsigned int binaryTam);
void incrementaSequencia();
FILE *abre_arquivo(char *nome_arquivo, char *modo);
#endif