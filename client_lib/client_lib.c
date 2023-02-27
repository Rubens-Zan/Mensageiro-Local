#include "./client_lib.h"
#include <time.h>


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

            unsigned int sequenciaAtual = 1;

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
                            printf("TIMEOUT DESISTO!!");
                            client->estado = INICIO;
                            return; 
                    }
                }

                unsigned int auxString[AVAILABLE_UNS_CHARS_PER_MSG + 1];
                memset(auxString, 0, AVAILABLE_UNS_CHARS_PER_MSG + 1);
                unsigned int sentChars = (sequenciaAtual - 1) * AVAILABLE_UNS_CHARS_PER_MSG;
                unsigned int uCharsInMessage = (remainingSize < AVAILABLE_UNS_CHARS_PER_MSG) ? remainingSize : AVAILABLE_UNS_CHARS_PER_MSG;
                bit mensagemEmBits[TAM_MAX_DADOS];
                memset(mensagemEmBits,0, TAM_MAX_DADOS);
                memcpy(auxString, buffer_c + sentChars, uCharsInMessage * sizeof(unsigned int)); //TODO COPIAR PARA AUXSTRING A PARTIR DA POS DA ULTIMA COPIADA
                // auxString[uCharsInMessage] = '\0';
                sequenciaAtual += 1;
                remainingSize -= AVAILABLE_UNS_CHARS_PER_MSG; // TAM_MAX_DADOS bits per message / 8 bits per char / 2 bits because of trelice

                getStringAsBinary(mensagemEmBits, auxString, uCharsInMessage, 8);
                initMessage(&mensagem, mensagemEmBits, uCharsInMessage * 8, TEXTO, sequenciaAtual);

                if (sendMessage(client->socket, &mensagem))
                {
                    time_t t;
                    time(&t);

                    printf("%s Mensagem: ", ctime(&t) );
                    for (unsigned int k=0;k < uCharsInMessage;++k)
                        printf("%lc",auxString[k]);

                    printf(" sequência: %d enviada! \n", mensagem.sequencia);
                }
                else
                {
                    printf("=> Mensagem não enviada!\n");
                }

                
            } 
            
            int ack = 0;
            while (!ack){
                int contador = 0;
                switch (recebeRetorno(client->socket, &mensagem, &contador, sequenciaAtual))
                {
                    case ACK:
                        ack = 1;
                        break;
                    case TIMEOUT:
                        ack = 1;
                        printf("TIMEOUT!!");
                        client->estado = INICIO;
                        return; 
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

int state_send_file(int soquete, tCliente *client)
{
    printf("\n=> FILE to be sent: %s\n", client->fileName);
    // File opening
    FILE *file = openFile(client->fileName, "rb");
    if (file == NULL)
    {
        printf("File not opened, try again\n");
        state_send_file(soquete, client);
    }

    // Send message of Initialization
    msgT ini_message;
    initMessage(&ini_message, "10000", 6, INIT, 1);
    int send_ret = sendMessage(soquete, &ini_message);
    if (send_ret == 1)
    {
        printf("=> Initialization message sent successfully, proceeding with sending file.\n");
    }
    else
    {
        printf("> Initialization message failed to be send\n");
    }
    // Receive ACK for Initialization
    while (1)
    {
        struct pollfd fds; // cuida do timeout
        fds.fd = soquete;
        fds.events = POLLIN;
        int retorno_poll = poll(&fds, 1, TIMEOUT);

        if (timeout) // Timeout error
        {
            if (retorno_poll == 0)
                return TIMEOUT_RETURN;
            else if (retorno_poll < 0)
                return 1;
        }

        if (recv(soquete, ini_message, sizeof(msgT), 0) < 0) // Error on recv
        {
            return 1;
        }
        if (ini_message->tipo == NACK)
        {
            printf("> Received NACK %d, resending init of transmission: %d\n", ini_message->sequencia);
            if (!sendMessage(soquete, &ini_message))
            {
                perror("> Erro ao enviar mensagem de Fim de Transmissão\n");
            }
        }
        else if (ini_message->tipo == ACK)
        {
            printf("=> ACK receive for initialization\n");
            break;
        }
    }

    // Sent filename
    if (send(sock, client->fileName, strlen(client->fileName), 0) < 0)
    {
        perror("Erro ao enviar mensagem");
        exit(1);
    }
    printf("=> Filename sent successfully.\n")

    int window_size = 3; // Size of Sliding Window
    int original_window_size = window_size;
    Packet window[window_size]; // Window array to hold the packets
    int seq_num = -1;           // Initial sequence number
    int base = 0;               // the sequence number of the oldest unacknowledged packet
    int bytes_read;             // Variable to Hold number of bytes read
    Packet packet;              // Packet that hold data and sequence number
    int chunk_size = 256;

    while (bytes_read > 0)
    {
        window_size = original_window_size;
        for (int i = 0; i < window_size && bytes_read > 0; i++) // Create Packets in the window
        {
            bytes_read = fread(window[i].data, sizeof(bit), chunk_size, file);
            memcpy(packet.data, window[i].data, bytes_read); // copies data from the window array to the packet buffer
            packet.seq_num = ++seq_num;                      // Packet sequence number receive actual sequence number
            window[i] = packet;
            printf("--> Packet - %d, Sequence Number: %d, Number of bytes Read: %d,  Data: %s\n", i, seq_num, bytes_read, window[i].data); // Print of variables
        }

        for (int i = 0; i < window_size; i++) // Send packet in the window
        {
            int sent = send(soquete, &window[i], sizeof(packet), 0);
            if (sent == -1)
            {
                perror("> Error sending packet\n");
                return 1;
            }
            printf("=> Package %d of window was successfully sent\n", i);
        }

        while (1)
        {
            // cuida do timeout
            struct pollfd fds;

            fds.fd = soquete;
            fds.events = POLLIN;

            msgT ack_message;
            ack_message.sequencia = -1;

            int retorno_poll = poll(&fds, 1, TIMEOUT);

            if (timeout) // Timeout error
            {
                if (retorno_poll == 0)
                    return TIMEOUT_RETURN;
                else if (retorno_poll < 0)
                    return 1;
            }

            if (recv(soquete, ack_message, sizeof(msgT), 0) < 0) // Error on recv
            {
                return 1;
            }
            if (ack_message->tipo == NACK)
            {
                for (int i = 0; i < window_size; ++i)
                {
                    if (window[i].seq_num == ack_message->sequencia)
                    {
                        int sent = send(soquete, &window[i], sizeof(packet), 0);
                        if (sent == -1)
                        {
                            perror("> Error resending packet\n");
                            return 1;
                        }
                        printf("> Received NACK %d, resending packet %d", ack_message->sequencia, window[i].seq_num);
                    }
                }
            }
            else if (ack_message->tipo == ACK)
            {
                printf("=> Received ACK %d\n", ack_message->sequencia);
                for (int i = 0; i < window_size; ++i)
                {
                    if (window[i].seq_num == ack_message->sequencia)
                    {
                        // Remove the element in the i position of the window
                        int j;
                        for (int j = i; i < window_size - 1; i++)
                        {
                            window[j] = window[j + 1];
                        }
                        window_size--; // adjust the size of the window, to exclude last element
                    }
                }
            }
        }

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

        for (int i = 0; i < original_window_size; ++i)
        {
            memset(window[i].data, 0, chunk_size); // Reset buffer with 0;
        }

        memset(packet.data, 0, chunk_size); // Reset buffer with 0;
    }

    // Envia mensagem do tipo FIM
    msgT end_message;
    initMessage(&end_message, NULL, chunk_size, END, seq_num);
    if (!sendMessage(soquete, &end_message))
    {
        perror("> Erro ao enviar mensagem de Fim de Transmissão\n");
    }
    while (1) // Receive ACK for end Message
    {        
        struct pollfd fds; // cuida do timeout

        fds.fd = soquete;
        fds.events = POLLIN;

        msgT end_message;
        end_message.sequencia = -1;

        int retorno_poll = poll(&fds, 1, TIMEOUT);

        if (timeout) // Timeout error
        {
            if (retorno_poll == 0)
                return TIMEOUT_RETURN;
            else if (retorno_poll < 0)
                return 1;
        }

        if (recv(soquete, end_message, sizeof(msgT), 0) < 0) // Error on recv
        {
            return 1;
        }
        if (end_message->tipo == NACK)
        {
            printf("> Received NACK %d, resending end of transmission: %d", end_message->sequencia);
            if (!sendMessage(soquete, &end_message))
            {
                perror("> Erro ao enviar mensagem de Fim de Transmissão\n");
            }
        }
        else if (end_message->tipo == ACK)
        {
            return 0; // File sent
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
                if (retorno_func == TIMEOUT_RETURN){
                    printf("> Timeout!!\n");
                    (*contador)++;
                }

                // Se o contador de timeout ja foi no maximo de tentativas, retorna timeout
                if (*contador >= MAX_TENTATIVAS)
                {
                    printf("RECEBI TIMEOUT");
                    return TIMEOUT;
                }

                // Incrementa o contador de NACKs recebidos
                if (mensagem->tipo == NACK)
                {
                    printf("RECEBI NACK");
                    (*contador)++;
                }

                // Remanda a mensagem 
                if (!sendMessage(soquete, mensagem))
                {
                    printf("> Erro ao re-mandar mensagem no recebe_retorno\n");
                }
                else
                {
                    printf("> Reenviei a mensagem: %d\n", mensagem->sequencia);
                }
            }

            // Senão retorna o tipo
            else
            {
                if ((unsigned int)mensagem_aux.tipo == ACK)
                {
                    printf("RECEBI RETORNO DE ACK: %d\n", mensagem_aux.sequencia);
                    return (unsigned int)mensagem_aux.tipo;
                }
                else if ((unsigned int)mensagem_aux.tipo == NACK)
                {
                    printf("RECEBI RETORNO DE NACK %d\n", mensagem_aux.sequencia);
                    return (unsigned int)mensagem_aux.tipo;
                }
            }
        }
        else
        {
            printf("marcador de inicio errado");
        }
    }
}
