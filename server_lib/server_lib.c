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
    printf("decoded : %s ",msg);

    for (unsigned int i=0;i < size;i+=8){
        unsigned char curIntConverted[8+1];
        memcpy(curIntConverted, msg + i,8 * sizeof(unsigned char) );
        originalMessage[counter] = binaryToDecimal(curIntConverted);
        ++counter; 
    }

    printf("< MENSAGEM RECEBIDA: %ls \n",originalMessage);
}


/**
 * @brief Função para receber a primeira mensagem no servidor em loop 
 * @param soquete 
 * @param mensagem - Mensagem que vai receber
 * @return int 
*/
void recebeMensagemServerLoop(tServer *server)
{   
    msgT mensagemInit; 
    mensagemInit.sequencia = -1;

    printf("Estou esperando a primeira mensagem \n"); 
    while (1)
    {
        int retorno_func = recebe_mensagem(server->socket, &mensagemInit, 0, 1);
        
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
        unsigned int valor = binaryToDecimal(viterbiAlgorithm(mensagemInit.dados, 2, mensagemInit.tam_msg));

        if (mensagemInit.marc_inicio == MARC_INICIO 
        // && (mensagem.paridade == (unsigned int)calculaParidade(mensagem.dados,mensagem.tam_msg) || 
        // mensagem.paridade == (unsigned int)calculaParidade(mensagem.dados,mensagem.tam_msg) - 256 )
        ){

            if (valor == TEXTO ){
                printf("RECEBI UMA MENSAGEM DE INICIO DE TRANSMISSAO DE TEXTO: %d\n", valor); 
                server->estado = RECEBE_TEXTO;

            }else if (valor == MIDIA){
                printf("RECEBI UMA MENSAGEM DE INICIO DE TRANSMISSAO DE MIDIA: %d\n", valor); 
                server->estado = RECEBE_ARQUIVO;
            }
            server->estado = RECEBE_TEXTO;
            
            // printf("VALOR AQUI: %d ", valor); 
            mandaRetorno(1, server->socket, 1);

            return;
        }else{
            if (mensagemInit.marc_inicio != MARC_INICIO)
                printf("m.e ");
                // printf("MARCADOR DE INICIO DE ERRO NA MENSAGEM INICIAL \n");
            else{
                printf("p.e ");
                // printf("PARIDADE ERRADA RECEBIDO: %d ESPERADO: %d\n", mensagem.paridade, (unsigned int)calculaParidade(mensagem.dados,mensagem.tam_msg));

            } 
            mandaRetorno(0, server->socket, 1);
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
    msgT mensagemTxt; 
    unsigned int sequencia_atual = 2;

    printf("< Estou aguardando recebimento do texto \n");

    while (1){
        mensagemTxt.sequencia = -1;
        int retorno_func = recebe_mensagem(server->socket, &mensagemTxt, 1,sequencia_atual );

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

        if (mensagemTxt.tipo == TEXTO){

         // efetua verificações e envia nack/ack
            if ( mensagemTxt.marc_inicio == MARC_INICIO 
            // && (mensagem.paridade == (unsigned int)calculaParidade(mensagem.dados, mensagem.tam_msg) || 
            // mensagem.paridade == (unsigned int)calculaParidade(mensagem.dados, mensagem.tam_msg) - 256 )
            ){
                bit *decodedMessage = viterbiAlgorithm(mensagemTxt.dados,2,mensagemTxt.tam_msg);
                printf("RECEBI tm : %d paridade: %d \n",mensagemTxt.tam_msg * 8 ,mensagemTxt.paridade );
                printOriginalMessage(decodedMessage,mensagemTxt.tam_msg * 8); 
                printf("\n");
                
                mandaRetorno(1, server->socket, mensagemTxt.sequencia);
                ++sequencia_atual;         
            }
            else{
                printf("Erro na paridade esperado: %c recebido: %d",mensagemTxt.paridade, (unsigned int)calculaParidade(mensagemTxt.dados, mensagemTxt.tam_msg));
                mandaRetorno(0, server->socket, mensagemTxt.sequencia);
            }
        }else if (mensagemTxt.tipo == END){
            printf("<Recebi a mensagem de fim de transmissão de texto \n");

            mandaRetorno(1, server->socket, mensagemTxt.sequencia);
            server->estado = INICIO_RECEBIMENTO;
            return; 
        }else{
            printf("tipoerrado %d ",mensagemTxt.tipo );
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
