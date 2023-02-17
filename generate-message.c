#include "generate-message.h"
#include "error-handle.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** TRELLIS ENCODING **/
void trellisShift(bit *trellis, bit newBit)
{
    trellis[2] = trellis[1];
    trellis[1] = trellis[0];
    trellis[0] = newBit;
}

bit encodedX1(bit *trellis)
{
    bit x1;
    x1 = INTTOBIT((BITTOINT(trellis[0]) + BITTOINT(trellis[1]) + BITTOINT(trellis[2])) % 2); // x1 = m xor m1 xor m2
    return x1;
}

bit encodedX2(bit *trellis)
{
    bit x2;
    x2 = INTTOBIT((BITTOINT(trellis[0]) + BITTOINT(trellis[2])) % 2); // x2 = m xor m2
    return x2;
}

void trellisEncode(bit *encodedMessage, bit *originalMessage, unsigned int size)
{
    bit trellis[4] = "000";
    trellis[3] = '\0';

    unsigned int encodCounter = 0;
    for (unsigned int i = 0; i < size; ++i)
    {
        trellisShift(trellis, originalMessage[i]);
        encodedMessage[encodCounter] = encodedX1(trellis);
        encodedMessage[encodCounter + 1] = encodedX2(trellis);
        encodCounter += 2;
    }
    encodedMessage[encodCounter] = '\0';
}

bit calculaParidade(bit *conteudo, unsigned int tam)
{
    // Faz um XOR dos bits para cada byte da mensagem.
    bit aux = conteudo[0];
    for (int i = 1; i < tam; i++)
    {
        aux = aux ^ conteudo[i];
    }

    return aux;
}

void initMessage(msgT *mensagem, bit *originalMessage, unsigned int size, typesMessage msgType, unsigned int sequencia)
{
    trellisEncode(mensagem->dados, originalMessage, size);
    mensagem->paridade = calculaParidade(mensagem->dados, size);
    mensagem->marc_inicio = MARC_INICIO;
    mensagem->tipo = msgType;
    mensagem->tam_msg = strlen(mensagem->dados); // duplica por causa da modulção da trelica
    mensagem->sequencia = sequencia_global;
}
