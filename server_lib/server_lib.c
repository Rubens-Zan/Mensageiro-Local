#include "./server_lib.h"
#include <math.h>

int binaryToDecimal(unsigned char* binary) {
    int decimal = 0, i = 0;
    size_t len = strlen(binary);
    for (int j = len - 1; j >= 0; j--) {
        if (binary[j] == '1') {
            decimal += pow(2, i);
        }
        i++;
    }
    return decimal;
}

void printOriginalMessage(bit *msg, unsigned int size ){
    unsigned int originalMessage[BUFFER_GIGANTE];
    int counter = 0;

    printf("mensagem %s sz %d \n\n", msg, size);

    for (unsigned int i=0;i < size;i+=8){
        unsigned char curIntConverted[8+1];
        memcpy(curIntConverted, msg + i,8 * sizeof(unsigned char) );
        // memcpy(auxString, buffer_c + sentChars, uCharsInMessage * sizeof(unsigned int)); //TODO COPIAR PARA AUXSTRING A PARTIR DA POS DA ULTIMA COPIADA
        printf("each: %s %d ",curIntConverted, counter);

        originalMessage[counter] = binaryToDecimal(curIntConverted);
        printf("[%d %d]",counter,originalMessage[counter]);
        ++counter; 
    }

    printf("ORIGINAL MESSAGE: %ls \n",originalMessage);
}


/**
 * @brief Função para receber a primeira mensagem no servidor em loop 
 * @param soquete 
 * @param mensagem - Mensagem que vai receber
 * @return int 
*/
void recebeMensagemServerLoop(tServer *server)
{   
    msgT mensagem; 
    mensagem.sequencia = -1;
    printf("Estou esperando a primeira mensagem \n"); 
    while (1)
    {
        int retorno_func = recebe_mensagem(server->socket, &mensagem, 0, server->sequencia_atual);
        
        if (retorno_func == TIMEOUT_RETURN)
        {
            printf("Timeout ao receber mensagem\n");
            continue;
        }
        else if (retorno_func == 0)
        {
            printf("Erro ao receber mensagem no loop\n");
            continue;
        } 

        unsigned int valor = binaryToDecimal(viterbiAlgorithm(mensagem.dados, 2, mensagem.tam_msg));
            
        if (mensagem.marc_inicio == MARC_INICIO && mensagem.paridade == calculaParidade(mensagem.dados,mensagem.tam_msg)){

            if (valor == TEXTO ){
                printf("RECEBI UMA MENSAGEM DE INICIO DE TRANSMISSAO DE TEXTO: %d\n", valor); 
                server->estado = RECEBE_TEXTO;

            }else if (valor == MIDIA){
                printf("RECEBI UMA MENSAGEM DE INICIO DE TRANSMISSAO DE MIDIA: %d\n", valor); 
                server->estado = RECEBE_ARQUIVO;
            }
            
            mandaRetorno(1, server->socket, mensagem.sequencia);

            // break;
            return;
        }else{
            if (mensagem.marc_inicio != MARC_INICIO)
                printf("MARCADOR DE INICIO DE ERRO NA MENSAGEM");
            else{
                printf("PARIDADE ERRADA %d %d\n", mensagem.paridade, calculaParidade(mensagem.dados,mensagem.tam_msg));

            } 
            // mandaRetorno(0, server->socket, mensagem.sequencia);
        }
    }
}

/**
 * @brief - Função para efetuar o recebimento da mensagem de texto
 * 
 * @param soquete 
 * @param mensagem 
 */
void recebeMensagemTexto(tServer *server){
    msgT mensagem; 
    mensagem.sequencia = -1;
    unsigned int sequencia_atual = 2;

    printf("Estou aguardando recebimento do texto \n");
    while (1){
        int retorno_func = recebe_mensagem(server->socket, &mensagem, 1,sequencia_atual );

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

        
        if(sequencia_atual != mensagem.sequencia){
            continue;
        }
        // printf("teste RECEBIDA: %d %d  %s  %d",mensagem.tipo, mensagem.sequencia, mensagem.dados, mensagem.tam_msg); 


        if (mensagem.tipo == TEXTO){

         // efetua verificações e envia nack/ack
            if ( mensagem.marc_inicio == MARC_INICIO){
                // mensagem.paridade == calculaParidade(mensagem.dados, mensagem.tam_msg)){
                bit *decodedMessage = viterbiAlgorithm(mensagem.dados,2,mensagem.tam_msg);
                printf("MENSAGEM RECEBIDA decoded: seq:%d decod: %s  \n",mensagem.sequencia, decodedMessage); 
                printOriginalMessage(decodedMessage,mensagem.tam_msg); 
                printf("\n");
                
                mandaRetorno(1, server->socket, mensagem.sequencia);
                ++sequencia_atual;         
            }
            else{
                printf("erro na paridade... ");
                mandaRetorno(0, server->socket, mensagem.sequencia);
            }
        }else{
                printf("Recebi a mensagem de fim de transmissão de texto \n");

                mandaRetorno(1, server->socket, mensagem.sequencia);
                server->estado = INICIO_RECEBIMENTO;
                return; 
        }
        


    }
}

/**
 * 
 * @brief - Função para recebimento de mensagens de arquivo 
 * 
 * @param soquete 
 * @param mensagem 
 */
void recebeMensagemArquivo(tServer *server){
    int proxima_sequencia = 1;
    int janela_inicio = 1;
    int window_size = 10;
    int janela_fim = janela_inicio + window_size - 1;
    msgT mensagem; 
    mensagem.sequencia = -1;
    printf("Estou aguardando recebimento do arquivo");

    while (1){
        int retorno_func = recebe_mensagem(server->socket, &mensagem, 1, sequencia_global);

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
        if (mensagem.tipo == DADOS)
        {
            if (mensagem.sequencia < proxima_sequencia || mensagem.sequencia > janela_fim)
            {
                printf("Mensagem fora da janela de recepção: sequencia %d\n", mensagem.sequencia);
                continue;
            }

            printf("Recebido pacote: sequencia %d\n", mensagem.sequencia);

            // processa mensagem...

            // atualiza a próxima sequência esperada
            proxima_sequencia = mensagem.sequencia + 1;

            // move a janela de recepção
            if (proxima_sequencia > janela_fim)
            {
                janela_inicio = proxima_sequencia;
                janela_fim = janela_inicio + window_size - 1;
            }

            // envia ACK
            mandaRetorno(ACK, server->socket, mensagem.sequencia);
        }
        else if (mensagem.tipo == END)
        {
            printf("Recebido pacote de fim\n");
            mandaRetorno(ACK, server->socket, mensagem.sequencia);
            // return END;
        }

    }
}
