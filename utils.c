#include "utils.h"
#include "cliente.h"
#include "error-handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <locale.h>
#include <curses.h>
#include <wchar.h>
#include <termios.h>

#include <dirent.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ESC 27
// #define ENTER 13
#define ENTER 10

unsigned int getNextPressedChar()
{

    wint_t wch;
    struct termios ttystate, ttysave;

    tcgetattr(STDIN_FILENO, &ttystate);
    ttysave = ttystate;
    ttystate.c_lflag &= ~(ICANON | ECHO);
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    setlocale(LC_ALL, "");
    wch = getwchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &ttysave);
    return wch;
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
    unsigned char output[binaryTam + 1];
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
    // s: Envia arquivo x.
    // q: Sai do programa.
    char buffer_c[100];
    char *filename_c;
    char char_code;

    while (1)
    {

        printf("Digite o comando <i: INSERT MESSAGE, q: QUIT, s: SEND>\n");
        // WINDOW * myWindow = initscr();
        char_code = getNextPressedChar();
        if (char_code == 'i')
        {
            printf("INSERT\n");
            client->estado = CRIACAO_MENSAGEM;
            return;
        }
        else if (char_code == 'q')
        {
            scanf("%63[^\n]", buffer_c);
            getchar();
            // printf("COMANDO %s\n", buffer_c);
            printf("QUIT\n");
            client->estado = FIM_PROGRAMA;
            return;
        }
        else if (char_code == 's')
        {
            filename_c = strtok(buffer_c, " ");
            filename_c = strtok(NULL, " ");
            client->estado = ENVIA_ARQUIVO;
            client->fileName = filename_c;
            return;
        }
        else
        {
            printf("COMANDO INVALIDO \n");
        }
    }
}

void incrementaSequencia()
{
    if (sequencia_global < 15)
        sequencia_global++;
    else
        sequencia_global = 1;
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
            printf("\nMENSAGEM A SER ENVIADA: \n");
            for (unsigned int i = 0; i < currentBufferPosition; ++i)
                printf("%d ", buffer_c[i]);

            printf("\n");

            incrementaSequencia();
            bit *myBinaryMsg = getStringAsBinary(buffer_c, currentBufferPosition, 8);
            client->message = initMessage(myBinaryMsg, currentBufferPosition * 8, TEXTO, sequencia_global);
            printf("myBinaryMsg %s \n", client->message->dados);
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

// TODO EFETUAR O ENVIO DA MENSAGEM PELO SOCKET
void put_dados_cliente(int soquete, FILE *arq, int permissao)
{

    int contador = 1;
    printf("put_dados_cliente\n");

    msgT mensagem;

    bit buffer_arq[TAM_MAX_DADOS];
    int bytes_lidos = fread(buffer_arq, sizeof(bit), (TAM_MAX_DADOS / 2) - 1, arq);
    while (bytes_lidos != 0)
    {
        contador++;
        // init_mensagem(&mensagem, bytes_lidos, sequencia_global, DADOS, buffer_arq);
        initMessage(buffer_arq, bytes_lidos, DADOS, sequencia_global);
        int ack = 0;
        while (!ack)
        {
            // if (! manda_mensagem (soquete, &mensagem, 0))
            // perror("Erro ao enviar mensagem no put_dados");
            printf("dados.. %d, buffer_arq \n %s", bytes_lidos, buffer_arq);
            // switch (recebe_retorno(soquete, &mensagem)) {

            // se for ack, quebra o laço interno e vai pro laço externo pegar mais dados
            //  case ACK:
            ack = 1;
            // break;
            // }
        }
        memset(buffer_arq, 0, TAM_MAX_DADOS);
        bytes_lidos = fread(buffer_arq, sizeof(bit), (TAM_MAX_DADOS / 2) - 1, arq);
    }

    // manda uma mensagem do tipo FIM
    //  char permissao_string[TAM_MAX_DADOS - 1];
    //  sprintf(permissao_string, "%d", permissao);

    // init_mensagem(&mensagem, strlen(permissao_string), sequencia_global, FIM, permissao_string);

    // considerando que o servidor responde um FIM com um ACK
    //  while (1){
    //  if (! manda_mensagem (soquete, &mensagem, 0))
    //  perror("Erro ao enviar mensagem no put_dados");

    // switch (recebe_retorno(soquete, &mensagem)) {
    // se for ack, acaba
    // case ACK:
    // printf("put_dados_cliente: recebeu um ack do server, retornando...\n");
    // printf("Contador -> %d\n", contador);
    return;
    // }
    // }
}

void state_send_message(tCliente *client, int socket)
{
    // while (1)
    // {
    //     printf("ENVIE A MENSAGEM: %d %d %d %d %s\n", client->message->marc_inicio,client->message->paridade,client->message->tam_msg,client->message->tipo,client->message->dados);
    // }

    // Send message to server

    if (send(socket, client->message, sizeof(msgT), 0) < 0)
    {
        return;
    }

    // Status of message: DONE
    printf("Mensagem enviada: %s\n", client->message->dados);
    client->estado = INICIO;
}

FILE *abre_arquivo(char *nome_arquivo, char *modo)
{

    char arquivo[65536]; // buffer imenso

    strcpy(arquivo, "./");
    strcat(arquivo, nome_arquivo);

    // abre o arquivo dado com o modo dado
    printf("ARQ %s MODO %s \n", arquivo, modo);

    FILE *arq = fopen(arquivo, modo);

    // retorna null se nao foi bem sucedido
    if (!arq)
    {
        perror("O arquivo nao pode ser aberto");
        return NULL;
    }

    // retorna o arquivo
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

int recebe_mensagem_server(int soquete, msgT *mensagem)
{

    while (1)
    {
        int retorno_func = recebe_mensagem(soquete, mensagem, 100000);
        if (retorno_func == 1)
            printf("retorno_func %s \n", mensagem->tipo);
        // if (retorno_func == 0)
        //     perror("Erro ao receber mensagem no recebe_retorno");
        // else if (retorno_func == 2)
        // {
        //     perror("Timeout");
        //     continue;
        // }

        // if (mensagem->marc_inicio == MARC_INICIO)
        // {
        //     if (testa_paridade(mensagem))
        //         return mensagem->tipo;
        //     else
        //         manda_nack(soquete);
        // }
        // else
        //     manda_nack(soquete);
    }
}

int recebe_mensagem(int soquete, msgT *mensagem, int timeout)
{
    while (1)
    {
        // printf("akns  ");
        // cuida do timeout
        struct pollfd fds;

        fds.fd = soquete;
        fds.events = POLLIN;

        int retorno_poll = poll(&fds, 1, TIMEOUT);

        if (timeout)
        {
            if (retorno_poll == 0)
                return 2;
            else if (retorno_poll < 0)
                return 0;
        }

        if (recv(soquete, mensagem, sizeof(msgT), 0) < 0){
            return 0;
        }else {
            printf("recebeu tipo %d e sequencia %d\n\n", mensagem->tipo, mensagem->sequencia);
            // return 1; //adicionar? 
        }

        // if (mensagem->sequencia != sequencia_global)
        // {
        //     continue;
        // }


        // if (sequencia_global >= 15)
        //     sequencia_global = 1;
        // else
        //     sequencia_global++;
        // return 1;
    }
}