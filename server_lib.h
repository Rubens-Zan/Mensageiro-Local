#ifndef _SERVER_LIB_H_
#define _SERVER_LIB_H_

#include "error_handle.h"
#include "client_lib.h"

#define ESC 27
// #define ENTER 13
#define ENTER 10

#define TIMEOUT 1000*3
#define TIMEOUT_RETURN 2
#define BUFFER_GIGANTE 65536

int recebe_mensagem(int soquete, msgT *mensagem, int timeout);

int recebeMensagemServerLoop(int soquete, msgT *mensagem);

int mandaRetorno(int isAck, int soquete);

#endif