#include "./server_lib.h"

int binaryToDecimal(unsigned char *binary, unsigned int tam)
{
    int decimal = 0, i = 0;
    for (int j = tam - 1; j >= 0; j--)
    {
        if (binary[j] == '1')
        {
            decimal += pow(2, i);
        }
        i++;
    }
    return decimal;
}

void printOriginalMessage(bit *msg, unsigned int size, unsigned int sequenciaAtual, unsigned int *fullMessage)
{
    wint_t originalMessage[BUFFER_GIGANTE];
    int counter = 0;
    setlocale(LC_ALL, "");

    for (unsigned int i = 0; i < size; i += UTF8CHARSIZE)
    {
        unsigned char curIntConverted[UTF8CHARSIZE + 1];
        memcpy(curIntConverted, msg + i, UTF8CHARSIZE * sizeof(unsigned char));
        originalMessage[counter] = (wchar_t)binaryToDecimal(curIntConverted, UTF8CHARSIZE);
        counter += 1;
    }
    unsigned int receivedChars = (sequenciaAtual - 2) * AVAILABLE_UNS_CHARS_PER_MSG;
    memcpy(fullMessage + receivedChars, originalMessage, (size / UTF8CHARSIZE) * sizeof(unsigned int));
    printf("Mensagem recebida: %ls sequência: %d", originalMessage, sequenciaAtual);
}

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
        unsigned int valor = binaryToDecimal(viterbiAlgorithm(mensagemInit.dados, PACKET_SIZE, mensagemInit.tam_msg), mensagemInit.tam_msg / PACKET_SIZE);

        if (mensagemInit.tipo == INIT)
        {

            if (mensagemInit.marc_inicio == MARC_INICIO && mensagemInit.paridade == calculaParidade(mensagemInit.dados, mensagemInit.tam_msg))
            {

                if (valor == TEXTO)
                {
                    printf("RECEBI UMA MENSAGEM DE INICIO DE TRANSMISSAO DE TEXTO: %d\n", valor);
                    server->estado = RECEBE_TEXTO;
                    mandaRetorno(1, server->socket, 1);
                }
                else if (valor == MIDIA)
                {
                    printf("RECEBI UMA MENSAGEM DE INICIO DE TRANSMISSAO DE MIDIA: %d\n", valor);
                    if (USE_PARAESPERA_ARQ){
                        server->estado = RECEBE_ARQUIVO_PARAESPERA;
                    }else{
                        server->estado = RECEBE_ARQUIVO;
                    }
                    mandaRetorno(1, server->socket, 1);
                }
                else
                {
                    mandaRetorno(0, server->socket, 1); // filtra pelo tipo da mensagem, se n for nem midia , nem texto nack
                }

                return;
            }
            else
            {
                if (mensagemInit.marc_inicio != MARC_INICIO)
                    printf("MARCADOR DE INICIO DE ERRO NA MENSAGEM INICIAL \n");
                else
                {
                    printf("PARIDADE ERRADA RECEBIDO: %d ESPERADO: %d\n", mensagemInit.paridade, (unsigned int)calculaParidade(mensagemInit.dados, mensagemInit.tam_msg));
                }
                mandaRetorno(0, server->socket, 1);
            }
        }
    }
}

void recebeMensagemTexto(tServer *server)
{
    msgT mensagemTxt;
    unsigned int sequencia_atual = 2;
    unsigned int sequencia_esperada = 2;

    unsigned int fullMessageReceived[BUFFER_GIGANTE];

    printf("< Estou aguardando recebimento do texto \n");
    unsigned int qtErros = 0;
    while (1)
    {
        if (qtErros > MAX_TENTATIVAS)
        {
            printf("MUITOS ERROS NAS TENTATIVAS, ESTOU RETORNANDO AO INICIO");
            server->estado = INICIO_RECEBIMENTO;
            return;
        }

        mensagemTxt.sequencia = -1;
        mensagemTxt.tam_msg = 0;
        memset(mensagemTxt.dados, 0, TAM_MAX_DADOS);

        int retorno_func = recebe_mensagem(server->socket, &mensagemTxt, 1, sequencia_esperada);

        if (retorno_func == TIMEOUT_RETURN)
        {
            printf("Timeout ao receber mensagem\n");
            ++qtErros;
            continue;
        }
        else if (retorno_func == 0)
        {
            printf("Erro ao receber mensagem\n");
            ++qtErros;
            continue;
        }

        qtErros = 0; // quantidade de erros volta para 0, pois consegui receber
        if (mensagemTxt.tipo == TEXTO)
        {

            // efetua verificações e envia nack/ack
            if (mensagemTxt.marc_inicio == MARC_INICIO && (mensagemTxt.paridade == (unsigned int)calculaParidade(mensagemTxt.dados, mensagemTxt.tam_msg) ||
                                                           mensagemTxt.paridade == (unsigned int)calculaParidade(mensagemTxt.dados, mensagemTxt.tam_msg) - 256))
            {
                bit *decodedMessage = viterbiAlgorithm(mensagemTxt.dados, PACKET_SIZE, mensagemTxt.tam_msg);
                printOriginalMessage(decodedMessage, mensagemTxt.tam_msg / PACKET_SIZE, sequencia_atual, fullMessageReceived);
                printf("\n");

                mandaRetorno(1, server->socket, mensagemTxt.sequencia);
                if (sequencia_esperada >= MAX_SEQ)
                {
                    sequencia_esperada = 1;
                }
                else
                {
                    ++sequencia_esperada;
                }
                ++sequencia_atual;
            }
            else
            {
                printf("Erro na paridade esperado: %c recebido: %d", mensagemTxt.paridade, (unsigned int)calculaParidade(mensagemTxt.dados, mensagemTxt.tam_msg));
                mandaRetorno(0, server->socket, mensagemTxt.sequencia);
            }
        }
        else if (mensagemTxt.tipo == END)
        {
            printf("<Recebi a mensagem de fim de transmissão de texto \n");
            printf("<Toda a mensagem recebida: '%ls' \n", fullMessageReceived);
            mandaRetorno(1, server->socket, mensagemTxt.sequencia);
            server->estado = INICIO_RECEBIMENTO;
            return;
        }
        else
        {
            printf("TIPO INESPERADO %d ", mensagemTxt.tipo);
            ++qtErros;
        }
    }
}

void recebeMensagemArquivo(tServer *server)
{
    printf("----> Receive media ACTIVATED <----\n"); // Function init

    // Receive filename from client
    int fileNameReceived = 0;
    msgT mensagemInit;
    mensagemInit.sequencia = -1;
    unsigned int filename_int[BUFFER_GIGANTE];
    char nomeDoArquivo[50];
    unsigned int tentativasReceberNome = 0;
    while (!fileNameReceived)
    {
        int retorno_func = recebe_mensagem(server->socket, &mensagemInit, 1, 2);

        if (tentativasReceberNome > MAX_TENTATIVAS)
        {
            printf("MUITOS ERROS NAS TENTATIVAS, ESTOU RETORNANDO AO INICIO");
            server->estado = INICIO_RECEBIMENTO;
        }

        if (retorno_func == TIMEOUT_RETURN)
        {
            printf("Timeout ao receber mensagem\n");
            ++tentativasReceberNome;
            continue;
        }
        else if (retorno_func == 0)
        {
            ++tentativasReceberNome;
            printf("Erro ao receber mensagem no loop\n");
            continue;
        }

        if (mensagemInit.tipo == TEXTO)
        {

            // efetua verificações e envia nack/ack
            if (mensagemInit.marc_inicio == MARC_INICIO && (mensagemInit.paridade == (unsigned int)calculaParidade(mensagemInit.dados, mensagemInit.tam_msg) ||
                                                            mensagemInit.paridade == (unsigned int)calculaParidade(mensagemInit.dados, mensagemInit.tam_msg) - 256))
            {
                bit *decodedMessage = viterbiAlgorithm(mensagemInit.dados, PACKET_SIZE, mensagemInit.tam_msg);

                printOriginalMessage(decodedMessage, mensagemInit.tam_msg / PACKET_SIZE, 2, filename_int);
                printf("\n");

                for (unsigned int i = 0; i < (mensagemInit.tam_msg / PACKET_SIZE); ++i)
                {
                    nomeDoArquivo[i] = (char)filename_int[i];
                }

                mandaRetorno(1, server->socket, mensagemInit.sequencia);
                fileNameReceived = 1;
            }
            else
            {
                mandaRetorno(0, server->socket, mensagemInit.sequencia);
            }
        }
    }
    printf("> Filename Received: %s \n", nomeDoArquivo);

    // Concatenate received filename with string "server_"
    char filename[100];
    strcpy(filename, "./server_");
    strcat(filename, nomeDoArquivo);
    // Open the file for writing
    char *buffer = filename;
    FILE *file = fopen(buffer, "wb");
    if (file == NULL)
    {
        printf("> Could not create file %s\n", buffer);
        return;
    }
    printf("=> File %s Successfully created on server.\n", buffer);

    // Treat data received
    struct pollfd fds; // cuida do timeout
    fds.fd = server->socket;
    int retorno_poll;
    int window_size = 16;       // Size of the sliding window
    Packet window[window_size]; // Window array to hold the packets
    unsigned int maxSeqPossivel = 3;
    int base = 0; // Sequence number of the oldest unacknowledged packet

    while (1)
    {
        for (int i = 0; i < window_size; i++) // Receive packet in the window
        {
            // fds.events = POLLIN;
            // retorno_poll = poll(&fds, 1, TIMEOUT);

            // if (TIMEOUT) // Timeout error
            // {
            //     if (retorno_poll == 0)
            //         return;
            //     else if (retorno_poll < 0)
            //         return;
            // }

            Packet packet;
            window[i] = packet;
            int recv_size = recv(server->socket, &window[i], sizeof(packet), 0);
            if (recv_size == -1) // Error on recv
            {
                perror("> Error receiving packet\n");
                return;
            }
            else if (recv_size == 0) // Connection closed by client
            {
                fprintf(stderr, "> Connection closed by client\n");
                return;
            }
            if (window[i].seq_num > maxSeqPossivel)
                continue;
            printf("=> Received packet %d, Data: %s.\n", window[i].seq_num, window[i].data); // Print total amount of received bytes
        }
        // Check if the packet is in the window

        if (window[0].seq_num <= base + window_size)
        {
            // Write data to file
            int bytes_written = fwrite(window[0].data, sizeof(bit), sizeof(Packet), file);
            if (bytes_written != sizeof(Packet))
            {
                printf("> Bytes written does not match with packet data size\n");
                return;
            }
            printf("=> Written %d bytes\n", bytes_written);

            // Send ACK
            msgT *ack_message;
            initMessage(ack_message, "001010", 6, ACK, window[0].seq_num,1);
            int sent = sendMessage(server->socket, ack_message);
            if (sent != 1)
            {
                printf("> Error sending ACK, for packet seq_num %d\n", window[0].seq_num);
                return;
            }

            // Slide window
            while (window[0].seq_num == base)
            {
                memmove(window, window + 1, sizeof(Packet) * (window_size - 1));
                --window_size;
                ++base;
            }
            // Add packet to window
            // window[window_size++] = packet;
        }
        else // Packet not in window, send NACK
        {
            // Send ACK
            msgT *ack_message;
            initMessage(ack_message, "000000", 6, NACK, window[0].seq_num,1);
            int sent = sendMessage(server->socket, ack_message);
            if (sent != 1)
            {
                printf("> Error sending NACK, for packet %d\n", window[0].seq_num);
                return;
            }
        }

        // Recebe mensagem do tipo FIM
        fds.fd = server->socket;
        fds.events = POLLIN;
        retorno_poll = poll(&fds, 1, TIMEOUT);
        msgT *end_message;

        if (TIMEOUT) // Timeout error
        {
            if (retorno_poll == 0)
                return;
            else if (retorno_poll < 0)
                return;
        }

        if (recv(server->socket, end_message, sizeof(msgT), 0) < 0) // Error on recv
        {
            return;
        }
        if (end_message->tipo == END)
        {
            printf("> Received END\n");
            break;
        }
        else
        {
            continue;
        }
    }

    fclose(file);
    return;
}

void recebeMensagemArquivo_PARAESPERA(tServer *server)
{
    printf("----> Receive media ACTIVATED <----\n"); // Function init

    // Receive filename from client
    int fileNameReceived = 0;
    msgT mensagemInit;
    msgT mensagem;
    mensagemInit.sequencia = -1;
    unsigned int sequencia_esperada = 2;

    unsigned int filename_int[BUFFER_GIGANTE];
    char nomeDoArquivo[50];
    unsigned int tentativasReceberNome = 0;

    // RECBIMENTO DO NOME DO ARQUIVO
    while (!fileNameReceived)
    {
        int retorno_func = recebe_mensagem(server->socket, &mensagemInit, 1, sequencia_esperada);

        if (tentativasReceberNome > MAX_TENTATIVAS)
        {
            printf("MUITOS ERROS NAS TENTATIVAS, ESTOU RETORNANDO AO INICIO");
            server->estado = INICIO_RECEBIMENTO;
            return;
        }

        if (retorno_func == TIMEOUT_RETURN)
        {
            printf("Timeout ao receber mensagem\n");
            ++tentativasReceberNome;
            continue;
        }
        else if (retorno_func == 0)
        {
            ++tentativasReceberNome;
            printf("Erro ao receber mensagem no loop\n");
            continue;
        }

        if (mensagemInit.tipo == TEXTO)
        {

            // efetua verificações e envia nack/ack
            if (mensagemInit.marc_inicio == MARC_INICIO && (mensagemInit.paridade == (unsigned int)calculaParidade(mensagemInit.dados, mensagemInit.tam_msg) ||
                                                            mensagemInit.paridade == (unsigned int)calculaParidade(mensagemInit.dados, mensagemInit.tam_msg) - 256))
            {
                bit *decodedMessage = viterbiAlgorithm(mensagemInit.dados, PACKET_SIZE, mensagemInit.tam_msg);

                printOriginalMessage(decodedMessage, mensagemInit.tam_msg / PACKET_SIZE, 2, filename_int);
                printf("\n");

                for (unsigned int i = 0; i < (mensagemInit.tam_msg / PACKET_SIZE); ++i)
                {
                    nomeDoArquivo[i] = (char)filename_int[i];
                }

                mandaRetorno(1, server->socket, mensagemInit.sequencia);
                fileNameReceived = 1;
                ++sequencia_esperada; 
            }
            else
            {
                mandaRetorno(0, server->socket, mensagemInit.sequencia);
            }
        }
   
    }
    printf("> Filename Received: %s \n", nomeDoArquivo);

    // Concatenate received filename with string "server_"
    char filename[100];
    strcpy(filename, "./server_");
    strcat(filename, nomeDoArquivo);
    // Open the file for writing
    char *buffer = filename;
    FILE *file = fopen(buffer, "wb");
    if (file == NULL)
    {
        printf("> Could not create file %s\n", buffer);
        return;
    }

    unsigned int qtErros = 0;

    // RECEBE O CONTEUDO DO ARQUIVO e escreve
     while (1)
    {
        if (qtErros > MAX_TENTATIVAS)
        {
            printf("MUITOS ERROS NAS TENTATIVAS, ESTOU RETORNANDO AO INICIO");
            server->estado = INICIO_RECEBIMENTO;
            return;
        }

        mensagem.sequencia = -1;
        mensagem.tam_msg = 0;
        memset(mensagem.dados, 0, TAM_MAX_DADOS);

        int retorno_func = recebe_mensagem(server->socket, &mensagem, 1, sequencia_esperada);

        if (retorno_func == TIMEOUT_RETURN)
        {
            printf("Timeout ao receber mensagem\n");
            ++qtErros;
            continue;
        }
        else if (retorno_func == 0)
        {
            printf("Erro ao receber mensagem\n");
            ++qtErros;
            continue;
        }

        qtErros = 0; // quantidade de erros volta para 0, pois consegui receber
        if (mensagem.tipo == MIDIA)
        {

            // efetua verificações e envia nack/ack
            if (mensagem.marc_inicio == MARC_INICIO)
            //  && (mensagem.paridade == (unsigned int)calculaParidade(mensagem.dados, mensagem.tam_msg) ||
                                                        //    mensagem.paridade == (unsigned int)calculaParidade(mensagem.dados, mensagem.tam_msg) - 256))
            {

                mandaRetorno(1, server->socket, mensagem.sequencia);
                if (fwrite(mensagem.dados, sizeof(char), mensagem.tam_msg, file) != mensagem.tam_msg){
                    perror("put_server(): Escrever tamanho errado no servidor");
                    return;
                }else{
                    printf("%d bytes escritos",mensagem.tam_msg);
                }

                if (sequencia_esperada >= MAX_SEQ)
                {
                    sequencia_esperada = 1;
                }
                else
                {
                    ++sequencia_esperada;
                }
            }
            else
            {
                printf("Erro na paridade esperado: %c recebido: %d", mensagem.paridade, (unsigned int)calculaParidade(mensagem.dados, mensagem.tam_msg));
                mandaRetorno(0, server->socket, mensagem.sequencia);
            }
        }
        else if (mensagem.tipo == END)
        {
            mandaRetorno(1, server->socket, mensagem.sequencia);
            server->estado = INICIO_RECEBIMENTO;
            fclose(file);
            return;
        }
        else
        {
            printf("TIPO INESPERADO %d %d", mensagem.tipo, sequencia_esperada);
            ++qtErros;
        }
    }


    printf("=> File %s Successfully created on server.\n", buffer);
}