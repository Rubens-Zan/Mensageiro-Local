#include "utils.h"
#include "cliente.h"
#include "error-handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
#include <unistd.h>
#include <curses.h>

#define ESC 27
// #define ENTER 13
#define ENTER 10

unsigned int getNextPressedChar()
{
    initscr(); 
    timeout(-1);
    unsigned int c = getwchar();
    endwin();
    // unsigned char c; 
    // c = getchar();
     
    return c;
}

bit *convertToBin(unsigned int num, unsigned int bitsSize)
{
    bit *convertedNumb = (bit *)malloc((bitsSize + 1) * sizeof(bit));
    int a[10], n, i, j;
    n = num;
    j = 0;
    for (unsigned int i = 0; i < bitsSize; ++i)
    {
        a[i] = 0;
    }
    convertedNumb[bitsSize] = '\0';
    for (i = 0; n > 0; i++)
    {
        a[i] = n % 2;
        n = n / 2;
    }
    for (i = bitsSize - 1; i >= 0; i--)
    {
        convertedNumb[j] = a[i] + '0';
        ++j;
    }
    return convertedNumb;
}

char *append(char *str, unsigned int shifQt)
{
    size_t len = strlen(str);

    for (unsigned int i = 0; i < shifQt; ++i)
    {
        memmove(str + 1, str, ++len);
        *str = 1;
        str[0] = '0';
    }

    return str;
}

bit *getStringAsBinary(unsigned int *s, unsigned int tam, unsigned int binaryTam)
{
    // A small 9 characters buffer we use to perform the conversion
    unsigned char output[binaryTam+1];
    unsigned int curPos = 0;
    bit *myBinaryMessage = (bit *)malloc((binaryTam * tam) * sizeof(bit));
    for (unsigned int i = 0; i < tam; ++i)
    {
        bit *myConvertedNumb = convertToBin(s[i], binaryTam);
        strncpy(&myBinaryMessage[curPos], myConvertedNumb, binaryTam);
        curPos += binaryTam;
    }
    myBinaryMessage[curPos] = '\0';
    return myBinaryMessage;
}

/**
 * ESTADOS DO CLIENTE
 */

void state_init(tCliente *client)
{
    // i: Inicia a criação de uma mensagem. Enter para enviar.
    // <esc>: Sai do modo de inserção de uma mensagem.
    // :q<enter>: Sai do programa.
    // :send x<enter>: Envia arquivo x.
    char buffer_c[100];
    char *filename_c;
    char char_code;

    while (1)
    {

        printf("Digite o comando\n");
        // WINDOW * myWindow = initscr(); 
        // char_code = getNextPressedChar();
        char_code = getchar();
        if (char_code == 'i')
        {
            printf("INSERT\n");
            client->estado = CRIACAO_MENSAGEM;
            return;
        }
        else if (char_code == ':')
        {
            printf(":");
            scanf("%63[^\n]", buffer_c);
            getchar();
            // printf("COMANDO %s\n", buffer_c);

            if (strncmp(buffer_c, "q", 1) == 0)
            {
                printf("QUITE\n");
                client->estado = FIM_PROGRAMA;
                return;
            }
            else if (strncmp(buffer_c, "send", 4) == 0)
            {
                filename_c = strtok(buffer_c, " ");
                filename_c = strtok(NULL, " ");
                client->estado = ENVIA_ARQUIVO;
                client->fileName = filename_c;
                return;
            }
        }
        else
        {
            printf("COMANDO INVALIDO \n");
        }
    }
}

void incrementaSequencia(){
    if (sequencia_global < 15) sequencia_global++;
    else sequencia_global=1;
}

void state_create_message(tCliente *client)
{
    unsigned int char_code;
    unsigned int buffer_c[100];
    unsigned int currentBufferPosition = 0;

    while (1)
    {
        char_code = getNextPressedChar();
        // <esc>: Sai do modo de inserção de uma mensagem.
        if (char_code == ESC)
        {
            client->estado = INICIO;
            return;
        }
        else if (char_code == ENTER)
        {
            client->estado = ENVIA_MENSAGEM;
            printf("MENSAGEM A SER ENVIADA: \n");
            for (unsigned int i = 0; i < currentBufferPosition; ++i)
                printf("%d ", buffer_c[i]);

            printf("\n");

            incrementaSequencia(); 
            bit *myBinaryMsg = getStringAsBinary(buffer_c, currentBufferPosition, 8);
            client->message = initMessage(myBinaryMsg, currentBufferPosition * 8, TEXTO, sequencia_global);
            printf("myBinaryMsg %s \n",client->message->dados);
            printf("viterbi: %s \n", viterbiAlgorithm(client->message->dados,2, currentBufferPosition * 8 * 2));
            return;
        }
        else
        {
            buffer_c[currentBufferPosition] = char_code;
            printf("%c", buffer_c[currentBufferPosition]);
            ++currentBufferPosition;
            buffer_c[currentBufferPosition] = '\0';
        }
    }
}

//TODO EFETUAR O ENVIO DA MENSAGEM PELO SOCKET
void put_dados_cliente (int soquete, FILE * arq, int permissao){

    int contador = 1;
    printf("put_dados_cliente\n");

    msgT mensagem;

    bit buffer_arq[TAM_MAX_DADOS];
    int bytes_lidos = fread(buffer_arq, sizeof(bit), (TAM_MAX_DADOS / 2) - 1, arq);
    while (bytes_lidos != 0){
        contador++;
        // init_mensagem(&mensagem, bytes_lidos, sequencia_global, DADOS, buffer_arq);
        initMessage(buffer_arq,bytes_lidos, DADOS, sequencia_global);
        int ack = 0;
        while (! ack){
            // if (! manda_mensagem (soquete, &mensagem, 0))
                // perror("Erro ao enviar mensagem no put_dados");
            printf("dados.. %d, buffer_arq \n %s",bytes_lidos,buffer_arq);
            // switch (recebe_retorno(soquete, &mensagem)) {
                
                //se for ack, quebra o laço interno e vai pro laço externo pegar mais dados
                // case ACK:
                    ack = 1;
                    // break;
            // }
        }
        memset(buffer_arq, 0, TAM_MAX_DADOS);
        bytes_lidos = fread(buffer_arq, sizeof(bit), (TAM_MAX_DADOS / 2) - 1, arq);
    }

    //manda uma mensagem do tipo FIM
	// char permissao_string[TAM_MAX_DADOS - 1];
    // sprintf(permissao_string, "%d", permissao);

    // init_mensagem(&mensagem, strlen(permissao_string), sequencia_global, FIM, permissao_string);

    //considerando que o servidor responde um FIM com um ACK
    // while (1){
        // if (! manda_mensagem (soquete, &mensagem, 0))
            // perror("Erro ao enviar mensagem no put_dados");

        // switch (recebe_retorno(soquete, &mensagem)) {
            //se for ack, acaba
            // case ACK:
                // printf("put_dados_cliente: recebeu um ack do server, retornando...\n");
                // printf("Contador -> %d\n", contador);
                return;
        // }
    // }
}

void state_send_message(tCliente *client)
{
    // while (1)
    // {
    //     printf("ENVIE A MENSAGEM: %d %d %d %d %s\n", client->message->marc_inicio,client->message->paridade,client->message->tam_msg,client->message->tipo,client->message->dados); 
    // }

    // Send message to server
    // if (send(sock, client->message, strlen(client->message), 0) < 0) {
    //     perror("Erro ao enviar mensagem");
    //     return;
    // }

    // Status of message: DONE
    // printf("Mensagem enviada: %s\n", client->message);    
}

FILE *abre_arquivo(char *nome_arquivo, char *modo) {

    char arquivo[65536]; // buffer imenso 

    strcpy(arquivo, "./");
    strcat(arquivo, nome_arquivo);

    //abre o arquivo dado com o modo dado
    printf("ARQ %s MODO %s \n", arquivo,modo); 

    FILE *arq = fopen(arquivo, modo);

    //retorna null se nao foi bem sucedido
    if (! arq) {
        perror("O arquivo nao pode ser aberto");
        return NULL;
    }

    //retorna o arquivo
    return arq;

}


void state_send_file(tCliente *client)
{
    FILE *arq = abre_arquivo(client->fileName, "rb"); 

    while (1)
    {

        put_dados_cliente(12012, arq, 10);
        client->estado = FIM_PROGRAMA;
        return;
        // printf("\n SEND state");
    }
}

void state_end(tCliente *client)
{
    while (1)
    {
        printf("\n state_end state");
    }
}

/**FIM ESTADOS DO CLIENTE*/