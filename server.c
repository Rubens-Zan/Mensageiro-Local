#include "./server_lib/server_lib.h"

int sequencia_global = 1;

int main()
{

    int soquete = ConexaoRawSocket("lo");
    // int soquete = ConexaoRawSocket("eno1");
    // int soquete = ConexaoRawSocket("enp1s0");
    printf ("soquete -> %d\n", soquete);

    msgT mensagem;
    printf("Server Initialized...\n");

    while (1)
    {

        // atraves da primeira mensagem vai para o proximo estado
        switch (recebeMensagemServerLoop(soquete, &mensagem)) 
        {
            case ENVIA_TEXTO:
                recebeMensagemArquivo(soquete, &mensagem);
                break;

            case ENVIA_ARQUIVO:
                recebeMensagemArquivo(soquete, &mensagem);
                break;
            // case NACK:
            //     //envia_nack()

            break;
        }

        // imprime_mensagem(&mensagem);
    }

    return 0;
}