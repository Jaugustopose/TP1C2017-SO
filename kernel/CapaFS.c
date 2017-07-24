#include "CapaFS.h"

t_list* tablaGlobalArchivos;
t_dictionary* tablasProcesos;

//*********************************Funciones Auxiliares********************

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

int agregarTablaProceso(int pid, int indiceTablaGlobal, char* permisos){
	//Si es la primera vez que se entra se crea el diccionario
	if(tablasProcesos==NULL){
		tablasProcesos = dictionary_create();
	}
	//Busco si ya existe la tabla de archivos para el proceso y si no existe la creo
	t_list* tablaProceso = dictionary_get(tablasProcesos, string_itoa(pid));
	if(tablaProceso==NULL){
		tablaProceso = list_create();
		dictionary_put(tablasProcesos, string_itoa(pid), tablaProceso);
	}
	FD_t* fileDescriptor = malloc(sizeof(FD_t));
	fileDescriptor->indiceTablaGlobal = indiceTablaGlobal;
	fileDescriptor->permisos = string_duplicate(permisos);
	fileDescriptor->offset = 0;
	return list_add(tablaProceso, fileDescriptor)+2;
}

FD_t* obtenerFD(int pid, int fd){
	t_list* tablaProceso = dictionary_get(tablasProcesos, string_itoa(pid));
	return list_get(tablaProceso, fd-2);
}

void quitarTablaProceso(int pid, int fd){
	t_list* tablaProceso = dictionary_get(tablasProcesos, string_itoa(pid));
	FD_t* fileDescriptor = list_get(tablaProceso, fd-2);
	free(fileDescriptor);
	//Reemplazo en vez de eliminar para que no me cambie el indice de los demas
	list_replace(tablaProceso,fd-2,NULL);
}

//*********************************Operaciones FS***************************

void crearArchivo(int Pid, char* path, char* permisos, int socketCpu, int socketFS){

	t_proceso* proceso = buscarProcesoPorPID(Pid);
	proceso->privilegiadas++;

	//Envio mensaje al FS
	int codOperacion = accionCrearArchivo;
	int pathSize = string_length(path)+1;
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
	if(res==1){
		int indiceTablaGlobal = agregarTablaGlobal(path);
		int fd = agregarTablaProceso(Pid, indiceTablaGlobal, permisos);
		send(socketCpu, &fd, sizeof(fd),0);
	}else{
		//TODO: -1 no hay bloques libres -2 no se pudo crear el archivo
	}
}

void abrirArchivo(int socketCpu, int socketFS){
	int pid;
	int tamanioPath;
	int tamanioPermisos;
	//Recibo datos del CPU
	recv(socketCpu, &pid, sizeof(pid), 0);
	recv(socketCpu, &tamanioPath, sizeof(tamanioPath), 0);
	recv(socketCpu, &tamanioPermisos, sizeof(tamanioPermisos), 0);
	char* path = malloc(tamanioPath);
	char* permisos = malloc(tamanioPermisos);
	recv(socketCpu, path, tamanioPath, 0);
	recv(socketCpu, permisos, tamanioPermisos, 0);


	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->privilegiadas++;

	//Envio mensaje al FS
	int codOperacion = accionAbrirArchivo;
	// Tamanio paquete = codOperacion + pathSize + path
	int packetSize = tamanioPath + sizeof(int)*2;
	void *buffer = malloc(packetSize);
	memcpy(buffer,&codOperacion,sizeof(int));
	memcpy(buffer + sizeof(int),&tamanioPath,sizeof(int));
	memcpy(buffer + sizeof(int)*2,path,tamanioPath);
	send(socketFS, buffer, packetSize,0);
	free(buffer);
	//Recibo respuesta
	int res=-1;
	recv(socketFS, &res, sizeof(res), 0);
	//Analizar respuesta y enviar a CPU
	if(res==1){
		int indiceTablaGlobal = agregarTablaGlobal(path);
		int fd = agregarTablaProceso(pid, indiceTablaGlobal, permisos);
		send(socketCpu, &fd, sizeof(fd),0);
	}else{
		// Si no existe el archivo pero tenemos permisos de creacion se lo manda a crear
		if(res==-1 && string_contains(permisos, "c")){
			crearArchivo(pid, path, permisos, socketCpu, socketFS);
			return;
		}else{
			//TODO: Error Archivo no existe
			//ACA METER send(socketCpu, &RES, sizeof(fd),0); y ese res tiene que ser -2
			//Primitiva abrir en CPU lo detecta como -2 => exit code y sale del programa.
			return;
		}
		//TODO: Error el archivo esta corrupto
	}
}

void leerArchivo(int socketCpu, int socketFS){
	int fd;
	int pid;
	int tamanio;
	//Recibo datos del CPU
	recv(socketCpu, &fd, sizeof(fd), 0);
	recv(socketCpu, &pid, sizeof(fd), 0);
	recv(socketCpu, &tamanio, sizeof(tamanio), 0);
	FD_t* fileDescriptor = obtenerFD(pid, fd);

	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->privilegiadas++;

	if(string_contains(fileDescriptor->permisos,"r")==NULL){
		//TODO Dar Error de permisos y terminar el proceso
		//ACA METER send(socketCpu, &RES, sizeof(fd),0); y ese res tiene que ser -2
		//Primitiva abrir en CPU lo detecta como -3 => exit code y sale del programa.
		return;
	}
	globalFD_t* globalFD = list_get(tablaGlobalArchivos, fileDescriptor->indiceTablaGlobal);
	//Envio mensaje al FS
	int codOperacion = accionObtenerDatosArchivo;
	int pathSize = string_length(globalFD->path)+1;
	// Tamanio paquete = codOperacion + pathSize + path + offset + tamanio
	int packetSize = pathSize + sizeof(int)*4;
	void *buffer = malloc(packetSize);
	memcpy(buffer, &codOperacion, sizeof(int));
	memcpy(buffer + sizeof(int), &pathSize, sizeof(int));
	memcpy(buffer + sizeof(int)*2, globalFD->path, pathSize);
	memcpy(buffer + sizeof(int)*2 + pathSize, &fileDescriptor->offset, sizeof(int));
	memcpy(buffer + sizeof(int)*3 + pathSize, &tamanio, sizeof(int));
	send(socketFS, buffer, packetSize,0);
	free(buffer);
	//Recibo respuesta
	int res;
	recv(socketFS, &res, sizeof(res), 0);
	if(res==1){
		void* datos = malloc(tamanio);
		recv(socketFS, datos, tamanio, 0);
		send(socketCpu, datos, tamanio,0);
	}else{
		//TODO: Dar error
	}
}

void escribirArchivo(int socketCpu, int socketFS){
	int fd;
	int pid;
	int tamanio;
	//Recibo datos del CPU
	recv(socketCpu, &fd, sizeof(fd), 0);
	recv(socketCpu, &pid, sizeof(fd), 0);
	recv(socketCpu, &tamanio, sizeof(tamanio), 0);
	void* datos = malloc(tamanio);
	recv(socketCpu, datos, tamanio, 0);
	if(fd==1){
		//TODO: Mandar a imprimir a la consola correspondiente
		char *texto = datos;
		int i;
		for (i = 0; i < tamanio; ++i) {
			if(i==0)
				printf("Printf : |");
			printf("%d|",(int)texto[i]);
		}
		printf("\n");
		return;
	}

	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->privilegiadas++;

	FD_t* fileDescriptor = obtenerFD(pid, fd);

	if(string_contains(fileDescriptor->permisos,"w")==NULL){
		//TODO Dar Error de permisos y terminar el proceso
		//ACA METER send(socketCpu, &RES, sizeof(fd),0); y ese res tiene que ser -4
		//Primitiva abrir en CPU lo detecta como -4 => exit code y sale del programa.
		return;
	}
	globalFD_t* globalFD = list_get(tablaGlobalArchivos, fileDescriptor->indiceTablaGlobal);
	//Envio mensaje al FS
	int codOperacion = accionEscribir;
	int pathSize = string_length(globalFD->path)+1;
	// Tamanio paquete = codOperacion + pathSize + path + offset + tamanio + datos
	int packetSize = pathSize + sizeof(int)*4 + tamanio;
	void *buffer = malloc(packetSize);
	memcpy(buffer, &codOperacion, sizeof(int));
	memcpy(buffer + sizeof(int), &pathSize, sizeof(int));
	memcpy(buffer + sizeof(int)*2, globalFD->path, pathSize);
	memcpy(buffer + sizeof(int)*2 + pathSize, &fileDescriptor->offset, sizeof(int));
	memcpy(buffer + sizeof(int)*3 + pathSize, &tamanio, sizeof(int));
	memcpy(buffer + sizeof(int)*4 + pathSize, datos, tamanio);
	send(socketFS, buffer, packetSize,0);
	free(buffer);
	//Recibo respuesta
	int res;
	recv(socketFS, &res, sizeof(res), 0);
	//Analizar respuesta y enviar a CPU
	if(res==1){
		//TODO: OK
	}else{
		//TODO: -1 error al escribir el archivo
	}
	//TODO: Enviar peticion al FS y enviar respuesta a la CPU
}

void cerrarArchivo(int socketCpu, int socketFS){
	int fd;
	int pid;
	recv(socketCpu, &fd, sizeof(fd), 0);
	recv(socketCpu, &pid, sizeof(pid), 0);
	FD_t* fileDescriptor = obtenerFD(pid, fd);

	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->privilegiadas++;

	//TODO: Recibir informacion desde el CPU
	quitarTablaGlobal(fileDescriptor);
	quitarTablaProceso(pid, fd);
}

void borrarArchivo(int socketCPU, int socketFS){
	//Recibo datos del CPU
	int pid;
	int fd;
	recv(socketCPU, &fd, sizeof(fd), 0);
	recv(socketCPU, &pid, sizeof(pid), 0);
	FD_t* fileDescriptor = obtenerFD(pid, fd);

	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->privilegiadas++;

	globalFD_t* globalFD = list_get(tablaGlobalArchivos, fileDescriptor->indiceTablaGlobal);
	if(globalFD->cantProcesos>1){
		//Error: No se puede borrar el archivo porque alguien mas lo tienen abierto
		//TODO: Terminar el proceso.
	}else{
		//quitarTablaGlobal(fileDescriptor);   VER SI HAY QUE SACARLO O NO
		//quitarTablaProceso(pid, fd);
		//Envio mensaje al FS
		int codOperacion = accionBorrarArchivo;
		int pathSize = string_length(globalFD->path)+1;
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
		if(res==1){
			//OK
		}else{
			//TODO: Error al borrar el archivo
		}
	}
}

void moverCursor(int socketCPU, int socketFS){
	//Recibo datos del CPU
	int pid;
	int fd;
	int offset;
	recv(socketCPU, &fd, sizeof(fd), 0);
	recv(socketCPU, &pid, sizeof(pid), 0);
	recv(socketCPU, &offset, sizeof(offset), 0);
	FD_t* fileDescriptor = obtenerFD(pid, fd);

	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->privilegiadas++;

	fileDescriptor->offset=offset;
}
