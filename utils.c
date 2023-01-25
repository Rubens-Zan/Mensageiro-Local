#include "utils.h"
#include "cliente.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

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

void printstringasbinary(char* s)
{
    // A small 9 characters buffer we use to perform the conversion
    char output[9];

    // Until the first character pointed by s is not a null character
    // that indicates end of string...
    while (*s)
    {
        // Convert the first character of the string to binary using itoa.
        // Characters in c are just 8 bit integers, at least, in noawdays computers.
        itoa(*s, output, 2); 

        // print out our string and let's write a new line.
        // printf("\n[%s, %d]", output,strlen(output));
        unsigned int paddBits = 8 - strlen(output);
        printf("%s \n", append(output,paddBits)); // append converts to 8 bits to use utf8

        ++s;
    }
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
    wint_t char_code; 

    while (1){
        
        printf("Digite o comando\n");
        char_code = get_wch();
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
    char char_code; 
    unsigned char buffer_c[100];
    unsigned int currentBufferPosition = 0;
    setlocale(LC_ALL, "");

    while (1){
        printf("\n state_create_message");
        // char_code = getch();
        char_code = get_wch();
        // <esc>: Sai do modo de inserção de uma mensagem.
        if ( (int)(char_code) == ESC){
            printf("INSERT\n");
            client->estado = INICIO;
            return;
        }else if((int)(char_code) == ENTER) {
            client->estado = ENVIA_MENSAGEM; 
            printf("MENSAGEM A SER ENVIADA %s", buffer_c); 
            printf("\n"); 
            printstringasbinary(buffer_c); 
            client->message = initMessage(buffer_c,currentBufferPosition);
            return;
        }else {
            buffer_c[currentBufferPosition] = char_code; 
            ++currentBufferPosition; 
            printf("msg %s    \n", buffer_c); 
        }
        printf("%d", char_code);

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