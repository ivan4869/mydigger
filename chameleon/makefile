
CC = gcc
#CFLAGS = -DDebug_parsingInput
#CFLAGS = -DDebug_readData
#CFLAGS = -g -DDebug_initialize
#CFLAGS = -DDebug
#CFLAGS = -g -DDebug_establish_hyperGraph -Wall
CFLAGS = -g -DDebug_phase2 -Wall 
#CFLAGS = 
CLIBS  =  -L. -lhmetis -lm
#LDFLAGS = -lpthread -L. -lsbt -lm

all: chameleon


chameleon : chameleon.c
	$(CC) $(CFLAGS) chameleon.c  -o chameleon  $(CLIBS)

#clean useless files
clean:
	rm -f chameleon core *.o *~


