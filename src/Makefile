#
# Makefile for TART library (libtart.a)
#

INC=	../inc
LIB=	../lib

LIBTART=$(LIB)/libtart.a
INCS=	$(INC)/tart.h \
		$(INC)/actor.h \
		$(INC)/expr.h \
		$(INC)/number.h \
		$(INC)/string.h \
		$(INC)/action.h \
		$(INC)/universe.h
OBJS=	actor.o \
		expr.o \
		number.o \
		string.o \
		action.o \
		universe.o
#PROGS=	tart tart_fx
PROGS=	tart

CFLAGS=	-I$(INC)
#CFLAGS=	-ansi -pedantic -Wall -Wextra -ffreestanding -fno-stack-protector -I$(INC)

all: $(LIBTART) $(PROGS)

test: all
	./tart
#	./tart_fx

$(LIBTART): $(OBJS)
	$(AR) rv $@ $(OBJS)

$(OBJS): $(INCS)

tart: tart.o $(LIBTART)
	$(CC) $(CFLAGS) -o $@ tart.o $(LIBTART)

tart_fx: tart_fx.o $(LIBTART)
	$(CC) $(CFLAGS) -o $@ tart_fx.o $(LIBTART)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -rf $(LIB)
	mkdir $(LIB)
	rm -f $(PROGS)
	rm -f *.o *~ *.stackdump
