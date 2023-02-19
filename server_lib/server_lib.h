#ifndef _SERVER_LIB_H_
#define _SERVER_LIB_H_

#include "../general/general.h"
#include "../client_lib/client_lib.h"

int recebeMensagemServerLoop(int soquete, msgT *mensagem);

#endif