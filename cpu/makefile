

CC=gcc
DEPS=cpu.h
RM=rm -f
CFLAGS= -lfunciones -lcommons -pthread -lparser-ansisop -g3

#Compilar CPU

all: Debug/cpu

Debug/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/cpu: DebugDir  Debug/cpu.o Debug/primitivas.o 
	$(CC)  -g -fPIC -Wall -pthread  -fmessage-length=0 -o  Debug/"cpu" Debug/cpu.o Debug/primitivas.o -lFunciones -lcommons -lparser-ansisop

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/cpu.o
	$(RM) Debug/primitivas.o	

.PHONY: clean all
