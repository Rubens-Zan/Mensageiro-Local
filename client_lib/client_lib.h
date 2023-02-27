#ifndef _CLIENT_LIB_H_
#define _CLIENT_LIB_H_

#include "../general/general.h"
#include "../server_lib/server_lib.h"
#include <time.h>

// #define ENTER 13
#define ENTER 10
#define ESC 27
#define TIMEOUT_RETURN 2
#define BUFFER_GIGANTE 65536
#define MAX_TENTATIVAS 5

typedef enum tState
{
    INICIO,
    ENVIA_TEXTO,
    ENVIA_MENSAGEM,
    ENVIA_ARQUIVO,
    FIM_PROGRAMA,
    ENVIA_ARQUIVO_PARAESPRA
} tClientState;

typedef struct tCliente
{
    tClientState estado;
    unsigned int fileName[50];
    unsigned int fileNameSize;
    msgT *message;
    int socket;
    unsigned int sequencia_atual;
} tCliente;


/**
 * @brief Get the Next Pressed Unsigned Char object
 * @return unsigned int
 */
unsigned int getNextPressedChar();

/**
 * @brief Get the Name of the file 
 * @return unsigned int
 */
unsigned int getFileName(unsigned int *filename_c);

/**
 * @brief Função para conversão de um número em binário,
 * já faz o append no início no número em binário conforme o bitsSize esperado
 * @param num - Numero a ser convertido
 * @param bitsSize - Quantidade de bits esperados no binario, para append no caracteres para UTF-8
 * @return bit*
 */
bit *convertToBin(unsigned int num, unsigned int bitsSize);

/**
 * @brief Get the String As Binary convertind the raw UF-8 message to binary
 *
 * @param messageS - Array de bits que recebe a string uf8 em binario conforme conversão
 * @param s  - Array contendo os caracteres em utf-8
 * @param tam - Tamanho do array de caracteres
 * @param binaryTam - Tamanho esperado dos tamanhos em binário
 */
void getStringAsBinary(bit *messageS, unsigned int *s, unsigned int tam, unsigned int binaryTam);

/* ESTADOS DO CLIENTE */
/**
 * @brief -Função do estado inicial do cliente, redireciona o estado do cliente conforme input
 * Funciona como o vim, sendo que sempre é lido o próximo caracter para verificação do próximo estado do cliente
 *  i: Inicia a criação de uma mensagem. Enter para enviar.
 *  s: Envia arquivo x.
 *  q: Sai do programa.
 * @param client
 */
void state_init(tCliente *client);

/**
 * @brief - Função para criação/envio da mensagem
 * Cria a mensagem recebendo o proximo caracter em UTF-8
 * ENTER: envia a mensagem
 * ESC: faz com que o estado do cliente volte ao estado inicial
 * @param soquete
 * @param client
 */
void state_create_message(int soquete, tCliente *client);

int  state_send_file(int soquete, tCliente *client);

void state_send_file_PARAESPERA(tCliente *client);

void state_end(tCliente *client);

/**
 * @brief - Função para recebimento de ACKS/NACKS usando técnica Para-Espera
 * Faz a retransmissão da mensagem em caso de timeout
 * Caso o número de tentativas de retransmissão/recebimento de ack seja excedido,
 * Conforme o retorno de TIMEOUT a função que chamou lida, podendo desistir de mandar a mensagem e retornar o cliente ao estado inicial
 * @param soquete - Socket do cliente
 * @param mensagem - Mensagem enviada, para caso seja neccessario efetuar o reenvio da mensagem
 * @param contador - Ponteiro para o contador de erros(NACK ou TIMEOUT), inicializar com 1
 * @param seqAtual - Sequencia atual da mensagem na transmissão
 * @return typesMessage
 */
typesMessage recebeRetorno(int soquete, msgT *mensagem, int *contador, int seqAtual);
#endif