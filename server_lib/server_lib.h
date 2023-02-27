#ifndef _SERVER_LIB_H_
#define _SERVER_LIB_H_

#include "../general/general.h"
#include "../client_lib/client_lib.h"
#include <math.h>

typedef enum tServerState
{
    INICIO_RECEBIMENTO,
    RECEBE_TEXTO,
    RECEBE_ARQUIVO,
    FIM_PGMA,
    RECEBE_ARQUIVO_PARAESPERA
} tServerState;


typedef struct tServer
{
    tServerState estado;
    int socket;
    msgT *message;
    unsigned int sequencia_atual;
} tServer;

/**
 * @brief - Função para a conversão de um número em binário em decimal
 * @param binary - String de bits para conversão
 * @param tam  - Tamanho da string
 * @return int
 */
int binaryToDecimal(unsigned char *binary, unsigned int tam);

/**
 * @brief - Função que converte o binário da mensagem recebida em unsigned char
 *
 * @param msg - Binário da mensagem sequencia recebido
 * @param size - Tamanho da mensagem nessa sequencia recebido
 * @param sequenciaAtual - Sequência atual da mensagem, para saber a quantidade de bits já recebidos
 * @param fullMessage - Array que recebe toda a mensagem das transmissões
 */
void printOriginalMessage(bit *msg, unsigned int size, unsigned int sequenciaAtual, unsigned int *fullMessage);

/**
 * @brief Função para receber a primeira mensagem no servidor em loop
 * @param server
 */
void recebeMensagemServerLoop(tServer *server);

/**
 * @brief - Função para efetuar o recebimento da mensagem de texto
 *
 * @param server
 */
void recebeMensagemTexto(tServer *server); 

/**
 *
 * @brief - Função para recebimento de arquivos em Janela Desizante VoltaN
 *
 * @param server - Holds all the content to be received
 */
void recebeMensagemArquivo(tServer *server);

/**
 *
 * @brief - Função para recebimento de arquivos em Para-Espera
 *
 * @param server - Holds all the content to be received
 */
void recebeMensagemArquivo_PARAESPERA(tServer *server); 
#endif