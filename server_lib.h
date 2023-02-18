#ifndef _SERVER_LIB_H_
#define _SERVER_LIB_H_

#include "cliente.h"
#include <stdio.h>
#include "utils.h"

#define TIMEOUT 1000*3
#define TIMEOUT_RETURN 2
#define BUFFER_GIGANTE 65536

int recebe_mensagem(int soquete, msgT *mensagem, int timeout);

int recebeMensagemServerLoop(int soquete, msgT *mensagem);

int mandaRetorno(int isAck, int soquete);

#endif