#ifndef _MSG_GENERATOR_H_
#define _MSG_GENERATOR_H_

#define TAM_MAX_DADOS 256
typedef unsigned char bit;  

extern int sequencia_global;

typedef enum {
    A, //= 00,
    B, //= 01,
    C, //= 10,
    D  //= 11
} typesState;

typedef enum {
    TEXTO=0x01,  //000001, 
    MIDIA=0x10,  //100000,
    ACK=  0x0A,  //001010,
    NACK=0x00,   //000000,
    ERRO=0x1E,   //011110,
    INIT=0x1D,   //011101,
    END=0x0F,    //001111,
    DADOS=0x0D   //001101 
} typesMessage; 

// nack: 0x00
// exto: 0x01
// mídia: 0x10
// ack: 0x0A
// erro: 0x1E
// inicio de transmissão: 0x1D
// fim de transmissão: 0x0F
// dados: 0x0D

typedef struct msgT
{
    bit marc_inicio:8;    //Marcador de início
    bit tam_msg:6;       //Tamanho da mensagem
    bit sequencia:4;      //Número da mensagem (até 16)
    bit tipo:6;           //Tipo da mensagem
    bit *dados;         //Buffer dos dados [64]
    bit paridade:8;       //Paridade
} msgT;

bit calculaParidade(bit *conteudo,unsigned int tam); 
bit * trellisEncode(bit *originalMessage, unsigned int size); 
msgT *initMessage(bit *originalMessage, unsigned int size,typesMessage msgType, unsigned int sequencia); 
#endif