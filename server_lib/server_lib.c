#include "./server_lib.h"

int binaryToDecimal(char* binary) {
    int decimal = 0;
    int length = strlen(binary);

    for (int i = 0; i < length; i++) {
        if (binary[i] == '1') {
            decimal += pow(2, length - 1 - i);
        }
    }

    return decimal;
}


/**
 * @brief Função para receber a primeira mensagem no servidor em loop 
 * @param soquete 
 * @param mensagem - Mensagem que vai receber
 * @return int - Retorna o tipo da mensagem, através deste o servidor vai para a função que trata o recebimento dos tipos de mensagem(arquivo e texto)
 */
int recebeMensagemServerLoop(int soquete, msgT *mensagem)
{
    while (1)
    {
        int retorno_func = recebe_mensagem(soquete, mensagem, 0);

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
        
        
        return mensagem->tipo;
    }
}

/**
 * @brief - Função para efetuar o recebimento da mensagem de texto
 * 
 * @param soquete 
 * @param mensagem 
 */
void recebeMensagemTexto(int soquete, msgT *mensagem){
    while (1){
        int retorno_func = recebe_mensagem(soquete, mensagem, 1);

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
        
        
        // efetua verificações e envia nack/ack
        

    }
}

/**
 * @brief - Função para recebimento de mensagens de arquivo 
 * 
 * @param soquete 
 * @param mensagem 
 */
void recebeMensagemArquivo(int soquete, msgT *mensagem){
    int proxima_sequencia = 1;
    int janela_inicio = 1;
    int window_size = 10;
    int janela_fim = janela_inicio + window_size - 1;

    while (1){
        int retorno_func = recebe_mensagem(soquete, mensagem, 1);

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

        // efetua verificações e envia nack/ack
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