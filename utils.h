#ifndef _UTILS_H_
#define _UTILS_H_

#include "cliente.h"
#include <stdio.h>

#define TIMEOUT 1000*3
#define TIMEOUT_RETURN 2
#define BUFFER_GIGANTE 65536
void state_init(tCliente *client); 
void state_create_message(int soquete,tCliente *client); 
void state_send_file(int soquete, FILE *arq); 

void state_end(tCliente *client); 
int recebe_mensagem (int soquete, msgT *mensagem, int timeout);
int recebeMensagemServerLoop(int soquete, msgT *mensagem); 
void getStringAsBinary(bit *messageS,unsigned int *s, unsigned int tam, unsigned int binaryTam);
void incrementaSequencia(); 
int sendMessage(int soquete, msgT *mensagem); 
#endif