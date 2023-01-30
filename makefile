CC     = gcc -g
CFLAGS =
LFLAGS = -lncurses

PROG = cliente
OBJS = utils.o \
	binary-tree.o \
	list.o \
	generate-message.o \
	error-handle.o 


.PHONY:  clean purge all

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $<

$(PROG):  $(OBJS) $(PROG).o
	$(CC) -o $@ $^ $(LFLAGS)

clean:
	@rm -f *~ *.bak

purge:  clean
	@rm -f *.o core a.out
	@rm -f $(PROG)

