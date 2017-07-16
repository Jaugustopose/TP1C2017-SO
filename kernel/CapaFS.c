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
	globalFD_t* fd = list_get(tablaGlobalArchivos,fileDescriptor->indiceTablaGlobal);
	fd->cantProcesos--;
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

void crearArchivo(int Pid, char* path, char* permisos, int socketCpu){
	//Envio mensaje al FS
	int codOperacion = accionCrearArchivo;
	int pathSize = string_length(path);
	// Tamanio paquete = codOperacion + pathSize + path
	int packetSize = pathSize + sizeof(int)*2;
	void *buffer = malloc(packetSize);
	memcpy(buffer,&codOperacion,sizeof(int));
	memcpy(buffer + sizeof(int),&pathSize,sizeof(int));
	memcpy(buffer + sizeof(int)*2,path,pathSize);
	send(socketFS, buffer, packetSize,0);
	free(buffer);
	//Recibo respuesta
	int res;
	recv(socketFS, &res, sizeof(res), 0);
	if(res==0){
		int indiceTablaGlobal = agregarTablaGlobal(path);
		FD_t* fileDescriptor = agregarTablaProceso(Pid, indiceTablaGlobal, permisos);
		//TODO: Enviar filedescriptor a CPU
	}else{
		//TODO: -1 no hay bloques libres -2 no se pudo crear el archivo
	}
}

void abrirArchivo(){
	int Pid;
	char* path;
	char* permisos;
	int socketCpu;
	//TODO: Recibir informacion desde el CPU
	//Envio mensaje al FS
	int codOperacion = accionAbrirArchivo;
	int pathSize = string_length(path);
	// Tamanio paquete = codOperacion + pathSize + path
	int packetSize = pathSize + sizeof(int)*2;
	void *buffer = malloc(packetSize);
	memcpy(buffer,&codOperacion,sizeof(int));
	memcpy(buffer + sizeof(int),&pathSize,sizeof(int));
	memcpy(buffer + sizeof(int)*2,path,pathSize);
	send(socketFS, buffer, packetSize,0);
	free(buffer);
	//Recibo respuesta
	int res;
	recv(socketFS, &res, sizeof(res), 0);
	//Analizar respuesta y enviar a CPU
	if(res==0){
		int indiceTablaGlobal = agregarTablaGlobal(path);
		FD_t* fileDescriptor = agregarTablaProceso(Pid, indiceTablaGlobal, permisos);
		//TODO: Enviar filedescriptor a CPU
	}else{
		if(res==-1 && string_contains(permisos, "c")){
			crearArchivo(Pid, path, permisos, socketCpu);
			return;
		}else{
			//TODO: Error Archivo no existe
			return;
		}
		//TODO: Error el archivo esta corrupto
	}
}

void leerArchivo(){
	FD_t* fileDescriptor;
	int offset;
	int tamanio;
	//TODO: Recibir informacion desde el CPU
	if(string_contains(fileDescriptor->permisos,"r")==NULL){
		//TODO Dar Error de permisos y terminar el proceso
		return;
	}
	globalFD_t* globalFD = list_get(tablaGlobalArchivos, fileDescriptor->indiceTablaGlobal);
	//Envio mensaje al FS
	int codOperacion = accionObtenerDatosArchivo;
	int pathSize = string_length(globalFD->path);
	// Tamanio paquete = codOperacion + pathSize + path + offset + tamanio
	int packetSize = pathSize + sizeof(int)*4;
	void *buffer = malloc(packetSize);
	memcpy(buffer, &codOperacion, sizeof(int));
	memcpy(buffer + sizeof(int), &pathSize, sizeof(int));
	memcpy(buffer + sizeof(int)*2, globalFD->path, pathSize);
	memcpy(buffer + sizeof(int)*2 + pathSize, &offset, sizeof(int));
	memcpy(buffer + sizeof(int)*3 + pathSize, &tamanio, sizeof(int));
	send(socketFS, buffer, packetSize,0);
	free(buffer);
	//Recibo respuesta
	int res;
	recv(socketFS, &res, sizeof(res), 0);
	if(res==0){
		char* datos = malloc(tamanio);
		recv(socketFS, datos, tamanio, 0);
		//TODO: Enviar respuesta a la CPU
	}else{
		//TODO: Dar error
	}
}

void escribirArchivo(){
	FD_t* fileDescriptor;
	int offset;
	int tamanio;
	char* datos;
	//TODO: Recibir informacion desde el CPU
	if(string_contains(fileDescriptor->permisos,"w")==NULL){
		//TODO Dar Error de permisos y terminar el proceso
		return;
	}
	globalFD_t* globalFD = list_get(tablaGlobalArchivos, fileDescriptor->indiceTablaGlobal);
	//Envio mensaje al FS
	int codOperacion = accionEscribir;
	int pathSize = string_length(globalFD->path);
	// Tamanio paquete = codOperacion + pathSize + path + offset + tamanio + datos
	int packetSize = pathSize + sizeof(int)*4 + tamanio;
	void *buffer = malloc(packetSize);
	memcpy(buffer, &codOperacion, sizeof(int));
	memcpy(buffer + sizeof(int), &pathSize, sizeof(int));
	memcpy(buffer + sizeof(int)*2, globalFD->path, pathSize);
	memcpy(buffer + sizeof(int)*2 + pathSize, &offset, sizeof(int));
	memcpy(buffer + sizeof(int)*3 + pathSize, &tamanio, sizeof(int));
	memcpy(buffer + sizeof(int)*4 + pathSize, datos, tamanio);
	send(socketFS, buffer, packetSize,0);
	free(buffer);
	//Recibo respuesta
	int res;
	recv(socketFS, &res, sizeof(res), 0);
	//Analizar respuesta y enviar a CPU
	if(res==0){
		//TODO: OK
	}else{
		//TODO: -1 error al escribir el archivo
	}
	//TODO: Enviar peticion al FS y enviar respuesta a la CPU
}

void cerrarArchivo(){
	FD_t* fileDescriptor;
	int Pid;
	//TODO: Recibir informacion desde el CPU
	quitarTablaGlobal(fileDescriptor);
	quitarTablaProceso(Pid, fileDescriptor);
}

void borrarArchivo(){
	FD_t* fileDescriptor;
	int Pid;
	//TODO: Recibir informacion desde el CPU
	globalFD_t* globalFD = list_get(tablaGlobalArchivos, Pid);
	if(globalFD->cantProcesos>1){
		//Error: No se puede borrar el archivo porque alguien mas lo tienen abierto
		//TODO: Enviar respuesta a CPU y terminar el proceso.
	}else{
		quitarTablaGlobal(fileDescriptor);
		quitarTablaProceso(Pid, fileDescriptor);
		//Envio mensaje al FS
		int codOperacion = accionBorrarArchivo;
		int pathSize = string_length(globalFD->path);
		// Tamanio paquete = codOperacion + pathSize + path
		int packetSize = pathSize + sizeof(int)*2;
		void *buffer = malloc(packetSize);
		memcpy(buffer,&codOperacion,sizeof(int));
		memcpy(buffer + sizeof(int),&pathSize,sizeof(int));
		memcpy(buffer + sizeof(int)*2,globalFD->path,pathSize);
		send(socketFS, buffer, packetSize,0);
		free(buffer);
		//Recibo respuesta
		int res;
		recv(socketFS, &res, sizeof(res), 0);
		if(res==0){
			//OK
		}else{
			//TODO: Error al borrar el archivo
		}
	}
}
