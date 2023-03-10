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
#include <errno.h>

// #define ENTER 13
#define ENTER 10
#define ESC 27
#define TIMEOUT 1000 * 5
#define TIMEOUT_RETURN 2
#define BUFFER_GIGANTE 65536
#define PACKET_SIZE 2
#define UTF8CHARSIZE 8
#define MAX_SEQ (16-1)
#define USE_PARAESPERA_ARQ 1
// #define AVAILABLE_UNS_CHARS_PER_MSG (unsigned int)(TAM_MAX_DADOS / (PACKET_SIZE * 8)) // SINCE ONE UNSIGNED CHAR WILL BE CONVERTED TO 8 BITS

// #define AVAILABLE_UNS_CHARS_PER_MSG (unsigned int)(10) // PARA MOSTRAR SEM ERROS
#define AVAILABLE_UNS_CHARS_PER_MSG (unsigned int)(16) // PARA MOSTRAR OS REENVIOS DE MENSAGEM NA MENSAGEM DE TEXTO

#define TAM_BUF 100
#define TAM_MAX_DADOS 1024 // 128 bytes = 1024 bits
#define MARC_INICIO 0x7e
#define BITTOINT(bitRepresentation) (bitRepresentation - '0')
#define INTTOBIT(intBit) (intBit + '0')
#define HANNINGDISTANCE(receivedBit, testedBit, packageSize) (return strncmp(receivedBit, testedBit, packageSize))

extern int sequencia_global;

typedef unsigned char bit;

typedef struct msgT
{
    unsigned int marc_inicio : 8;       // Marcador de início
    unsigned int tam_msg : 11;          // Tamanho da mensagem
    unsigned int sequencia : 4;         // Número da mensagem (até 16)
    unsigned int tipo : 6;              // Tipo da mensagem
    unsigned char dados[TAM_MAX_DADOS]; // Buffer dos dados [64bytes]
    // bit paridade:6;    // Paridade
    unsigned int paridade : 6; // Paridade
    int numero_ack;            // número do ACK
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

typedef struct packet
{
    unsigned int seq_num : 4;
    bit data[TAM_BUF];
} Packet;

/**********************ERROR*****************************************************************************************/
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

unsigned int calcHanningDistance(bit *receivedPacketMessage, bit *correctedPacketMessage, unsigned int packageSize);

/** VITERBI ALGORITHM **/
void updatePathError(tNode *root, unsigned int level, tListNode **auxList, unsigned int height, bit *receivedStepMessage, unsigned int packetSize);

tNode *getMinHanningDistancePathLeaf(tNode *root, unsigned int packetSize);

void cutLeafs(tListNode *head, unsigned int tam);

bit *viterbiAlgorithm(bit *receivedMessage, unsigned int packetSize, unsigned int msgSize);
/**********************END_ERROR*****************************************************************************************/

/**********************GENERATE_MESSAGE**********************************************************************************/
/** TRELLIS ENCODING **/
void trellisShift(bit *trellis, bit newBit);

bit encodedX1(bit *trellis);

bit encodedX2(bit *trellis); 

void trellisEncode(bit *encodedMessage, bit *originalMessage, unsigned int size);

bit calculaParidade(bit *conteudo, unsigned int tam);

/**
 * @brief Função para inicialização da mensagem
 * 
 * @param mensagem - Mensagem que recebe a inicialização
 * @param originalMessage - Mensagem original em binário, que vai efetuar o encode pela treliça
 * @param size - Tamanho da mensagem original
 * @param msgType - Tipo da mensagem
 * @param sequencia - Sequencia atual da mensagem
 */
void initMessage(msgT *mensagem, bit *originalMessage, unsigned int size, typesMessage msgType, int sequencia, int shouldEncode);
/**********************END_GENERATE_MESSAGE**********************************************************************************/

/**********************LIST**************************************************************************************************/
void insertfirst(tNode *element, tListNode **head);

/* Function to delete the entire linked list */
void deleteList(tListNode **head_ref);

bit *getDecodedMessage(tListNode *head, unsigned int decodedMsgSize);

unsigned int listSize(tListNode *head);

void prnList(tListNode *head);
/**********************END_LIST**************************************************************************************************/

/**********************BINARY_TREE*******************************************************************************************/
void copyPathAggregatedMessage(bit *prevPathMess, bit *levelMessage, bit *pathAggMess, unsigned int level, unsigned int packetSize);

tNode *startNode(unsigned int curPathError, unsigned int inputBit, typesState curState, unsigned int level, unsigned int packetSize, tNode *parentNode);

void free_binary_tree(tNode *root);

unsigned int countNodes(tNode *n);

void emordem(tNode *no);

/* Function to print level order traversal a tree*/
void printLevelOrder(tNode *root);

void getNextStep(tNode *root, unsigned int packetSize);

void getNextLeafOnLevel(tNode *root, int level, unsigned int packetSize, unsigned int height);

void getListLeafsHannigPathDistance(tNode *root, int level, unsigned int packetSize, unsigned int height, tNode **minHanningDistPathNode);

/* Print nodes at a current level */
void printCurrentLevel(tNode *root, int level);

void getFullMessageDecoded(tNode *leaf);

unsigned int height(tNode *p);
/**********************END_BINARY_TREE*******************************************************************************************/

/**********************UTILS*****************************************************************************************************/
/**
 * @brief Função para envio da mensagem, em caso de erro no envio retorna 0
 * 
 * @param soquete 
 * @param mensagem 
 * @return int 
 */
int sendMessage(int soquete, msgT *mensagem);

int ConexaoRawSocket(char *device);

void incrementaSequencia();

/**
 * @brief Função para receber a mensagem
 * 
 * @param soquete 
 * @param mensagem - mensagem que vai receber a mensagem
 * @param timeout - Se o timeout esta ligado, sendo que não deve ocorrer timeout antes que receba a mensagem de inicio
 * @param sequencia_atual - Sequencia atual esperada pela função
 *  @return int - 2 = timeout, 1= ok, 0 = erro no recebimento 
 */
int recebe_mensagem(int soquete, msgT *mensagem, int timeout, unsigned int sequencia_atual);

/**
 * @brief Função para que seja efetuado o retorno de ACK/NACK da mensagem recebida
 * 
 * @param isAck - Se eh um ack (1 ou 0)
 * @param soquete - Socket
 * @param sequencia - Sequencia atual para mandar o ack ou nack
 * @return int 
 */
int mandaRetorno(int isAck, int soquete, int sequencia);

/**
 * @brief Open a file and return a pointer to the file
 * @param filename -> Name of the file
 * @param mode -> Mode of opening the file
 * @return FILE Object
 **/
FILE *openFile(unsigned char *filename, char *mode);
/**********************END_UTILS*****************************************************************************************************/
#endif