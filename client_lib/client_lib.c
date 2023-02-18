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
            client->estado = ENVIA_TEXTO;
            return;
        }
        else if (char_code == 'q')
        {
            // scanf("%63[^\n]", buffer_c);
            // getchar();
            // printf("COMANDO %s\n", buffer_c);
            printf("QUIT\n");
            client->estado = FIM_PROGRAMA;
            return;
        }
        else if (char_code == 's')
        {
            scanf("%63[^\n]", filename_c);
            getchar();

            printf("ARQ %s", filename_c);
            // filename_c = strtok(buffer_c, " ");
            // filename_c = strtok(NULL, " ");
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
            int inicio_mensagem = 0;
            // VERIFICAR RETORNO DE ACK/NACK
            unsigned int totalBitsMsg = currentBufferPosition * 8; // como sao UTF-8 vao utilizar 8 bits para cada wide char
            bit auxString[64];
            unsigned int remainingSize = currentBufferPosition;
            printf("\nMENSAGEM A SER ENVIADA: \n");
            for (unsigned int i = 0; i < currentBufferPosition; ++i)
                printf("%d ", buffer_c[i]);

            printf("\n");

            // memcpy(auxString, buffer_c + inicio_mensagem, 8); //TODO COPIAR PARA AUXSTRING A PARTIR DA POS DA ULTIMA COPIADA
            // ADICIONAR VERIFICAÇÃO DO TAMANHO, SENDO O CONTEUDO DE NO MAXIMO 63 BYTES
            // ATE O RESTANTE, SE CONSEGUIR, SENAO 64 NO MAX...
            // SE FOR MAIOR, IR SEPARANDO AS MENSAGENS
            bit myBinaryMsg[9];
            getStringAsBinary(myBinaryMsg, buffer_c, currentBufferPosition, 8);
            initMessage(client->message, myBinaryMsg, currentBufferPosition * 8, TEXTO, sequencia_global);
            incrementaSequencia();
            printf("myBinaryMsg %s \n", client->message->dados);
            // MANDAR A MENSAGEM CRIADA
            // SE CONSEGUI MANDAR ATE O FINAL

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
void state_send_file(int soquete, FILE *arq)
{

    int contador = 1;
    printf("put_dados_cliente\n");

    msgT mensagem;

    bit buffer_arq[TAM_MAX_DADOS];
    int bytes_lidos = fread(buffer_arq, sizeof(bit), (TAM_MAX_DADOS / 2) - 1, arq);
    while (bytes_lidos != 0)
    {
        contador++;
        // initMessage(buffer_arq, bytes_lidos, DADOS, sequencia_global);
        int ack = 0;
        while (!ack)
        {
            // if (! sendMessage (soquete, &mensagem, 0))
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

    // init_mensagem(&mensagem,  sequencia_global, FIM);

    // considerando que o servidor responde um FIM com um ACK
    //  while (1){
    //  if (! sendMessage (soquete, &mensagem, 0))
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

void state_end(tCliente *client)
{
    while (1)
    {
        printf("\n state_end state");
    }
}

/**FIM ESTADOS DO CLIENTE*/
typesMessage recebeRetorno(int soquete, msgT *mensagem)
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
        // if ((mensagem->marc_inicio == MARC_INICIO) || (retorno_func == TIMEOUT_RETURN)) {
        //     //Testa a paridade
        //     if (testa_paridade(mensagem) || (retorno_func == TIMEOUT_RETURN)) {

        //         //se for um NACK, reenvia a mensagem
        //         if ((mensagem->tipo == NACK) || (retorno_func == TIMEOUT_RETURN)){
        // 			if (retorno_func == TIMEOUT_RETURN)
        // 				perror ("Timout");

        //             //aqui nao damos return pro laço recomeçar e esperar mais uma resposta
        //             char buffer_aux[TAM_MAX_DADOS];

        // 			memset(buffer_aux, 0, TAM_MAX_DADOS);
        //             memcpy(buffer_aux, mensagem_aux.dados, mensagem_aux.tam_msg);

        // 			initMessage(&mensagem_aux,buffer_aux, mensagem_aux.tam_msg, sequencia_global, mensagem_aux.tipo);

        // 			//printf("Remandando o seguinte:\n");
        // 			//imprime_mensagem(&mensagem_aux);
        // 			//printf("\n");

        //             if (! sendMessage (soquete, &mensagem_aux))
        //                 perror("Erro ao re-mandar mensagem no recebe_retorno_put");
        //         }

        //         // Senão retorna o tipo
        //         else {
        //             return mensagem->tipo;
        //         }

        //     }
        //     else{
        //     //retorna NACK para mensagens com erro no marcador ou na paridade
        //         mandaRetorno(0,soquete);
        //     }
        // }
        // else {
        //     mandaRetorno(1,soquete);
        // }
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

void incrementaSequencia()
{
    if (sequencia_global < 15)
        sequencia_global++;
    else
        sequencia_global = 1;
}
