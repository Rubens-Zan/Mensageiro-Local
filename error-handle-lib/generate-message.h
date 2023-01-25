#ifndef _MSG_GENERATOR_H_
#define _MSG_GENERATOR_H_

typedef unsigned char bit;    

typedef enum {
    A, //= 00,
    B, //= 01,
    C, //= 10,
    D  //= 11
} typesState;

typedef enum {
    TEXTO=0x01,
    MIDIA=0x10,
    ACK=0x0A,
    NACK=0x00,
    ERRO=0x1E,
    INIT=0x1D,
    END=0x0F,
    DADOS=0x0D
} typesMessage; 

// exto: 0x01
// mídia: 0x10
// ack: 0x0A
// nack: 0x00
// erro: 0x1E
// inicio de transmissão: 0x1D
// fim de transmissão: 0x0F
// dados: 0x0D

typedef struct msgT
{
  
    bit marc_inicio : 8;    //Marcador de início
    bit tam_msg : 6;       //Tamanho da mensagem
    bit sequencia : 4;      //Número da mensagem (até 16)
    bit tipo : 6;           //Tipo da mensagem
    bit *dados;         //Buffer dos dados [64]
    bit paridade : 8;       //Paridade
} msgT;

bit calculaParidade(bit *conteudo,unsigned int tam); 
bit * trellisEncode(bit *originalMessage, unsigned int size); 
msgT *initMessage(bit *originalMessage, unsigned int size); 

#endif