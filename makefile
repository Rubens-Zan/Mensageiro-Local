# Makefile compilacao programa processamento de imagens
# Para efetuar a compilação digite make all ou make
# Para remover os arquivos temporários digite make clean
# Para remover os arquivos temporários e o arquivo executável digite make purge

CFLAGS = #-Wall -lm
#-lncurses

OBJ_SERVER =  server.o server_lib.o general.o
OBJ_CLIENT =  client.o client_lib.o general.o

.PHONY : all clean purge debug

all : server client
server_s: server
client_s: client

debug: CFLAGS += -DDEBUG
debug: $(PROG)

client: $(OBJ_CLIENT)
	gcc -o client $(CFLAGS) $(OBJ_CLIENT)

server: $(OBJ_SERVER)
	gcc -o server $(CFLAGS) $(OBJ_SERVER) -lm

general.o: ./general/general.c
	gcc $(CFLAGS) -c ./general/general.c

server_lib.o: ./server_lib/server_lib.c
	gcc $(CFLAGS) -c ./server_lib/server_lib.c -lm

server.o: server.c
	gcc $(CFLAGS) -c server.c

client_lib.o: ./client_lib/client_lib.c
	gcc $(CFLAGS) -c ./client_lib/client_lib.c

client.o: client.c
	gcc $(CFLAGS) -c client.c

clean : 
	$(RM) $(OBJ_CLIENT) $(OBJ_SERVER)

purge:  clean
	$(RM) $(OBJ_CLIENT) $(OBJ_SERVER) client server
	@rm -f *.o core a.out

