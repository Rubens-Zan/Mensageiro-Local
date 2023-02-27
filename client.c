#include "./client_lib/client_lib.h"

int sequencia_global = 1;

int main()
{

    tCliente *client = (tCliente *)malloc(sizeof(tCliente));
    int soquete = ConexaoRawSocket("lo");
    FILE *file;
    client->estado = INICIO;
    client->socket = soquete; 

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
            // file = fopen(client->fileName, "rb");
            state_send_file(soquete, client);
            break;
        case ENVIA_ARQUIVO_PARAESPRA:
            // file = fopen(client->fileName, "rb");
            state_send_file_PARAESPERA(client);
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