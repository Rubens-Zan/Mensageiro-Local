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

        switch (recebeMensagemServerLoop(soquete, &mensagem))
        {
            // case PUT:
            //     trata_put_servidor(soquete, &mensagem);
            //     break;

            // case GET:
            //     trata_get_servidor(soquete, &mensagem);
            //     break;
            // case MKDIR:
            //     trata_mkdir_servidor(soquete, &mensagem);
            //     break;

            // case LS:
            //     trata_ls_servidor(soquete, &mensagem);
            //     break;
            // case CD:
            //     trata_cd_servidor(soquete, &mensagem);
            //     break;
            // case NACK:
            //     //envia_nack()

            break;
        }
        printf("vms la ");

        // imprime_mensagem(&mensagem);
    }

    return 0;
}