

CC=gcc
DEPS=kernel.h
RM=rm -f
CFLAGS= -lfunciones -lcommons -pthread -g3

#Compilar KERNEL

all: Debug/kernel

Debug/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/kernel: DebugDir  Debug/kernel.o  Debug/CapaFS.o Debug/CapaMemoria.o Debug/gestionDeProcesos.o Debug/primitivasKernel.o Debug/funcionesKernel.o
	$(CC)  -g -fPIC -Wall -pthread  -fmessage-length=0 -o  Debug/"kernel" Debug/kernel.o Debug/CapaFS.o Debug/CapaMemoria.o Debug/gestionDeProcesos.o Debug/primitivasKernel.o Debug/kernel.o -lfunciones -lcommons -lparser-ansisop

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/kernel.o
	$(RM) Debug/CapaFS.o
	$(RM) Debug/CapaMemoria.o 
	$(RM) Debug/gestionDeProcesos.o
	$(RM) Debug/primitivasKernel.o
	$(RM) Debug/funcionesKernel.o

.PHONY: clean all
