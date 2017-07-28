
CC=gcc
DEPS=fileSystem.h
RM=rm -f
CFLAGS= -lFunciones -lcommons -pthread -g3

#Compilar FILESYSTEM

all: Debug/fileSystem

Debug/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/fileSystem: DebugDir  Debug/fileSystem.o 
	$(CC)  -g -fPIC -Wall -pthread  -fmessage-length=0 -o  Debug/"fileSystem" Debug/fileSystem.o -lFunciones -lcommons 

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/fileSystem.o


.PHONY: clean all
