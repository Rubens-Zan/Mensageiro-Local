#include "./server_lib.h"

int binaryToDecimal(unsigned char *binary)
{
    int decimal = 0, i = 0;
    size_t len = strlen(binary);
    for (int j = len - 1; j >= 0; j--)
    {
        if (binary[j] == '1')
        {
            decimal += pow(2, i);
        }
        i++;
    }
    return decimal;
}

void printOriginalMessage(bit *msg, unsigned int size)
{
    unsigned int originalMessage[BUFFER_GIGANTE];
    int counter = 0;
    printf("decoded : %s ", msg);

    for (unsigned int i = 0; i < size; i += 8)
    {
        unsigned char curIntConverted[8 + 1];
        memcpy(curIntConverted, msg + i, 8 * sizeof(unsigned char));
        originalMessage[counter] = binaryToDecimal(curIntConverted);
        ++counter;
    }

    printf("< MENSAGEM RECEBIDA: %ls \n", originalMessage);
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
        )
        {

            if (mensagem.marc_inicio == MARC_INICIO && mensagem.paridade == calculaParidade(mensagem.dados, mensagem.tam_msg))
            {

                if (valor == TEXTO)
                {
                    printf("RECEBI UMA MENSAGEM DE INICIO DE TRANSMISSAO DE TEXTO: %d\n", valor);
                    server->estado = RECEBE_TEXTO;
                }
                else if (valor == MIDIA)
                {
                    printf("RECEBI UMA MENSAGEM DE INICIO DE TRANSMISSAO DE MIDIA: %d\n", valor);
                    server->estado = RECEBE_ARQUIVO;
                }
                server->estado = RECEBE_TEXTO;

                // printf("VALOR AQUI: %d ", valor);
                mandaRetorno(1, server->socket, 1);

                return;
            }
            else
            {
                if (mensagemInit.marc_inicio != MARC_INICIO)
                    printf("m.e ");
                // printf("MARCADOR DE INICIO DE ERRO NA MENSAGEM INICIAL \n");
                else
                {
                    printf("p.e ");
                    // printf("PARIDADE ERRADA RECEBIDO: %d ESPERADO: %d\n", mensagem.paridade, (unsigned int)calculaParidade(mensagem.dados,mensagem.tam_msg));
                }
                mandaRetorno(0, server->socket, 1);
            }
        }
    }
}
/**
 * @brief - Função para efetuar o recebimento da mensagem de texto
 *
 * @param soquete
 * @param mensagem
 */
void recebeMensagemTexto(tServer *server)
{
    msgT mensagemTxt;
    unsigned int sequencia_atual = 2;

    printf("< Estou aguardando recebimento do texto \n");

    while (1)
    {
        mensagemTxt.sequencia = -1;
        int retorno_func = recebe_mensagem(server->socket, &mensagemTxt, 1, sequencia_atual);

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

        if (mensagemTxt.tipo == TEXTO)
        {

            // efetua verificações e envia nack/ack
            if (mensagemTxt.marc_inicio == MARC_INICIO
                // && (mensagem.paridade == (unsigned int)calculaParidade(mensagem.dados, mensagem.tam_msg) ||
                // mensagem.paridade == (unsigned int)calculaParidade(mensagem.dados, mensagem.tam_msg) - 256 )
            )
            {
                bit *decodedMessage = viterbiAlgorithm(mensagemTxt.dados, 2, mensagemTxt.tam_msg);
                printf("RECEBI tm : %d paridade: %d \n", mensagemTxt.tam_msg * 8, mensagemTxt.paridade);
                printOriginalMessage(decodedMessage, mensagemTxt.tam_msg * 8);
                printf("\n");

                mandaRetorno(1, server->socket, mensagemTxt.sequencia);
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

            mandaRetorno(1, server->socket, mensagemTxt.sequencia);
            server->estado = INICIO_RECEBIMENTO;
            return;
        }
        else
        {
            printf("tipoerrado %d ", mensagemTxt.tipo);
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
void recebeMensagemArquivo(tCliente *client)
{
    // Open the file for writing
    FILE *file = fopen(client->fileName, "wb");
    if (file == NULL)
    {
        printf("> Could not create file %s\n", client->fileName);
        return;
    }
    printf("=> File %s Successfully created on server.\n", client->filename)

    int window_size = 4;        // Size of the sliding window
    Packet window[window_size]; // Window array to hold the packets
    int base = 0;               // Sequence number of the oldest unacknowledged packet

    while (1)
    {
        // Receive packets
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(socket, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int select_result = select(socket + 1, &read_fds, NULL, NULL, &timeout);
        if (select_result == -1)
        { // Error
            perror("> Error in select\n");
            return;
        }
        else if (select_result == 0)
        { // Timeout
            printf("> Timeout\n");
            continue;
        }

        Packet packet;
        int recv_size = recv(socket, &packet, sizeof(Packet), 0);
        if (recv_size == -1)
        { // Error on recv
            perror("> Error receiving packet\n");
            return;
        }
        else if (recv_size == 0)
        { // Connection closed by client
            fprintf(stderr, "> Connection closed by client\n");
            return;
        }

        // Check if the packet is in the window
        if (packet.seq_num >= base && packet.seq_num < base + window_size)
        {
            // Write data to file
            int bytes_written = fwrite(packet.data, sizeof(bit), packet.length, file);
            if (bytes_written != packet.length)
            {
                printf("Error writing to file\n");
                return;
            }
            printf("Received packet %d\n", packet.seq_num);

            // Send ACK
            Packet ack_packet;
            initPacket(&ack_packet, NULL, 0, ACK, packet.seq_num);
            int sent = send(socket, &ack_packet, sizeof(Packet), 0);
            if (sent == -1)
            {
                perror("> Error sending ACK\n");
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
            window[window_size++] = packet;
        }
        else
        {
            // Packet not in window, send NACK
            Packet nack_packet;
            initPacket(&nack_packet, NULL, 0, NACK, base);
            int sent = send(socket, &nack_packet, sizeof(Packet), 0);
            if (sent == -1)
            {
                perror("> Error sending NACK\n");
                return;
            }
        }

        // Check if all packets have been received
        if (base == client->numPacotes)
        {
            break;
        }
    }

    fclose(file);
}
