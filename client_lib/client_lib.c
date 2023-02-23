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

        printf("Digite o comando <i: INSERT MESSAGE, q: QUIT, s: SEND FILE>\n");
        // WINDOW * myWindow = initscr();
        char_code = getNextPressedChar();
        if (char_code == 'i' || char_code == 'I')
        {
            printf("INSERT\n");
            client->estado = ENVIA_TEXTO;
            return;
        }
        else if (char_code == 'q' || char_code == 'Q')
        {
            // scanf("%63[^\n]", buffer_c);
            // getchar();
            // printf("COMANDO %s\n", buffer_c);
            printf("QUIT\n");
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
            printf("COMANDO INVALIDO \n");
        }
    }
}

void state_create_message(int soquete, tCliente *client)
{
    unsigned int char_code;
    unsigned int buffer_c[BUFFER_GIGANTE];
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
            // TODO
            // MANDAR MENSAGEM DE INICIO

            // FAZER UM WHILE(1) AQUI PARA FAZER O PARA E ESPERA
            msgT mensagem;
            msgT mensagemInicio;

            int inicio_mensagem = 0;
            int ack = 0;
            int hasNextMessage = 1;
            int contador = 1;

            // VERIFICAR RETORNO DE ACK/NACK
            unsigned int totalBitsMsg = currentBufferPosition * 8; // como sao UTF-8 vao utilizar 8 bits para cada wide char
            bit auxString[512];
            unsigned int remainingSize = currentBufferPosition;
            printf("\nMENSAGEM A SER ENVIADA: \n");
            for (unsigned int i = 0; i < currentBufferPosition; ++i)
                printf("%d ", buffer_c[i]);

            printf("\n");

            // memcpy(auxString, buffer_c + inicio_mensagem, 8); //TODO COPIAR PARA AUXSTRING A PARTIR DA POS DA ULTIMA COPIADA
            // ADICIONAR VERIFICAÇÃO DO TAMANHO, SENDO O CONTEUDO DE NO MAXIMO 63 BYTES
            // ATE O RESTANTE, SE CONSEGUIR, SENAO 64 NO MAX...
            // SE FOR MAIOR, IR SEPARANDO AS MENSAGENS
           

            if (sendMessage(soquete, &mensagem))
                printf("Mensagem Enviada!\n");
            else {
                printf("Mensagem INICIO NAO Enviada!\n");
            }

            while (hasNextMessage){
                ack = 0;

                while (!ack){
                    switch (recebeRetorno(soquete, mensagem, &contador, sequencia_global))
                    {
                        case ACK:
                            ack = 1;
                            break;
                    }
                }
                bit myBinaryMsg[BUFFER_GIGANTE];
                getStringAsBinary(myBinaryMsg, buffer_c, currentBufferPosition, 8);
                
                initMessage(&mensagem, myBinaryMsg, currentBufferPosition * 8, TEXTO, sequencia_global);

                if (sendMessage(soquete, &mensagemInicio)){
                    printf("Mensagem Enviada!\n");
                }
                else
                {
                    printf("Mensagem não enviada!\n");
                }

                incrementaSequencia();
                hasNextMessage--;
            }


            // MANDA AS MENSAGENS
         
            // printf("myBinaryMsg %s %d\n", mensagem.dados,mensagem.sequencia);
            // MANDAR A MENSAGEM CRIADA
            // SE CONSEGUI MANDAR ATE O FINAL

            // RECEBO RETORNO DE INICIO DA TRANSMISSAO
            // while (!recebeRetorno()){

            // }
           
            // SAIO DO LAÇO E MANDO A MENSAGEM DE FIM

            client->estado = INICIO;
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

void state_send_file(int soquete, tCliente *client)
{
    int window_size = 10; // tamanho da janela deslizante
    int contador = 1;
    printf("\nARQUIVO A SER ENVIADO:\n");
    FILE *meuArq = abre_arquivo(client->fileName, "rb");
    msgT mensagem;
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
                perror("Erro ao enviar mensagem no put_dados");
                // Tratar erro de envio
            }
            printf("dados.. %d, buffer_arq \n %s", bytes_lidos, buffer_arq);
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
            case NACK:
                // Tratar resposta de NACK
            default:
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
        perror("Erro ao enviar mensagem no put_dados");
    }

    // Recebe ACK para a mensagem de FIM
    while (1)
    {
        switch (recebeRetorno(soquete, &mensagem, &contador, seq_num))
        {
        case ACK: // Terminou a transmissão com sucesso
            printf("put_dados_cliente: recebeu um ack do server, retornando...\n");
            printf("Contador -> %d\n", contador);
            return;
        case NACK: // Tratar resposta de NACK
            printf("Recebeu NACK..\n");
            return;
        default: // Tratar outras respostas
            printf("Outras Respostas..\n");
            return;
        }
    }
}

void state_end(tCliente *client)
{
    while (1)
    {
        printf("\nstate_end state\n");
    }
}

/**FIM ESTADOS DO CLIENTE*/
typesMessage recebeRetorno(int soquete, msgT *mensagem, int *contador, int seq_num)
{
    msgT mensagem_aux;

    mensagem_aux.tam_msg = mensagem->tam_msg;
    mensagem_aux.tipo = mensagem->tipo;
    memcpy(mensagem_aux.dados, mensagem->dados, mensagem->tam_msg);

    while (1)
    {
        // Recebe uma mensagem
        int retorno_func = recebe_mensagem(soquete, mensagem, 1);

        if (retorno_func == 0)
            perror("Erro ao receber mensagem no recebe_retorno");

        // Verifica se o marcador de início e a paridade são os corretos
        if ((mensagem->marc_inicio == MARC_INICIO) || (retorno_func == TIMEOUT_RETURN))
        {
            // Testa a paridade
            if (/*calculaParidade(mensagem, TAM_MAX_DADOS) || */ (retorno_func == TIMEOUT_RETURN))
            {
                // se for um NACK, reenvia a mensagem
                if ((mensagem->tipo == NACK) || (retorno_func == TIMEOUT_RETURN))
                {
                    if (retorno_func == TIMEOUT_RETURN)
                        perror("Timeout");

                    // aqui nao damos return pro laço recomeçar e esperar mais uma resposta
                    char buffer_aux[TAM_MAX_DADOS];

                    memset(buffer_aux, 0, TAM_MAX_DADOS);
                    memcpy(buffer_aux, mensagem_aux.dados, mensagem_aux.tam_msg);

                    initMessage(&mensagem_aux, buffer_aux, mensagem_aux.tam_msg, sequencia_global, mensagem_aux.tipo);

                    if (*contador >= MAX_TENTATIVAS)
                    {
                        return TIMEOUT;
                    }

                    // Incrementa o contador de NACKs recebidos
                    if (mensagem->tipo == NACK)
                    {
                        (*contador)++;
                    }

                    // printf("Remandando o seguinte:\n");
                    // imprime_mensagem(&mensagem_aux);
                    // printf("\n");

                    if (!sendMessage(soquete, &mensagem_aux))
                        perror("Erro ao re-mandar mensagem no recebe_retorno_put");
                }

                // Senão retorna o tipo
                else
                {
                    return mensagem->tipo;
                }
            }
            else
            {
                // retorna NACK para mensagens com erro no marcador ou na paridade
                mandaRetorno(0, soquete, seq_num);
            }
        }
        else
        {
            mandaRetorno(1, soquete, seq_num);
        }
    }
}

FILE *abre_arquivo(char *nome_arquivo, char *modo)
{

    char arquivo[BUFFER_GIGANTE]; // buffer imenso

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
