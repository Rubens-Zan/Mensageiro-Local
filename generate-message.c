#include "generate-message.h"
#include "error-handle.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MARC_INICIO 0x7e

/** TRELLIS ENCODING **/
void trellisShift(bit *trellis, bit newBit){
    trellis[2] = trellis[1]; 
    trellis[1] = trellis[0]; 
    trellis[0] = newBit; 
}

bit encodedX1(bit *trellis){
    bit x1; 
    x1 = INTTOBIT(( BITTOINT(trellis[0]) +BITTOINT(trellis[1]) +BITTOINT(trellis[2])) % 2);  // x1 = m xor m1 xor m2 
    return x1; 
}

bit encodedX2(bit *trellis){
    bit x2; 
    x2 = INTTOBIT((BITTOINT(trellis[0]) + BITTOINT(trellis[2]) ) % 2);  // x2 = m xor m2 
    return x2; 
}

bit * trellisEncode(bit *originalMessage, unsigned int size){
    bit trellis[4] = "000"; 
    trellis[3] = '\0';

    bit *encodedMessage = (bit *) malloc(sizeof(bit) * (size * 2)); // since message will be encoded to x1 and x2, it will be double the message
    
    unsigned int encodCounter = 0;
    for (unsigned int i=0;i < size;++i){
        trellisShift(trellis, originalMessage[i]);
        encodedMessage[encodCounter] = encodedX1(trellis); 
        encodedMessage[encodCounter+1]= encodedX2(trellis);
        encodCounter+=2; 
    }
    encodedMessage[encodCounter] = '\0';

    return encodedMessage; 
}

bit calculaParidade(bit *conteudo,unsigned int tam){
   // Faz um XOR dos bits para cada byte da mensagem.
    bit aux = conteudo[0];
    for (int i = 1; i < tam; i++){
        aux = aux ^ conteudo[i];
	}

    return aux;
}

msgT *initMessage(bit *originalMessage, unsigned int size,typesMessage msgType, unsigned int sequencia){ 
    msgT *message = (msgT *)malloc(sizeof(msgT));
    
    message->dados = trellisEncode(originalMessage, size); 
    message->paridade = calculaParidade(message->dados, size);
    message->marc_inicio = MARC_INICIO;
    message->tipo = msgType; 
    message->tam_msg = strlen(message->dados); // duplica por causa da modulacao da trelica 
    message->sequencia = sequencia_global; 
    
    return message; 
}
/****/


