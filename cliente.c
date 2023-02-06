#include <stdio.h>
#include <stdlib.h>
#include "cliente.h"
#include "utils.h"
#include "ConexaoRawSocket.h"

int sequencia_global = 1;

void main()
{

    tCliente *client = (tCliente *)malloc(sizeof(tCliente));
    int soquete = ConexaoRawSocket("lo");

    client->estado = INICIO;

    while (1)
    {
        switch (client->estado)
        {
        case INICIO:
            state_init(client);
            break;
        case CRIACAO_MENSAGEM:
            state_create_message(client);
            break;
        case ENVIA_MENSAGEM:
            state_send_message(client, soquete);
            break;
        case ENVIA_ARQUIVO:
            state_send_file(client);
            break;
        case FIM_PROGRAMA:
            // state_end(client);
            return;
            break;
        default:
            break;
        }
    }
}