#ifndef cpu_h
#define cpu_h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "kernel.h"
#include "estructurasCompartidas.h"

typedef struct{
	char* permisos;
	int offset;
	int indiceTablaGlobal;
}FD_t;

typedef struct{
	char* path;
	int cantProcesos;
}globalFD_t;

#endif
