#include "./server_lib/server_lib.h"

int sequencia_global = 1;

int main()
{

    int soquete = ConexaoRawSocket("lo");
    // int soquete = ConexaoRawSocket("eno1");
    // int soquete = ConexaoRawSocket("enp1s0");
    printf ("soquete -> %d\n", soquete);

    msgT mensagem;
    mensagem.sequencia = -1;

    printf("Server Initialized...\n");

    tServer *server = (tServer *)malloc(sizeof(tServer));
    server->socket = soquete;
    server->sequencia_atual=1;
    server->estado = INICIO_RECEBIMENTO;

    while (1)
    {

        // atraves da primeira mensagem vai para o proximo estado
        switch (server->estado) 
        {
            case INICIO_RECEBIMENTO:
                recebeMensagemServerLoop(server); 
                break;
            case RECEBE_ARQUIVO:
                recebeMensagemArquivo(server);
                break;

            case RECEBE_TEXTO:
                recebeMensagemTexto(server);
                break;
            // case NACK:
            //     //envia_nack()

            break;
        }

        // imprime_mensagem(&mensagem);
    }

    return 0;
}