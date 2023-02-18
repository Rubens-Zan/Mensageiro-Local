# Makefile compilacao programa processamento de imagens
# Para efetuar a compilação digite make all ou make
# Para remover os arquivos temporários digite make clean
# Para remover os arquivos temporários e o arquivo executável digite make purge

CFLAGS =
#-lncurses

MODULOS = client_lib \
	server_lib \
	binary-tree \
	list \
	generate-message \
	error-handle \
	ConexaoRawSocket


OBJETOS = $(addsuffix .o,$(MODULOS)) 

ALVOS =  servidor cliente 

.PHONY : all clean purge debug

all : $(ALVOS)

debug: CFLAGS += -DDEBUG
debug: $(PROG)

servidor: $(OBJETOS)
	gcc -o $@ $(OBJETOS) servidor.c $(CFLAGS)
	
cliente: $(OBJETOS)
	gcc -o $@ $(OBJETOS) cliente.c $(CFLAGS)


clean : 
	$(RM) $(OBJETOS)

purge:  clean
	$(RM) $(OBJETOS) cliente servidor
	@rm -f *.o core a.out

