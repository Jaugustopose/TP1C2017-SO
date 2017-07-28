
CC=gcc
DEPS=consola.h
RM=rm -f
CFLAGS= -lFunciones -lcommons -pthread -g3

#Compilar CONSOLA

all: Debug/consola

Debug/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/consola: DebugDir  Debug/consola.o 
	$(CC)  -g -fPIC -Wall -pthread  -fmessage-length=0 -o  Debug/"consola" Debug/consola.o -lFunciones -lcommons

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/consola.o

.PHONY: clean all