#ifndef _SERVER_LIB_H_
#define _SERVER_LIB_H_

#include "general.h"
#include "client_lib.h"

int recebeMensagemServerLoop(int soquete, msgT *mensagem);

int mandaRetorno(int isAck, int soquete);

#endif