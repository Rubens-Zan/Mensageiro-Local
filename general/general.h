#ifndef _GENERAL_H_
#define _GENERAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>
#include <termios.h>
#include <dirent.h>
#include <poll.h>

// #define ENTER 13
#define ENTER 10
#define ESC 27
#define TIMEOUT 1000 * 3
#define TIMEOUT_RETURN 2
#define BUFFER_GIGANTE 65536

#define TAM_BUF 100
#define TAM_MAX_DADOS 512
#define MARC_INICIO 0x7e
#define BITTOINT(bitRepresentation) (bitRepresentation - '0')
#define INTTOBIT(intBit) (intBit + '0')
#define HANNINGDISTANCE(receivedBit, testedBit, packageSize) (return strncmp(receivedBit, testedBit, packageSize))

extern int sequencia_global;

typedef unsigned char bit;

typedef struct msgT
{
    bit marc_inicio : 8; // Marcador de início
    bit tam_msg : 6;     // Tamanho da mensagem
    bit sequencia : 4;   // Número da mensagem (até 16)
    bit tipo : 6;        // Tipo da mensagem
    bit dados[512];       // Buffer dos dados [64bytes]
    bit paridade : 8;    // Paridade
    int numero_ack;      //número do ACK
} msgT;

typedef enum
{
    A, //= 00,
    B, //= 01,
    C, //= 10,
    D  //= 11
} typesState;

typedef enum
{
    TEXTO = 0x01, // texto: 0x01, 000001
    MIDIA = 0x10, // mídia: 0x10, 10000
    ACK = 0x0A,   // ack: 0x0A, 001010
    NACK = 0x00,  // nack: 0x00, 000000
    ERRO = 0x1E,  // erro: 0x1E, 011110
    INIT = 0x1D,  // inicio de transmissão: 0x1D, 011101
    END = 0x0F,   // fim de transmissão: 0x0F, 001111
    DADOS = 0x0D  // dados: 0x0D, 001101
} typesMessage;

typedef struct tNode tNode;
struct tNode
{
    bit receivedBit;             // {1,0}
    bit correctedBits[3];        // "XY"
    typesState curState;         // {A,B,C,D}
    tNode *parent;               // Parent node
    tNode *left, *right;         // Next path, being left if input is 0 and right 1
    unsigned int shouldContinue; // determine if this path should be continued or not
    unsigned int pathError;
};

typedef struct tListNode tListNode;

struct tListNode
{
    tNode *value;
    tListNode *next;
};

int mandaRetorno(int isAck, int soquete, int sequencia);

/**********************ERROR*****************************************************************************************/

bit *viterbiAlgorithm(bit *receivedMessage, unsigned int packetSize, unsigned int msgSize);

/**
 * @brief Get the next state object
 * PATH DIAGRAM ACCORDING IF RECEIVED 0 OR 1
 * A -> A,C
 * B -> A,C
 * C -> B,D
 * D -> B,D
 *
 * @param curState{A,B,C,D} - Current state of the path
 * @param receivedBit{0,1} - Received bit
 * @param nextState{A,B,C,D} - Next state of the path
 */
void getNextState(typesState curState, unsigned int receivedBit, tNode *nextNode);

/**********************END_ERROR*****************************************************************************************/

/**********************BINARY_TREE*******************************************************************************************/

void free_binary_tree(tNode *root);
unsigned int countNodes(tNode *n);
tNode *startNode(unsigned int curPathError, unsigned int inputBit, typesState curState, unsigned int level, unsigned int packetSize, tNode *parentNode);
unsigned int height(tNode *p);
void emordem(tNode *no);
void printLevelOrder(tNode *root);
void printCurrentLevel(tNode *root, int level);
void getNextStep(tNode *root, unsigned int packetSize);
void getNextLeafOnLevel(tNode *root, int level, unsigned int packetSize, unsigned int height);
void getListLeafsHannigPathDistance(tNode *root, int level, unsigned int packetSize, unsigned int height, tNode **minHanningDistPathNode);
void getFullMessageDecoded(tNode *leaf);

/**********************END_BINARY_TREE*******************************************************************************************/

/**********************GENERATE_MESSAGE**********************************************************************************/

bit calculaParidade(bit *conteudo, unsigned int tam);
void trellisEncode(bit *encodedMessage, bit *originalMessage, unsigned int size);
void initMessage(msgT *mensagem, bit *originalMessage, unsigned int size, typesMessage msgType, int sequencia);

/**********************END_GENERATE_MESSAGE**********************************************************************************/

/**********************LIST**************************************************************************************************/

void insertfirst(tNode *element, tListNode **head);
unsigned int listSize(tListNode *head);
void prnList(tListNode *head);
bit *getDecodedMessage(tListNode *head, unsigned int decodedMsgSize);
void deleteList(tListNode **head_ref);

/**********************END_LIST**************************************************************************************************/

/**********************UTILS*****************************************************************************************************/

int sendMessage(int soquete, msgT *mensagem);
int ConexaoRawSocket(char *device);
void incrementaSequencia();

/**********************END_UTILS*****************************************************************************************************/

int recebe_mensagem(int soquete, msgT *mensagem, int timeout, unsigned int sequencia_atual);

#endif