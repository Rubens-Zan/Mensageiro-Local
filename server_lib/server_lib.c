#include "./server_lib.h"

int recebeMensagemServerLoop(int soquete, msgT *mensagem)
{
    int proxima_sequencia = 1;
    int janela_inicio = 1;
    int window_size = 10;
    int janela_fim = janela_inicio + window_size - 1;

    while (1)
    {
        int retorno_func = recebe_mensagem(soquete, mensagem, TIMEOUT);

        if (retorno_func == TIMEOUT_RETURN)
        {
            printf("Timeout ao receber mensagem\n");
            continue;
        }
        else if (retorno_func == 0)
        {
            printf("Erro ao receber mensagem\n");
            continue;
        }

        if (mensagem->tipo == DADOS)
        {
            if (mensagem->sequencia < proxima_sequencia || mensagem->sequencia > janela_fim)
            {
                printf("Mensagem fora da janela de recepção: sequencia %d\n", mensagem->sequencia);
                continue;
            }

            printf("Recebido pacote: sequencia %d\n", mensagem->sequencia);

            // processa mensagem...

            // atualiza a próxima sequência esperada
            proxima_sequencia = mensagem->sequencia + 1;

            // move a janela de recepção
            if (proxima_sequencia > janela_fim)
            {
                janela_inicio = proxima_sequencia;
                janela_fim = janela_inicio + window_size - 1;
            }

            // envia ACK
            mandaRetorno(ACK, soquete, mensagem->sequencia);
        }
        else if (mensagem->tipo == END)
        {
            printf("Recebido pacote de fim\n");
            mandaRetorno(ACK, soquete, mensagem->sequencia);
            return END;
        }
    }
}


