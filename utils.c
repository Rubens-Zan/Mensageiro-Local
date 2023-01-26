#include "utils.h"
#include "cliente.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <unistd.h>

#define ESC 27
#define ENTER 13

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

bit * printstringasbinary(unsigned int* s, unsigned int tam)
{
    // A small 9 characters buffer we use to perform the conversion
    unsigned char output[9];
    unsigned int curPos = 0;
    bit *myBinaryMessage = (bit *)malloc((8 * tam) * sizeof(bit)); 
    // Until the first character pointed by s is not a null character
    // that indicates end of string...
    while (*s)
    {
        itoa(*s, output, 2); 
        // TODO: ADD FUNCTION TO CONVERT THE UNSIGNED INT TO BINARY INSTEAD 
        // itoa nao funciona em unix
        
        // transforma caracteres de 6 ou 7 bits em 8, adicionando padding de 0,para transformar em utf-8
        int paddBits = 8 - strlen(output);
        if(paddBits > 0){
            strcpy(output,append(output,paddBits)); 
        }
        printf("%s\n ",output);
        strncpy(output, &myBinaryMessage[curPos], 8);
        curPos+=8;
        // TODO: DEPOIS DA CONVERSAO DE DECIMAL PARA BINARIO VERIFICAR A COPIA 
        printf("curPos %s %d %s\n",output,curPos,myBinaryMessage);
        // myBinaryMessage[curPos]='\0'; 
        ++s;
    }
    printf("\nMENSAGEM BIN: ");
    for (int i=0;i < 8 * tam;++i){
        printf("%c",myBinaryMessage[i]);
    }
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

            printf("COMANDO %s\n", buffer_c); 

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
            
        }

    }

}

void state_create_message(tCliente *client){
    unsigned int char_code; 
    unsigned int buffer_c[100];
    unsigned int currentBufferPosition = 0;

    while (1){
        printf("\n state_create_message");
        char_code = getch();
        // <esc>: Sai do modo de inserção de uma mensagem.
        if ( (int)(char_code) == ESC){
            printf("INSERT\n");
            client->estado = INICIO;
            return;
        }else if((int)(char_code) == ENTER) {
            client->estado = ENVIA_MENSAGEM; 
            printf("MENSAGEM A SER ENVIADA: \n"); 
            for (unsigned int i=0; i<currentBufferPosition;++i)
                printf("%d ", buffer_c[i]); 
                
            printf("\n"); 
            bit *myBinaryMsg = printstringasbinary(buffer_c, currentBufferPosition); 
            client->message = initMessage(myBinaryMsg,currentBufferPosition);
            return;
        }else {
            buffer_c[currentBufferPosition] = char_code; 
            printf("msg %c    \n", buffer_c[currentBufferPosition]); 
            ++currentBufferPosition; 
            buffer_c[currentBufferPosition]= '\0'; 
        }
        printf("--%d--", char_code);

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