#include "./client_lib.h"
#include <time.h>

#define PACKET_SIZE 2
// 32 bytes
#define AVAILABLE_UNS_CHARS_PER_MSG (TAM_MAX_DADOS/(PACKET_SIZE*8)) // SINCE ONE UNSIGNED CHAR WILL BE CONVERTED TO 8 BITS

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

void getFileName(char *file)
{
    struct termios original_attributes;
    tcgetattr(STDIN_FILENO, &original_attributes);

    // Set the terminal to canonical mode
    struct termios new_attributes = original_attributes;
    new_attributes.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_attributes);

    // Read input from the user
    unsigned int i = 0;
    char c;
    printf("\nINPUT FILE: ");
    while (read(STDIN_FILENO, &c, 1) > 0 && c != '\n')
    {
        printf("%c", c);
        file[i] = c;
        ++i;
    }
    printf("\n");

    // Restore the terminal's original attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &original_attributes);
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


void getStringAsBinary(bit *messageS, unsigned int *s, unsigned int tam, unsigned int binaryTam)
{
    // A small 9 characters buffer we use to perform the conversion
    unsigned char output[binaryTam + 1];
    unsigned int curPos = 0;

    for (unsigned int i = 0; i < tam; ++i)
    {
        bit *myConvertedNumb = convertToBin(s[i], binaryTam);
        strncpy(&messageS[curPos], myConvertedNumb, binaryTam);
        curPos += binaryTam;
        free(myConvertedNumb);
    }
    messageS[curPos] = '\0';
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
    char filename_c[50];
    char char_code;

    while (1)
    {

        printf("\n--> Digite o comando <i: INSERT MESSAGE, q: QUIT, s: SEND FILE>\n");
        // WINDOW * myWindow = initscr();
        char_code = getNextPressedChar();
        if (char_code == 'i' || char_code == 'I')
        {
            printf("> INSERT:\n");
            client->estado = ENVIA_TEXTO;
            return;
        }
        else if (char_code == 'q' || char_code == 'Q')
        {
            // scanf("%63[^\n]", buffer_c);
            // getchar();
            // printf("COMANDO %s\n", buffer_c);
            printf("> QUIT\n");
            client->estado = FIM_PROGRAMA;
            return;
        }
        else if (char_code == 's' || char_code == 'S')
        {
            printf("SEND");
            getFileName(filename_c);
            client->estado = ENVIA_ARQUIVO;
            client->fileName = filename_c;
            return;
        }
        else
        {
            printf("> COMANDO INVALIDO! \n");
        }
    }
}

void state_create_message(int soquete, tCliente *client)
{
    unsigned int char_code;
    unsigned int buffer_c[BUFFER_GIGANTE];
    unsigned int totalCharsInBuffer = 0;
    
    // TAM_MAX_DADOS / PACKET_SIZE * SIZEOF(UNSIGNED CHAR) 
    // USES 2 AS PACKET SIZE BECAUSE OF THE MDULATION CHOOSEN FOR THE TRELLICE CODE
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
            msgT mensagem;

            unsigned int sequenciaAtual=1;

            // totalCharsInBuffer = Amount of unsigned chars to send 
            int remainingSize = totalCharsInBuffer;

            #ifdef DEBUG
            printf("\n> MENSAGEM A SER ENVIADA: \n");
            for (unsigned int i = 0; i < totalCharsInBuffer; ++i)
                printf("%d ", buffer_c[i]);

            printf("\n");
            #endif
            
            printf("\n> Enviando a mensagem: %ls | separando em %d mensagens \n", buffer_c, totalCharsInBuffer / 16);
            // 000001 = TEXTO
            initMessage(&mensagem,  "000001",6, INIT, 1);


            if (sendMessage(client->socket, &mensagem)){
                printf("> MENSAGEM DE INICIO ENVIADA! %d %d %s\n", mensagem.tipo, mensagem.sequencia, mensagem.dados);

            }else {
                printf("> MENSAGEM DE INICIO NAO ENVIADA!\n");
            }

            while(remainingSize > 0){
                int contador = 1;
                int ack = 0;
                while (!ack){
                    switch (recebeRetorno(client->socket, &mensagem, &contador, sequenciaAtual))
                    {
                        case ACK:
                            ack = 1;
                            break;
                        case TIMEOUT:
                            ack = 1;
                            client->estado = INICIO;
                            return; 
                    }
                }


                unsigned int auxString[AVAILABLE_UNS_CHARS_PER_MSG + 1];
                memset(auxString,0, AVAILABLE_UNS_CHARS_PER_MSG + 1); 
                unsigned int sentChars = (sequenciaAtual-1) * AVAILABLE_UNS_CHARS_PER_MSG;
                unsigned int uCharsInMessage = (remainingSize < AVAILABLE_UNS_CHARS_PER_MSG) ? remainingSize : AVAILABLE_UNS_CHARS_PER_MSG;
                bit mensagemEmBits[TAM_MAX_DADOS];
                
                memcpy(auxString, buffer_c + sentChars, uCharsInMessage * sizeof(unsigned int)); //TODO COPIAR PARA AUXSTRING A PARTIR DA POS DA ULTIMA COPIADA
                // auxString[uCharsInMessage] = '\0';
                sequenciaAtual+=1;
                remainingSize-= AVAILABLE_UNS_CHARS_PER_MSG; // TAM_MAX_DADOS bits per message / 8 bits per char / 2 bits because of trelice 

                getStringAsBinary(mensagemEmBits, auxString, uCharsInMessage, 8);
                initMessage(&mensagem, mensagemEmBits, uCharsInMessage * 8, TEXTO, sequenciaAtual);

                if (sendMessage(client->socket, &mensagem)){
                    time_t t;
                    time(&t);

                    printf("%s Mensagem: '%ls ' sequência: %d enviada!", ctime(&t),auxString, mensagem.sequencia);
                }
                else
                {
                    printf("=> Mensagem não enviada!\n");
                }

                
            } 
            
            // Manda a mensagem de fim de transmissao apos nao sobrar tamanho a ser enviado
            initMessage(&mensagem,  NULL,0, END, sequenciaAtual+1);
            if (sendMessage(client->socket, &mensagem))
                printf("> MENSAGEM DE FIM ENVIADA!\n");
            else {
                printf("> MENSAGEM DE FIM NAO ENVIADA! \n");
            }

            client->estado = INICIO; // apos o envio das mensagens volta para o estado inicial
            return;
        }
        else
        {
            buffer_c[totalCharsInBuffer] = char_code;
            printf("%lc", buffer_c[totalCharsInBuffer]);
            ++totalCharsInBuffer;
            buffer_c[totalCharsInBuffer] = '\0';
        }
    }
}

// TODO EFETUAR O ENVIO DA MENSAGEM PELO SOCKET
void state_send_file(int soquete, tCliente *client)
{
    int window_size = 10; // tamanho da janela deslizante
    int contador = 1;
    printf("\nARQUIVO A SER ENVIADO:\n");
    FILE *meuArq = abre_arquivo(client->fileName, "rb");
    msgT mensagem;
    mensagem.sequencia = -1;
    bit buffer_arq[TAM_MAX_DADOS];

    int bytes_lidos = fread(buffer_arq, sizeof(bit), (TAM_MAX_DADOS / 2) - 1, meuArq);
    int seq_num = 0;      // número de sequência inicial
    int last_ack_num = 0; // último número de ACK recebido

    while (bytes_lidos != 0 || seq_num <= last_ack_num)
    {
        while ((seq_num < last_ack_num + window_size) && (bytes_lidos != 0))
        {
            // Envia pacote de dados com número de sequência seq_num
            initMessage(NULL, buffer_arq, bytes_lidos, DADOS, seq_num);
            if (!sendMessage(soquete, &mensagem))
            {
                perror("> Erro ao enviar mensagem no put_dados\n");
                // Tratar erro de envio
            }
            printf("--> dados.. %d, buffer_arq \n %s", bytes_lidos, buffer_arq);
            seq_num++;
            memset(buffer_arq, 0, TAM_MAX_DADOS);
            bytes_lidos = fread(buffer_arq, sizeof(bit), (TAM_MAX_DADOS / 2) - 1, meuArq);
        }

        // Recebe ACKs e NACKs
        while (1)
        {
            switch (recebeRetorno(soquete, &mensagem, &contador, seq_num))
            {
                case ACK:
                if (mensagem.numero_ack > last_ack_num)
                {
                    last_ack_num = mensagem.numero_ack;
                    break;
                }
                // else ignora ACKs repetidos
            // case NACK:
                // Tratar resposta de NACK
            // // default:
                // Tratar outras respostas
            }

            // Sai do loop de recebimento de ACKs se todos os pacotes enviados foram confirmados
            if (last_ack_num >= seq_num - 1)
            {
                break;
            }
        }
    }

    // Envia mensagem do tipo FIM
    initMessage(&mensagem, NULL, TAM_MAX_DADOS, END, seq_num);
    if (!sendMessage(soquete, &mensagem))
    {
        perror("> Erro ao enviar mensagem no put_dados\n");
    }

    // Recebe ACK para a mensagem de FIM
    while (1)
    {
        switch (recebeRetorno(soquete, &mensagem, &contador, seq_num))
        {
        case ACK: // Terminou a transmissão com sucesso
            printf("=> put_dados_cliente: recebeu um ack do server, retornando...\n");
            printf("=> Contador -> %d\n", contador);
            return;
        case NACK: // Tratar resposta de NACK
            printf("=> Recebeu NACK..\n");
            return;
        default: // Tratar outras respostas
            printf("=> Outras Respostas..\n");
            return;
        }
    }
}

void state_end(tCliente *client)
{
    while (1)
    {
        printf("\n> state_end state\n");
    }
}

typesMessage recebeRetorno(int soquete, msgT *mensagem, int *contador, int seqAtual)
{
    msgT mensagem_aux;
    mensagem_aux.sequencia = -1;
    printf("> Aguardando por ack da sequencia : %d\n", seqAtual);

    while (1)
    {
        // Recebe uma mensagem
        int retorno_func = recebe_mensagem(soquete, &mensagem_aux, 1, seqAtual);

        if (retorno_func == 0)
            printf("> Erro ao receber mensagem no recebe_retorno\n");

        // Verifica se o marcador de início e a paridade são os corretos
        if ((mensagem_aux.marc_inicio == MARC_INICIO) || (retorno_func == TIMEOUT_RETURN))
        {
            // se for um NACK, reenvia a mensagem
            if ((mensagem_aux.tipo == NACK) || (retorno_func == TIMEOUT_RETURN))
            {
                if (retorno_func == TIMEOUT_RETURN)
                    printf("> Timeout!!\n");

                // Se o contador de timeout ja foi no maximo de tentativas, retorna timeout
                if (*contador >= MAX_TENTATIVAS)
                {
                    return TIMEOUT;
                }

                // Incrementa o contador de NACKs recebidos
                if (mensagem->tipo == NACK)
                {
                    printf("RECEBI NACK"); 
                    (*contador)++;
                }

                // printf("Remandando o seguinte:\n");
                // imprime_mensagem(&mensagem_aux);
                // printf("\n");

                if (!sendMessage(soquete, mensagem)){
                    printf("> Erro ao re-mandar mensagem no recebe_retorno\n");
                }else{
                    printf("> Reenviei a mensagem\n");
                }
            }

            // Senão retorna o tipo
            else
            {
                if ((unsigned int)mensagem_aux.tipo == ACK){
                    printf("ack %d\n", mensagem_aux.sequencia);
                    return (unsigned int) mensagem_aux.tipo;
                    
                }else if((unsigned int)mensagem_aux.tipo == NACK){
                    printf("Nack %d\n", mensagem_aux.sequencia);
                    return (unsigned int) mensagem_aux.tipo;
                }else{
                    printf("nao sei oq eh isso %d",mensagem_aux.tipo);
                }

            }
        }else{
            printf("marcador de inicio errado");
        }
    }
}

