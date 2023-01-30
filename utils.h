#ifndef _UTILS_H_
#define _UTILS_H_

#include "cliente.h"

void state_init(tCliente *client); 
void state_create_message(tCliente *client); 
void state_send_file(tCliente *client); 
void state_end(tCliente *client); 
void state_send_message(tCliente *client); 

bit *getStringAsBinary(unsigned int *s, unsigned int tam, unsigned int binaryTam);

#endif