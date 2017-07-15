#include "CapaFS.h"

t_list* tablaGlobalArchivos;
t_dictionary* tablasProcesos;

int agregarTablaGlobal(char* path){
	//Si es la primera vez que se entra se crea la lista
	if(tablaGlobalArchivos==NULL){
		tablaGlobalArchivos = list_create();
	}
	//Busco si ya fue abierto
	int i;
	for (i = 0; i < list_size(tablaGlobalArchivos); ++i) {
		globalFD_t* globalFD = list_get(tablaGlobalArchivos, i);
		if(globalFD->path==path){
			globalFD->cantProcesos++;
			return i;
		}
	}
	//Si no fue abierto antes creo globalFD y lo agrego al final de la lista
	globalFD_t* globalFD_t = malloc(sizeof(globalFD_t));
	globalFD_t->path=path;
	globalFD_t->cantProcesos=1;
	list_add(tablaGlobalArchivos,globalFD_t);
	return list_size(tablaGlobalArchivos)-1;
}

void quitarTablaGlobal(FD_t* fileDescriptor){
	globalFD_t fd = list_get(tablaGlobalArchivos,fileDescriptor->indiceTablaGlobal);
	fd.cantProcesos--;
}

FD_t* agregarTablaProceso(int Pid, int indiceTablaGlobal, char* permisos){
	//Si es la primera vez que se entra se crea el diccionario
	if(tablasProcesos==NULL){
		tablasProcesos = dictionary_create();
	}
	//Busco si ya existe la tabla de archivos para el proceso y si no existe la creo
	t_list* tablaProceso = dictionary_get(tablasProcesos, string_itoa(Pid));
	if(tablaProceso==NULL){
		tablaProceso = list_create();
		dictionary_put(tablasProcesos, string_itoa(Pid), tablaProceso);
	}
	FD_t* fileDescriptor = malloc(sizeof(FD_t));
	fileDescriptor->indiceTablaGlobal = indiceTablaGlobal;
	fileDescriptor->permisos = string_duplicate(permisos);
	list_add(tablaProceso, fileDescriptor);
	return fileDescriptor;
}

void quitarTablaProceso(int Pid, FD_t* fileDescriptor){
	t_list* tablaProceso = dictionary_get(tablasProcesos, string_itoa(Pid));
	int i;
	for (i = 0; i < list_size(tablaProceso); ++i) {
		FD_t* fd = list_get(tablaProceso,i);
		if(fileDescriptor->indiceTablaGlobal == fd->indiceTablaGlobal){
			list_remove(tablaProceso,i);
			free(fd);
			break;
		}
	}
}

void abrirArchivo(int Pid, char* path, char* permisos){
	int indiceTablaGlobal = agregarTablaGlobal(path);
	FD_t* fileDescriptor = agregarTablaProceso(Pid, indiceTablaGlobal, permisos);
	//TODO: Enviar al FS la peticion de apertura de archivo
	//TODO: Enviar filedescriptor al CPU
}

void leerArchivo(FD_t* fileDescriptor, int offset, int tamanio){
	if(string_contains(fileDescriptor->permisos,"r")==NULL){
		//TODO Dar Error de permisos y terminar el proceso
		return;
	}
	globalFD_t* globalFD = list_get(tablaGlobalArchivos, fileDescriptor->indiceTablaGlobal);
	//TODO: Enviar peticion al FS y enviar respuesta a la CPU
}

void escribirArchivo(FD_t* fileDescriptor, int offset, int tamanio, char* datos){
	if(string_contains(fileDescriptor->permisos,"w")==NULL){
		//TODO Dar Error de permisos y terminar el proceso
		return;
	}
	globalFD_t* globalFD = list_get(tablaGlobalArchivos, fileDescriptor->indiceTablaGlobal);
	//TODO: Enviar peticion al FS y enviar respuesta a la CPU
}

void cerrarArchivo(int Pid, FD_t* fileDescriptor){
	quitarTablaGlobal(fileDescriptor);
	quitarTablaProceso(Pid, fileDescriptor);

}
