#ifndef CapaFS_h
#define CapaFS_h

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

typedef struct{
	char* permisos;
	int offset;
	int indiceTablaGlobal;
}FD_t;

typedef struct{
	char* path;
	int cantProcesos;
}globalFD_t;

void abrirArchivo(int socketCpu, int socketFS);
void leerArchivo(int socketCpu, int socketFS);
void escribirArchivo(int socketCpu, int socketFS);
void cerrarArchivo(int socketCpu, int socketFS);
void borrarArchivo(int socketCPU, int socketFS);
void moverCursor(int socketCPU, int socketFS);
void liberarRecursosFS(int pid);
void obtenerTablaProceso(int pid);

#endif
