#include "utils.h"
#include "cliente.h"
#include "error-handle-lib/error-handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <conio.h>

#define ESC 27
#define ENTER 13

bit *convertToBin(unsigned int num, unsigned int bitsSize){
    bit *convertedNumb = (bit *) malloc ((bitsSize+1) * sizeof(bit));
    int a[10],n,i,j;  
    n = num; 
    j=0;
    for (unsigned int i=0;i<bitsSize;++i){
        a[i] = 0;
    }
    convertedNumb[bitsSize] = '\0'; 
    for(i=0;n>0;i++){    
        a[i]=n%2;    
        n=n/2;    
    }    
    for(i=bitsSize-1;i >=0;i--)    
    {    
        convertedNumb[j] = a[i] + '0';
        ++j;    
    }    
    return convertedNumb; 
}

char *append( char *str, unsigned int shifQt)
{
    size_t len = strlen(str);

    for (unsigned int i=0;i< shifQt;++i){
        memmove(str + 1, str, ++len);
        *str = 1;
        str[0] = '0'; 
    }

    return str;
}

bit * getStringAsBinary(unsigned int* s, unsigned int tam)
{
    // A small 9 characters buffer we use to perform the conversion
    unsigned char output[9];
    unsigned int curPos = 0;
    bit *myBinaryMessage = (bit *)malloc((8 * tam) * sizeof(bit)); 
    for (unsigned int i=0;i < tam;++i){
        bit* myConvertedNumb = convertToBin(s[i], 8);
        strncpy(&myBinaryMessage[curPos], myConvertedNumb, 8);
        curPos+=8;
    }
    myBinaryMessage[curPos] = '\0';
    // printf("binaria: %s\n", myBinaryMessage);
    return myBinaryMessage;
}

/**
 * ESTADOS DO CLIENTE
 */

void state_init(tCliente *client){
    // i: Inicia a criação de uma mensagem. Enter para enviar.
    // <esc>: Sai do modo de inserção de uma mensagem.
    // :q<enter>: Sai do programa.
    // :send x<enter>: Envia arquivo x.
    char buffer_c[100];
    char *filename_c;
    char char_code; 

    while (1){
        
        printf("Digite o comando\n");
        char_code = getch();
        if (char_code == 'i'){
            printf("INSERT\n");
            client->estado = CRIACAO_MENSAGEM;
            return;
        }else if (char_code == ':') {
            printf(":"); 
            scanf("%63[^\n]", buffer_c);
            getchar();
            // printf("COMANDO %s\n", buffer_c);

            if (strncmp(buffer_c, "q",1) == 0){
                printf("QUITE\n");
                client->estado = FIM_PROGRAMA;
                return;
            }else if(strncmp(buffer_c, "send", 4) == 0){
                filename_c = strtok(buffer_c," ");
                filename_c = strtok(NULL, " "); 
                client->estado = ENVIA_ARQUIVO;
                client->fileName = filename_c; 
                return;
            }
        }else{
            printf("COMANDO INVALIDO \n");
        }

    }

}

void state_create_message(tCliente *client){
    unsigned int char_code; 
    unsigned int buffer_c[100];
    unsigned int currentBufferPosition = 0;

    while (1){
        char_code = getch();
        // <esc>: Sai do modo de inserção de uma mensagem.
        if ( (int)(char_code) == ESC){
            client->estado = INICIO;
            return;
        }else if((int)(char_code) == ENTER) {
            client->estado = ENVIA_MENSAGEM; 
            printf("MENSAGEM A SER ENVIADA: \n"); 
            for (unsigned int i=0; i<currentBufferPosition;++i)
                printf("%d ", buffer_c[i]); 
                
            printf("\n"); 
            bit *myBinaryMsg = getStringAsBinary(buffer_c, currentBufferPosition); 
            client->message = initMessage(myBinaryMsg,currentBufferPosition*8, TEXTO);
            // printf("%s \n",viterbiAlgorithm(client->message->dados, 2,96));
            // printf("%s %d %d",client->message->dados, 2,48 );
            
            return;
        }else {
            buffer_c[currentBufferPosition] = char_code; 
            printf("%c", buffer_c[currentBufferPosition]); 
            ++currentBufferPosition; 
            buffer_c[currentBufferPosition]= '\0'; 
        }

    }
}

void state_send_message(tCliente *client){

}

void state_send_file(tCliente *client){
    while (1){
        printf("\n SEND state");
    }
}

void state_end(tCliente *client){
    while (1){
        printf("\n state_end state");
        
    }  
}

/**FIM ESTADOS DO CLIENTE*/