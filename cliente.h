#ifndef _CLIENTE_H_
#define _CLIENTE_H_

#include "generate-message.h"

typedef enum tState
{
    INICIO,
    CRIACAO_MENSAGEM,
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

#endif