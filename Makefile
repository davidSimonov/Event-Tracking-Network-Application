# David Simonov
# das892
# 11301273

CFLAGS = -g -I.
CPPFLAGS = -std=gnu90 -Wall -Wextra -pedantic -I.

LIBS = -L. -lcalendar$(TS) -llist$(TS)
LIBS_A = calendar$(TS).a list$(TS).a
LIBS_O = list_adders$(TS).o list_movers$(TS).o list_removers$(TS).o
SERVERS = server_select$(TS) server_proc$(TS) server_thread$(TS)

# Default architecture
ARCH ?= x86

ifeq ($(ARCH),x86)
    CC = gcc
    TS = _x86
endif

ifeq ($(ARCH),arm)
    CC = arm-linux-gnueabihf-gcc-10
    TS = _arm
endif

ifeq ($(ARCH),ppc)
    CC = powerpc-linux-gnu-gcc-10
    TS = _ppc
endif

all: client$(TS) $(SERVERS)


# Client/Server
client$(TS): client$(TS).o
	${CC} -o client$(TS) client$(TS).o

client$(TS).o: client.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c client.c -o client$(TS).o

server_select$(TS): server_select$(TS).o utils$(TS).o $(LIBS_A)
	${CC} -o server_select$(TS) server_select$(TS).o utils$(TS).o $(LIBS)

server_select$(TS).o: server_select.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c server_select.c -o server_select$(TS).o

server_proc$(TS): server_proc$(TS).o utils$(TS).o $(LIBS_A)
	${CC} -o server_proc$(TS) server_proc$(TS).o utils$(TS).o $(LIBS)

server_proc$(TS).o: server_proc.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c server_proc.c -o server_proc$(TS).o

server_thread$(TS): server_thread$(TS).o utils$(TS).o $(LIBS_A)
	${CC} -o server_thread$(TS) server_thread$(TS).o utils$(TS).o $(LIBS)

server_thread$(TS).o: server_thread.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c server_thread.c -o server_thread$(TS).o

utils$(TS).o: utils.c utils.h
	${CC} ${CFLAGS} ${CPPFLAGS} -c utils.c -o utils$(TS).o


# Libraries
calendar$(TS).a:
	${CC} -c $(CFLAGS) $(CPPFLAGS) calendar.c -o calendar$(TS).o
	ar -r -c -s libcalendar$(TS).a calendar$(TS).o

list$(TS).a:
	${CC} -c $(CFLAGS) $(CPPFLAGS) list_adders.c -o list_adders$(TS).o
	${CC} -c $(CFLAGS) $(CPPFLAGS) list_movers.c -o list_movers$(TS).o
	${CC} -c $(CFLAGS) $(CPPFLAGS) list_removers.c -o list_removers$(TS).o
	ar -r -c -s liblist$(TS).a $(LIBS_O)

# Cleanup
clean:
	rm -f *.o *.a client$(TS) $(SERVERS)
