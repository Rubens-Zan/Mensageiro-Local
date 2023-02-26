#include "./client_lib.h"
#include <time.h>

#define PACKET_SIZE 2
#define AVAILABLE_UNS_CHARS_PER_MSG (TAM_MAX_DADOS / (PACKET_SIZE * 8)) // SINCE ONE UNSIGNED CHAR WILL BE CONVERTED TO 8 BITS

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

/* ESTADOS DO CLIENTE*/
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
            printf("> SEND:");
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

// TODO EFETUAR O ENVIO DA MENSAGEM PELO SOCKET
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
            unsigned int sequenciaAtual = 0;

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
            initMessage(&mensagem, "000001", 6, INIT, 1);

            if (sendMessage(client->socket, &mensagem))
            {
                printf("> MENSAGEM DE INICIO ENVIADA! %d %d %s\n", mensagem.tipo, mensagem.sequencia, mensagem.dados);
            }
            else
            {
                printf("> MENSAGEM DE INICIO NAO ENVIADA!\n");
            }

            while (remainingSize > 0)
            {
                int contador = 1;
                int ack = 0;
                while (!ack)
                {
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
                memset(auxString, 0, AVAILABLE_UNS_CHARS_PER_MSG + 1);
                unsigned int sentChars = (sequenciaAtual - 1) * AVAILABLE_UNS_CHARS_PER_MSG;
                unsigned int uCharsInMessage = (remainingSize < AVAILABLE_UNS_CHARS_PER_MSG) ? remainingSize : AVAILABLE_UNS_CHARS_PER_MSG;
                bit mensagemEmBits[TAM_MAX_DADOS];

                memcpy(auxString, buffer_c + sentChars, uCharsInMessage * sizeof(unsigned int)); // TODO COPIAR PARA AUXSTRING A PARTIR DA POS DA ULTIMA COPIADA
                // auxString[uCharsInMessage] = '\0';
                sequenciaAtual += 1;
                remainingSize -= AVAILABLE_UNS_CHARS_PER_MSG; // TAM_MAX_DADOS bits per message / 8 bits per char / 2 bits because of trelice

                getStringAsBinary(mensagemEmBits, auxString, uCharsInMessage, 8);
                initMessage(&mensagem, mensagemEmBits, uCharsInMessage * 8, TEXTO, sequenciaAtual);

                if (sendMessage(client->socket, &mensagem))
                {
                    time_t t;
                    time(&t);

                    printf("%s Mensagem: '%ls ' sequência: %d enviada!", ctime(&t), auxString, mensagem.sequencia);
                }
                else
                {
                    printf("=> Mensagem não enviada!\n");
                }
            }

            // Manda a mensagem de fim de transmissao apos nao sobrar tamanho a ser enviado
            initMessage(&mensagem, NULL, 0, END, sequenciaAtual + 1);
            if (sendMessage(client->socket, &mensagem))
                printf("> MENSAGEM DE FIM ENVIADA!\n");
            else
            {
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

void state_send_file(int soquete, tCliente *client)
{
    printf("\n=> FILE to be sent: %s\n", client->filename);
    // File opening
    FILE *file = openFile(client->fileName, "rb");
    if (fp == NULL)
    {
        printf("File not opened, try again\n");
        state_send_file(int soquete, tCliente *client);
    }

    // Send message of Initialization
    msgT ini_message;
    initMessage(&ini_message, "10000", 6, INIT, 1);
    int send_ret = sendMessage(client->soquete, &ini_message);
    if ( send_ret == 0)
    {
        printf("=> Initialization message sent successfully, proceeding with sending file.\n");
    }
    else
    {
        printf("> Initialization message failed to be send\n")
    }

    int window_size = 4;        // Size of Sliding Window
    Packet window[window_size]; // Window array to hold the packets
    int seq_num = 0;            // Initial sequence number
    int base = 0;               // the sequence number of the oldest unacknowledged packet
    int bytes_read;             // Variable to Hold number of bytes read

    while (bytes_read > 0)
    {
        for (int i = 0; i < window_size && bytes_read > 0; ++i) // Create Packets in the window
        {
            bytes_read = fread(window[i].data, sizeof(bit), TAM_MAX_DADOS, file;
            Packet packet; // construct packet
            memcpy(packet.data, window[i].data, bytes_read); // copies data from the window array to the packet buffer
            packet.seq_num = seq_num; // Packet sequence number receive actual sequence number
            window[i] = packet;
            printf("--> Number of bytes Read: %d, Packet %d:, Data: %s\n", bytes_read, i, window[i].data); // Print of variables
        }

        for (int i = 0; i < window_size; ++i) // Send packet in the window
        {
            int sent = send(sock, &window[i], sizeof(packet), 0);
            if (sent == -1)
            {
                perror("> Error sending packet\n");
                return 1;
            }
            printf("=> Package %d of window was successfully sent\n", i);

            // Increase the sequence number
            if (seq_num < 8)
            {
                printf("=> Increasing sequence number %d in one\n", seq_num);
                ++seq_num;
            }
            else
            {
                printf("=> Reseting sequence number %d to 0\n", seq_num);
                seq_num = 0;
            }
        }

        // check for ACKs and NACKs
        while (1)
        {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int select_result = select(soquete + 1, &read_fds, NULL, NULL, &timeout);
            if (select_result == -1) // Error
            {
                perror("> Error in select\n");
                return 1;
            }
            else if (select_result == 0) // timeout expired, retransmit packets
            {
                for (int i = base; i < seq_num; i++)
                {
                    int sent = send(soquete, &window[i % window_size], sizeof(Packet), 0);
                    if (sent == -1)
                    {
                        perror("> Error retransmitting packet\n");
                        return 1;
                    }
                    printf("=> Package %d of window was successfully sent\n", i % window_size);
                }
            }
            else // acknowledgement received
            {
                Packet ack_packet;
                int recv_size = recv(soquete, &ack_packet, sizeof(Packet), 0);
                if (recv_size == -1) // Error on recv
                {
                    perror("> Error receiving acknowledgement\n");
                    return 1;
                }
                else if (recv_size == 0) // Timeout
                {
                    fprintf(stderr, "> Server closed connection\n");
                    return 1;
                }
                else // Success
                {
                    printf("=> ACK Received\n");
                    base = ack_packet.seq_num + 1;
                }
            }

            if (base == seq_num) // all packets have been acknowledged
            {
                break;
            }
        }

        for (int i = 0; i < window_size; ++i)
        {
            memset(window[i].data, 0, TAM_MAX_DADOS); // Reset buffer with 0;
        }

        memset(packet.data, 0, TAM_MAX_DADOS); // Reset buffer with 0;
    }

    // Envia mensagem do tipo FIM
    initMessage(&mensagem, NULL, TAM_MAX_DADOS, END, seq_num);
    if (!sendMessage(soquete, &mensagem))
    {
        perror("> Erro ao enviar mensagem de Fim de Transmissão\n");
    }

    // Recebe ACK para a mensagem de FIM
    while (1)
    {
        switch (recebeRetorno(soquete, &mensagem, &contador, seq_num))
        {
        case ACK: // Success
            printf("=> Received ACK for end of transmission\n");
            return;
        case NACK: // Failed
            printf("=> Recebeu NACK for end of transmission\n");
            return;
        default: // Other cases
            printf("=> Not recognized\n");
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

                if (!sendMessage(soquete, mensagem))
                {
                    printf("> Erro ao re-mandar mensagem no recebe_retorno\n");
                }
                else
                {
                    printf("> Reenviei a mensagem\n");
                }
            }

            // Senão retorna o tipo
            else
            {
                if ((unsigned int)mensagem_aux.tipo == ACK)
                {
                    printf("ack %d\n", mensagem_aux.sequencia);
                    return (unsigned int)mensagem_aux.tipo;
                }
                else if ((unsigned int)mensagem_aux.tipo == NACK)
                {
                    printf("Nack %d\n", mensagem_aux.sequencia);
                    return (unsigned int)mensagem_aux.tipo;
                }
                else
                {
                    printf("nao sei oq eh isso %d", mensagem_aux.tipo);
                }
            }
        }
        else
        {
            printf("marcador de inicio errado");
        }
    }
}
