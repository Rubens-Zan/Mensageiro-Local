#include "client_lib.h"

int main()
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
        case ENVIA_TEXTO:
            state_create_message(soquete, client);
            break;
        case ENVIA_ARQUIVO:
            FILE *file = fopen(client->fileName, "rb");
            state_send_file(soquete, file);
            break;
        case FIM_PROGRAMA:
            // state_end(client);
            return 1;
            break;
        default:
            break;
        }
    }

    free(client);

    return 1;
}