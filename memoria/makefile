
CC=gcc
DEPS=memo.h
RM=rm -f
CFLAGS= -lFunciones -lcommons -pthread -g3

#Compilar MEMORIA

all: Debug/memoria

Debug/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/memoria: DebugDir  Debug/memo.o 
	$(CC)  -g -fPIC -Wall -pthread  -fmessage-length=0 -o  Debug/"memoria" Debug/memo.o -lFunciones -lcommons

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/memo.o

.PHONY: clean all
