#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

typedef struct{
	char* permisos;
	int indiceTablaGlobal;
}FD_t;

typedef struct{
	char* path;
	int cantProcesos;
}globalFD_t;
