#include "./client_lib.h"

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

unsigned int getFileName(unsigned int *filename_c)
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
        printf("%lc", c);
        filename_c[i] = c;
        ++i;
    }
    printf("\n");

    // Restore the terminal's original attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &original_attributes);
    return i;
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

    char buffer_c[100];
    unsigned int filename_c[50];
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
            client->fileNameSize = getFileName(filename_c);
            memcpy(client->fileName, filename_c, client->fileNameSize * sizeof(unsigned int));
            if (USE_PARAESPERA_ARQ)
            {
                client->estado = ENVIA_ARQUIVO_PARAESPRA;
            }
            else
            {
                client->estado = ENVIA_ARQUIVO;
            }
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
            unsigned int sequenciaEsperada = 1;

            // totalCharsInBuffer = Amount of unsigned chars to send
            int remainingSize = totalCharsInBuffer;

            printf("\n> Enviando a mensagem: %ls | separando em %d mensagens \n", buffer_c, (totalCharsInBuffer / AVAILABLE_UNS_CHARS_PER_MSG) + 1);
            // 000001 = TEXTO
            initMessage(&mensagem, "000001", 6, INIT, 1,1);

            if (sendMessage(client->socket, &mensagem))
            {
                printf("> MENSAGEM DE INICIO ENVIADA! \n");
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
                    switch (recebeRetorno(client->socket, &mensagem, &contador, sequenciaEsperada))
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
                memset(mensagemEmBits, 0, TAM_MAX_DADOS);
                memcpy(auxString, buffer_c + sentChars, uCharsInMessage * sizeof(unsigned int)); // TODO COPIAR PARA AUXSTRING A PARTIR DA POS DA ULTIMA COPIADA
                // auxString[uCharsInMessage] = '\0';
                sequenciaAtual += 1;
                if (sequenciaEsperada >= MAX_SEQ)
                {
                    sequenciaEsperada = 1;
                }
                else
                {
                    ++sequenciaEsperada;
                };

                remainingSize -= AVAILABLE_UNS_CHARS_PER_MSG; // TAM_MAX_DADOS bits per message / 8 bits per char / 2 bits because of trelice

                getStringAsBinary(mensagemEmBits, auxString, uCharsInMessage, UTF8CHARSIZE);
                initMessage(&mensagem, mensagemEmBits, uCharsInMessage * UTF8CHARSIZE, TEXTO, sequenciaEsperada, 1);

                if (sendMessage(client->socket, &mensagem))
                {
                    time_t t;
                    time(&t);

                    printf("%s Mensagem: ", ctime(&t));
                    for (unsigned int k = 0; k < uCharsInMessage; ++k)
                        printf("%lc", auxString[k]);

                    printf(" sequência: %d enviada! \n", mensagem.sequencia);
                }
                else
                {
                    printf("=> Mensagem não enviada!\n");
                }
            }

            int ack = 0;
            while (!ack)
            {
                int contador = 0;
                switch (recebeRetorno(client->socket, &mensagem, &contador, sequenciaEsperada))
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

            if (sequenciaEsperada >= MAX_SEQ)
            {
                sequenciaEsperada = 1;
            }
            else
            {
                ++sequenciaEsperada;
            };
            // Manda a mensagem de fim de transmissao apos nao sobrar tamanho a ser enviado
            initMessage(&mensagem, NULL, 0, END, sequenciaEsperada, 0);
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
    // File opening
    char nomeDoArquivo[80];

    for (unsigned int i = 0; i < client->fileNameSize; ++i)
    {
        nomeDoArquivo[i] = (char)client->fileName[i];
    }

    FILE *file = openFile(nomeDoArquivo, "rb");
    if (file == NULL)
    {
        printf("File not opened, try again, %s \n", nomeDoArquivo);
        exit(1);
    }
    printf("\n=> FILE to be sent: %s\n", nomeDoArquivo);
    int seqAtual = 1;
    // Send message of Initialization
    msgT ini_message;
    initMessage(&ini_message, "010000", 6, INIT, seqAtual, 1); // envia a mensagem inicial
    int ack = 0;
    // INICIALIZAÇÃO, DEVO RECEBER DOIS ACK, UM PARA O INICIO DA TRANSMISSÃO E UM PARA O NOME DO ARQUIVO
    while (ack != 2)
    {
        int contador = 0;
        switch (recebeRetorno(client->socket, &ini_message, &contador, seqAtual))
        {
        case ACK:
            ++ack;
            ++seqAtual; // seqatual = 2
            // RECEBI ACK DA INICIALIZAÇÃO DA TRANSMISSÃO, ENTÃO VOU ENVIAR A MENSAGEM COM NOME DO ARQUIVO
            bit mensagemEmBits[TAM_MAX_DADOS];
            getStringAsBinary(mensagemEmBits, client->fileName, client->fileNameSize, UTF8CHARSIZE);
            initMessage(&ini_message, mensagemEmBits, client->fileNameSize * UTF8CHARSIZE, TEXTO, seqAtual, 1);
            if (sendMessage(client->socket, &ini_message))
                printf("> MENSAGEM DE NOME DO ARQUIVO ENVIADA!\n");
            else
            {
                printf("> MENSAGEM DE NOME DO ARQUIVO NAO ENVIADA! \n");
            }
            break;
        case TIMEOUT:
            ack = 1;
            printf("TIMEOUT!!");
            client->estado = INICIO;
            return -1; // retorna erro ? verificar
        }
    }

    struct pollfd fds; // cuida do timeout
    int retorno_poll;
    fds.fd = soquete;

    seqAtual = 0;
    int window_size = 16;                   // Size of Sliding Window
    int original_window_size = window_size; // Aux variable
    Packet window[window_size];             // Window array to hold the packets
    int seq_num = -1;                       // Initial sequence number
    int base = 0;                           // the sequence number of the oldest unacknowledged packet
    int bytes_read = 1;                     // Variable to Hold number of bytes read
    Packet packet;                          // Packet that hold data and sequence number

    while (!feof(file))
    {
        window_size = original_window_size;

        for (int i = 0; i < window_size && bytes_read > 0; i++) // Create Packets in the window
        {
            bytes_read = fread(window[i].data, sizeof(bit), TAM_BUF, file);
            memcpy(packet.data, window[i].data, bytes_read); // copies data from the window array to the packet buffer
            // Increase the sequence number
            if (seq_num < 15)
            {
                seq_num += 1;
            }
            else
            {
                printf("=> Reseting sequence number %d to 0\n", seq_num);
                seq_num = 0;
            }
            packet.seq_num = seq_num; // Packet sequence number receive actual sequence number
            window[i] = packet;
            printf("\n--> Packet - %d, Sequence Number: %d, Number of bytes Read: %d,  Data: %s\n", i, window[i].seq_num, bytes_read, window[i].data); // Print of variables
        }

        for (int i = 0; i < window_size; i++) // Send packet in the window
        {
            sleep(1); // Give time to server before receive ack and nack
            int sent = send(soquete, &window[i], sizeof(packet), 0);
            if (sent == -1)
            {
                perror("> Error sending packet\n");
                return 1;
            }
            printf("=> Package %d of window was successfully sent, size of Packet %d: %d\n", i, i, (int)sizeof(packet));
        }

        while (window_size > 0)
        {
            // cuida do timeout
            fds.events = POLLIN;

            msgT ack_message;
            initMessage(&ack_message, "00000", 6, NACK, seq_num,1);
            retorno_poll = poll(&fds, 1, TIMEOUT);

            if (retorno_poll == 0)
                return TIMEOUT_RETURN;
            else if (retorno_poll < 0)
                return 1;

            if (recv(soquete, &ack_message, sizeof(msgT), 0) < 0) // Error on recv
            {
                return 1;
            }
            if (ack_message.tipo == NACK)
            {
                for (int i = 0; i < window_size; ++i)
                {
                    if (window[i].seq_num == ack_message.sequencia)
                    {
                        int sent = send(soquete, &window[i], sizeof(packet), 0);
                        if (sent == -1)
                        {
                            perror("> Error resending packet\n");
                            return 1;
                        }
                        printf("> Received NACK %d, resending packet %d\n", ack_message.sequencia, window[i].seq_num);
                    }
                }
            }
            else if (ack_message.tipo == ACK)
            {
                printf("=> Received ACK %d\n", ack_message.sequencia);
                for (int i = 0; i < window_size; ++i)
                {
                    if (ack_message.sequencia == (window_size - 1))
                    {
                        window_size = 0;
                        printf("=> Window size is %d, reset\n", window_size);
                        break;
                    }
                    else if (window[i].seq_num < ack_message.sequencia)
                    {
                        // Remove the element in the i position of the window
                        int j;
                        for (int j = i; i < window_size - 1; i++)
                        {
                            window[j] = window[j + 1];
                        }
                        window_size--; // adjust the size of the window, to exclude last element
                    }
                    else if (window[i].seq_num == ack_message.sequencia)
                    {
                        // Remove the element in the i position of the window
                        int j;
                        for (int j = i; i < window_size - 1; i++)
                        {
                            window[j] = window[j + 1];
                        }
                        window_size--; // adjust the size of the window, to exclude last element
                    }
                    printf("=> Window size is %d", window_size);
                }
            }
        }

        for (int i = 0; i < original_window_size; ++i)
        {
            printf("=> Reseting window at index %d\n", i);
            memset(window[i].data, 0, TAM_BUF); // Reset buffer with 0;
        }

        printf("=> Reseting packet\n");
        memset(packet.data, 0, TAM_BUF); // Reset buffer with 0;
    }

    // Envia mensagem do tipo FIM
    msgT *end_message;
    initMessage(end_message, NULL, 6, END, seq_num,1);
    if (!sendMessage(soquete, end_message))
    {
        perror("> Erro ao enviar mensagem de Fim de Transmissão\n");
    }
    int retry = 0;
    while (retry < MAX_TENTATIVAS) // Receive ACK for end Message
    {
        fds.events = POLLIN;
        retorno_poll = poll(&fds, 1, TIMEOUT);

        if (TIMEOUT) // Timeout error
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
            printf("> Received NACK %d, resending end of transmission: %d\n", end_message->sequencia);
            if (!sendMessage(soquete, end_message))
            {
                perror("> Erro ao enviar mensagem de Fim de Transmissão\n");
            }
            retry++;
        }
        else if (end_message->tipo == ACK)
        {
            return 0; // File sent
        }
    }
    if (retry)
    {
        printf("=> Did not receive End of Transmission\n");
        return 1;
    }
}

void state_send_file_PARAESPERA(tCliente *client)
{
    // File opening
    char nomeDoArquivo[80];

    for (unsigned int i = 0; i < client->fileNameSize; ++i)
    {
        nomeDoArquivo[i] = (char)client->fileName[i];
    }

    FILE *file = openFile(nomeDoArquivo, "rb");
    if (file == NULL)
    {
        printf("File not opened, try again, %s \n", nomeDoArquivo);
        exit(1);
    }
    printf("\n=> FILE to be sent: %s\n", nomeDoArquivo);
    int seqAtual = 1;
    // Send message of Initialization
    msgT ini_message;
    initMessage(&ini_message, "010000", 6, INIT, seqAtual,1); // envia a mensagem inicial
    int ack = 0;
    // INICIALIZAÇÃO, DEVO RECEBER DOIS ACK, UM PARA O INICIO DA TRANSMISSÃO E UM PARA O NOME DO ARQUIVO
    while (ack != 2)
    {
        int contador = 0;
        switch (recebeRetorno(client->socket, &ini_message, &contador, seqAtual))
        {
        case ACK:
            ++ack;
            ++seqAtual; // seqatual = 2
            // RECEBI ACK DA INICIALIZAÇÃO DA TRANSMISSÃO, ENTÃO VOU ENVIAR A MENSAGEM COM NOME DO ARQUIVO
            bit mensagemEmBits[TAM_MAX_DADOS];
            getStringAsBinary(mensagemEmBits, client->fileName, client->fileNameSize, UTF8CHARSIZE);
            initMessage(&ini_message, mensagemEmBits, client->fileNameSize * UTF8CHARSIZE, TEXTO, seqAtual,1);
            if (sendMessage(client->socket, &ini_message))
                printf("> MENSAGEM DE NOME DO ARQUIVO ENVIADA!\n");
            else
            {
                printf("> MENSAGEM DE NOME DO ARQUIVO NAO ENVIADA! \n");
            }
            break;
        case TIMEOUT:
            ack = 1;
            printf("TIMEOUT!!");
            client->estado = INICIO;
            return;
        }
    }

    // LEITURA E ENVIO DOS PACOTES
    msgT mensagem;

    unsigned char buffer_arq[64];
    int bytes_lidos = fread(buffer_arq, sizeof(char), 64 - 1, file);
    while (bytes_lidos != 0)
    {
        initMessage(&mensagem, buffer_arq, bytes_lidos, MIDIA, seqAtual,0);
        ack = 0;
        while (!ack)
        {
            if (!sendMessage(client->socket, &mensagem))
                perror("Erro ao enviar mensagem no state_send_file_PARAESPERA");
           
           
            int contador = 0;
            switch (recebeRetorno(client->socket, &mensagem, &contador, seqAtual))
            {

            // se for ack, quebra o laço interno e vai pro laço externo pegar mais dados
            case ACK:
                ack = 1;
                break;
            }
        }
        memset(buffer_arq, 0, 64);
        bytes_lidos = fread(buffer_arq, sizeof(char), 64 - 1, file);
        if (seqAtual >= MAX_SEQ)
        {
            seqAtual = 1;
        }
        else
        {
            ++seqAtual;
        }
    }

    ack = 0;
    while (!ack)
    {
        int contador = 0;
        switch (recebeRetorno(client->socket, &mensagem, &contador, seqAtual-1))
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
    initMessage(&mensagem, NULL, 0, END, seqAtual, 0);
    if (sendMessage(client->socket, &mensagem))
        printf("> MENSAGEM DE FIM ENVIADA!\n");
    else
    {
        printf("> MENSAGEM DE FIM NAO ENVIADA! \n");
    }

    client->estado = INICIO; // apos o envio das mensagens volta para o estado inicial
    return;
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
                {
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