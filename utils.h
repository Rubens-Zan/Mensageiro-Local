#ifndef _UTILS_H_
#define _UTILS_H_

#include "cliente.h"

#define TIMEOUT 1000*3

void state_init(tCliente *client); 
void state_create_message(tCliente *client); 
void state_send_file(tCliente *client); 
void state_end(tCliente *client); 
void state_send_message(tCliente *client, int socket); 
int recebe_mensagem (int soquete, msgT *mensagem, int timeout);
int recebe_mensagem_server(int soquete, msgT *mensagem); 
bit *getStringAsBinary(unsigned int *s, unsigned int tam, unsigned int binaryTam);

#endif