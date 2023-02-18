# Makefile compilacao programa processamento de imagens
# Para efetuar a compilação digite make all ou make
# Para remover os arquivos temporários digite make clean
# Para remover os arquivos temporários e o arquivo executável digite make purge

CFLAGS =
#-lncurses

MODULOS = error_handle \
	client_lib \
	server_lib

OBJETOS = $(addsuffix .o,$(MODULOS)) 

ALVOS =  client server

.PHONY : all clean purge debug

all : $(ALVOS)

debug: CFLAGS += -DDEBUG
debug: $(PROG)

client: $(OBJETOS)
	gcc -o $@ $(OBJETOS) client.c $(CFLAGS)

server: $(OBJETOS)
	gcc -o $@ $(OBJETOS) server.c $(CFLAGS)

clean : 
	$(RM) $(OBJETOS)

purge:  clean
	$(RM) $(OBJETOS) client server
	@rm -f *.o core a.out

