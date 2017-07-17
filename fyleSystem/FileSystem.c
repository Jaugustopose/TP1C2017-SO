/*
 * FileSystem.c
 *
 *  Created on: 7/4/2017
 *      Author: utnso
 */

#include "fileSystem.h"

int cargarConfiguracion()
{
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat,getcwd(cwd,sizeof(cwd)));
	if (string_contains(pat, "/Debug")){
		string_append(&pat,"/FileSystem.cfg");
	}else{
	string_append(&pat, "/Debug/FileSystem.cfg");
	}
	t_config* configFs = config_create(pat);
	if(configFs==NULL){
		return -1;
		printf("No se encontró el archivo de configuración: %s\n", pat);
	}
	printf("El directorio sobre el que se esta trabajando es %s\n", pat);
	free(pat);

	if (config_has_property(configFs, "PUERTO")){
		config.PUERTO = config_get_int_value(configFs,"PUERTO");
		printf("config.PUERTO: %i\n", config.PUERTO);
	}else{
		printf("No se encontró el parámetro PUERTO dentro del archivo de configuración\n");
		return -1;
	}
	if (config_has_property(configFs, "PUNTO_MONTAJE")){
		config.PUNTO_MONTAJE = config_get_string_value(configFs,"PUNTO_MONTAJE");
		printf("config.PUNTO_MONTAJE: %s\n", config.PUNTO_MONTAJE);
	}else{
		printf("No se encontró el parámetro PUNTO_MONTAJE dentro del archivo de configuración\n");
		return -1;
	}

	pat = string_new();
	string_append(&pat,config.PUNTO_MONTAJE);
	string_append(&pat,"/Metadata/Metadata.bin");
	paths.Metadata = pat;

	pat = string_new();
	string_append(&pat,config.PUNTO_MONTAJE);
	string_append(&pat,"/Metadata/Bitmap.bin");
	paths.Bitmap = pat;

	pat = string_new();
	string_append(&pat,config.PUNTO_MONTAJE);
	string_append(&pat,"/Archivos/");
	paths.Archivos = pat;

	pat = string_new();
	string_append(&pat,config.PUNTO_MONTAJE);
	string_append(&pat,"/Bloques/");
	paths.Bloques = pat;

	return 0;
}

int leerMetadata(){
	t_config* config = config_create(paths.Metadata);
	if(config==NULL){
		printf("No se encontró el archivo Metadata en :%s\n", paths.Metadata);
		return -1;
	}

	if (config_has_property(config, "CANTIDAD_BLOQUES")){
		metadata.CANTIDAD_BLOQUES = config_get_int_value(config,"CANTIDAD_BLOQUES");
		printf("metadata.CANTIDAD_BLOQUES: %i\n", metadata.CANTIDAD_BLOQUES);
	}else{
		printf("No se encontró el parámetro CANTIDAD_BLOQUES dentro del archivo Metadata\n");
		return -1;
	}
	if (config_has_property(config, "TAMANIO_BLOQUES")){
		metadata.TAMANIO_BLOQUES = config_get_int_value(config,"TAMANIO_BLOQUES");
		printf("metadata.TAMANIO_BLOQUES: %i\n", metadata.TAMANIO_BLOQUES);
	}else{
		printf("No se encontró el parámetro TAMANIO_BLOQUES dentro del archivo Metadata\n");
		return -1;
	}
	if (config_has_property(config, "MAGIC_NUMBER")){
		metadata.MAGIC_NUMBER = config_get_string_value(config,"MAGIC_NUMBER");
		printf("metadata.MAGIC_NUMBER: %s\n", metadata.MAGIC_NUMBER);
	}else{
		printf("No se encontró el parámetro MAGIC_NUMBER dentro del archivo Metadata\n");
		return -1;
	}
	//Creo carpetas de Bloques y archivos si es que no existen
	mkdir(paths.Bloques, S_IRWXU);
	mkdir(paths.Archivos, S_IRWXU);
	return 0;
}

/***********************************BITMAP**************************************/

void crearBitmap(){
	char * bitarray = (char*) malloc(sizeof(char)* ((metadata.CANTIDAD_BLOQUES+8-1)/8)); //Redondeado para arriba
	bitmap = bitarray_create_with_mode(bitarray,(metadata.CANTIDAD_BLOQUES+8-1)/8, LSB_FIRST);
}

void leerBitmap(){
	FILE *archivo = fopen(paths.Bitmap,"rb");
	if(archivo==NULL){
		archivo = fopen(paths.Bitmap, "wb");
		crearBitmap();
		fclose(archivo);
	}else{
		crearBitmap();
		fread(bitmap->bitarray, sizeof(char), (metadata.CANTIDAD_BLOQUES+8-1)/8, archivo);
		fclose(archivo);
	}
}

void escribirBitmap(){
	FILE *archivo = fopen(paths.Bitmap,"wb");
	fwrite(bitmap->bitarray, sizeof(char), (metadata.CANTIDAD_BLOQUES+8-1)/8, archivo);
	fclose(archivo);
}

void destruirBitmap(t_bitarray *bitmap){
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);
}

/***********************************BLOQUES*************************************/

void crearBloques(){
	int i;
	FILE *archivo;
	for (i = 1; i <= metadata.CANTIDAD_BLOQUES; ++i) {
		char* pat = string_new();
		string_append(&pat,paths.Bloques);
		string_append(&pat,string_itoa(i));
		string_append(&pat,".bin");
		archivo = fopen(pat,"rb");
		if(archivo==NULL){
			archivo = fopen(pat,"wb");
		}
		free(pat);
		fclose(archivo);
	}
}

int* buscarBloquesLibres(int cantidad){
	int i;
	int j=0;
	int *bloques = (int*) malloc(sizeof(int) * cantidad);
	for (i = 0; i < metadata.CANTIDAD_BLOQUES && j != cantidad; ++i) {
		if(bitarray_test_bit(bitmap, i)==0){
			bloques[j]=i;
			j++;
		}
	}
	return (i == metadata.CANTIDAD_BLOQUES)? NULL : bloques;
}

void liberarBloque(int index){
	bitarray_clean_bit(bitmap, index);
}

void reservarBloque(int index){
	bitarray_set_bit(bitmap, index);
}

/***********************************ARCHIVOS************************************/

archivo_t* newArchivo(){
	return (archivo_t*) malloc(sizeof(archivo_t));
}

int leerArchivo(char *path, archivo_t *archivo){
	char* pat = string_new();
	string_append(&pat,paths.Archivos);
	string_append(&pat,path);
	t_config* config = config_create(pat);
	free(pat);

	if(config==NULL){
		return -1; //Archivo inexistente
	}

	if (config_has_property(config, "TAMANIO")){
		archivo->TAMANIO = config_get_int_value(config,"TAMANIO");
	}else{
		return -2; //Archivo corrupto
	}
	if (config_has_property(config, "BLOQUES")){
		archivo->BLOQUES = config_get_array_value(config,"BLOQUES");
	}else{
		return -2; //Archivo corrupto
	}

	return 0;
}

void escribirArchivo(char* path, archivo_t *archivo){
	char* pat = string_new();
	string_append(&pat,paths.Archivos);
	string_append(&pat,path);
	FILE* arch = fopen(pat, "w");
	free(pat);
	fprintf(arch, "TAMANIO=%i\n", archivo->TAMANIO);
	fprintf(arch, "BLOQUES=[");
	int i;
	for (i = 0; archivo->BLOQUES[i]; ++i) {
		if(i!=0){
			fprintf(arch, ",");
		}
		fprintf(arch, "%s", archivo->BLOQUES[i]);
	}
	fprintf(arch, "]");
	fclose(arch);
}

int crearDirectorio(char* path){
	char* p;
	char* pat;
	char* pathLocal = string_new();
	string_append(&pathLocal,path);
	for (p = pathLocal + 1; *p; p++) {
		if (*p == '/') {

			*p = '\0';
			pat = string_new();
			string_append(&pat,paths.Archivos);
			string_append(&pat,pathLocal);

			if (mkdir(pat, S_IRWXU) != 0) {
				if (errno != EEXIST){
					free(pat);
					free(pathLocal);
					return -1;
				}
			}

			*p = '/';
			free(pat);
		}
	}
	free(pathLocal);
	return 0;
}

/***********************************OPERACIONES FS******************************/

int validarArchivo(char *path)
{
	archivo_t *archivo = newArchivo();
	return leerArchivo(path, archivo);
	free(archivo);
}

int crearArchivo(char *path)
{
	int *bloque = buscarBloquesLibres(1);
	if(bloque == NULL){
		return -1;
	}
	char *pat = string_new();
	string_append(&pat,paths.Archivos);
	string_append(&pat,path);
	FILE *archivo = fopen(pat,"w");
	if(archivo == NULL){
		if(crearDirectorio(path)<0){
			return -2;
		}else{
			archivo = fopen(pat,"w");
			if(archivo == NULL){
				return -2;
			}
		}
	}
	bitarray_set_bit(bitmap,bloque[0]);
	escribirBitmap();
	fprintf(archivo, "TAMANIO=0\n");
	fprintf(archivo, "BLOQUES=[%i]", bloque[0]);
	fclose(archivo);
	free(bloque);
	return 0;
}

int borrarArchivo(char *path)
{
	archivo_t *archivo = newArchivo();
	int res = leerArchivo(path, archivo);
	if(res<0){
		puts("Error al borrar el archivo");
		return -1;
	}
	int i=0;
	while(archivo->BLOQUES[i]){
		liberarBloque(strtol(archivo->BLOQUES[i],NULL,10));
		printf("Bloque liberado: %s\n",archivo->BLOQUES[i]);
		i++;
	}
	escribirBitmap();
	char * pat = string_new();
	string_append(&pat,paths.Archivos);
	string_append(&pat, path);
	remove(pat);
	free(pat);
	return 0;
}

char* obtenerDatos(char *path, int offset, int size)
{
	archivo_t *archivo = newArchivo();
	int res = leerArchivo(path, archivo);
	if(res<0){
		puts("Error al leer datos del archivo");
		return NULL;
	}

	//Leer datos en bloques
	int bloqueActual = offset / metadata.TAMANIO_BLOQUES; //Bloque inicial
	int offsetBloque;
	int bytesALeer = size;
	char* buffer = (char*) malloc(sizeof(char)*size+1);
	while(bytesALeer){
		offsetBloque = offset - bloqueActual * metadata.TAMANIO_BLOQUES;
		char *pat = string_new();
		string_append(&pat,paths.Bloques);
		string_append(&pat,archivo->BLOQUES[bloqueActual]);
		string_append(&pat,".bin");
		FILE* archBloque = fopen(pat,"r");
		fseek(archBloque,offsetBloque,SEEK_SET);
		int bytesLibres = metadata.TAMANIO_BLOQUES - offsetBloque;
		int cant = (bytesALeer<=bytesLibres)? bytesALeer : bytesLibres;
		fread(buffer+size-bytesALeer,sizeof(char),cant,archBloque);
		fclose(archBloque);
		free(pat);
		bytesALeer-=cant;
		bloqueActual++;
	}

	return buffer;
}

int guardarDatos(char *path, int offset, int size, char* buffer)
{
	archivo_t *archivo = newArchivo();
	int res = leerArchivo(path, archivo);
	if(res<0){
		puts("Error al guardar datos en el archivo");
		return -1;
	}
	//Calculo bloques necesarios y los reservo
	int tamanio = offset + size;
	int bloquesNecesarios = (tamanio + metadata.TAMANIO_BLOQUES - 1) / metadata.TAMANIO_BLOQUES; // redondeo para arriba
	int bloquesReservados = 0;
	for (bloquesReservados = 0 ; archivo->BLOQUES[bloquesReservados] ; bloquesReservados++);
	if(bloquesNecesarios>bloquesReservados){
		int cantBloques = bloquesNecesarios-bloquesReservados;
		int *bloques = buscarBloquesLibres(cantBloques);
		if(bloques==NULL){
			puts("Error al guardar datos en el archivo. No hay suficientes bloques libres");
			return -1;
		}
		int i;
		char **arrBloques = (char**) malloc(sizeof(char*)*bloquesNecesarios);
		for (i = 0; archivo->BLOQUES[i]; ++i) {
			arrBloques[i] = archivo->BLOQUES[i];
		}
		int j;
		for (j = 0; j < cantBloques; ++j) {
			reservarBloque(bloques[j]);
			arrBloques[i] = string_itoa(bloques[j]);
			i++;
		}
		free(archivo->BLOQUES);
		archivo->BLOQUES=arrBloques;
	}
	//Escribir datos en bloques
	int bloqueActual = offset / metadata.TAMANIO_BLOQUES; //Bloque inicial
	int offsetBloque;
	int bytesAEscribir = size;
	while(bytesAEscribir){
		offsetBloque = offset - bloqueActual * metadata.TAMANIO_BLOQUES;
		char *pat = string_new();
		string_append(&pat,paths.Bloques);
		string_append(&pat,archivo->BLOQUES[bloqueActual]);
		string_append(&pat,".bin");
		FILE* archBloque = fopen(pat,"r+");
		fseek(archBloque,offsetBloque,SEEK_SET);
		int bytesLibres = metadata.TAMANIO_BLOQUES - offsetBloque;
		int cant = (bytesAEscribir<=bytesLibres)? bytesAEscribir : bytesLibres;
		fwrite(buffer+size-bytesAEscribir,sizeof(char),cant,archBloque);
		fclose(archBloque);
		free(pat);
		bytesAEscribir-=cant;
		bloqueActual++;
	}
	//Escribo nuevo tamaño
	archivo->TAMANIO = (tamanio>archivo->TAMANIO)? tamanio : archivo->TAMANIO;
	//Escribo archivos de metadata
	escribirArchivo(path, archivo);
	escribirBitmap();
	return 0;
}

/*************************************SOCKETS FS********************************/

int verificarIdentidad(){
	int identidad;
	recv(sockClie, &identidad, sizeof(int), 0);
	if(identidad==SOYKERNEL){
		int res = SOYFS;
		send(sockClie, &res, sizeof(res),0);
		return 1;
	}
	return 0;
}

void sockets(){

	struct sockaddr_in direccionServidor; // Información sobre mi dirección
	struct sockaddr_in direccionCliente; // Información sobre la dirección del cliente
	socklen_t addrlen; // El tamaño de la direccion del cliente
	int cantBytesRecibidos;

	sockServ = crearSocket();
	reusarSocket(sockServ, 1);
	direccionServidor = crearDireccionServidor(config.PUERTO);
	bind_w(sockServ, &direccionServidor);
	listen_w(sockServ);

	for (;;) {
		printf("Escuchando nuevas solicitudes tcp en el puerto %d...\n", config.PUERTO);
		if ((sockClie = accept(sockServ, (struct sockaddr*) &direccionCliente, &addrlen)) == -1) {
			perror("Error en el accept");
		} else {
			if(!verificarIdentidad()){
				close(sockClie);
				printf("El cliente conectado no era un Kernel, se rechaza la conexión");
				continue;
			}
			printf("Server: nueva conexion de %s en socket %d\n", inet_ntoa(direccionCliente.sin_addr), sockClie);
			for (;;) {
				// Gestionar datos de un cliente. Recibimos el código de acción que quiere realizar.
				int codAccion;
				if ((cantBytesRecibidos = recv(sockClie, &codAccion, sizeof(int), 0)) <= 0) {
					// error o conexión cerrada por el cliente
					if (cantBytesRecibidos == 0) {
						// conexión cerrada
						printf("Server: socket %d termino la conexion\n", sockClie);
						close(sockClie);
						break;
					} else {
						perror("Se ha producido un error en el Recv");
						break;
					}
				} else {
					printf("He recibido %d bytes con la acción: %d\n", cantBytesRecibidos, codAccion);
					int resultAccion;
					int largoPath;
					char *path;
					int res;
					int offset;
					int size;
					char* datos;

					switch (codAccion) {

					case accionAbrirArchivo:
						recv(sockClie, &largoPath, sizeof(largoPath), 0);
						path = malloc(largoPath);
						recv(sockClie, path, largoPath, 0);
						res = validarArchivo(path);
						free(path);
						send(sockClie, &res, sizeof(res),0);
						printf("Recibida solicitud de apertura de archivo: %s\n", path);
						break;

					case accionBorrarArchivo:
						recv(sockClie, &largoPath, sizeof(largoPath), 0);
						path = malloc(largoPath);
						recv(sockClie, path, largoPath, 0);
						res = borrarArchivo(path);
						free(path);
						send(sockClie, &res, sizeof(res),0);
						printf("Recibida solicitud de borrado de archivo: %s\n", path);
						break;

					case accionCrearArchivo:
						recv(sockClie, &largoPath, sizeof(largoPath), 0);
						path = malloc(largoPath);
						recv(sockClie, path, largoPath, 0);
						res = crearArchivo(path);
						free(path);
						send(sockClie, &res, sizeof(res),0);
						printf("Recibida solicitud de creacion de archivo: %s\n", path);
						break;

					case accionObtenerDatosArchivo:
						recv(sockClie, &largoPath, sizeof(largoPath), 0);
						path = malloc(largoPath);
						recv(sockClie, path, largoPath, 0);
						recv(sockClie, &offset, sizeof(offset), 0);
						recv(sockClie, &size, sizeof(size), 0);
						datos = obtenerDatos(path, offset, size);
						free(path);
						if(datos==NULL){
							int error = -1;
							send(sockClie, &error, sizeof(int),0);
						}else{
							int error = 0;
							send(sockClie, &error, sizeof(int),0);
							send(sockClie, datos, size,0);
							free(datos);
						}
						printf("Recibida solicitud de lectura de archivo: %s\n", path);
						break;

					case accionEscribir:
						recv(sockClie, &largoPath, sizeof(largoPath), 0);
						path = malloc(largoPath);
						recv(sockClie, path, largoPath, 0);
						recv(sockClie, &offset, sizeof(offset), 0);
						recv(sockClie, &size, sizeof(size), 0);
						datos = malloc(size);
						recv(sockClie, datos, size, 0);
						res = guardarDatos(path, offset, size, datos);
						free(path);
						free(datos);
						send(sockClie, &res, sizeof(res),0);
						printf("Recibida solicitud de escritura de archivo: %s\n", path);
						break;


					default:
						printf("No reconozco el código de acción\n");
						resultAccion = -13;
						send(sockClie, &resultAccion, sizeof(resultAccion), 0);
					}
					printf("Fin atención acción\n");
				}
			}
		}
	}
}

/***************************************MAIN FS*********************************/

int main(void) {

	if(cargarConfiguracion()<0){
		exit(EXIT_FAILURE);
	}
	if(leerMetadata()<0){
		exit(EXIT_FAILURE);
	}
	crearBloques();
	leerBitmap();
	printf("Cantidad de bloques en bitmap = %i\n", bitarray_get_max_bit(bitmap));

	//Crear hilo para manejar al comunicacion con el kernel
	pthread_t hiloSockets;
	pthread_create(&hiloSockets, NULL, (void*)sockets, NULL);

	for(;;){
		char *userInput=NULL;
		size_t size=0;
		getline(&userInput,&size,stdin);
		if(!strcmp("exit\n",userInput)){
			close(sockClie);
			close(sockServ);
			printf("Proceso finalizado por el usuario\n");
			exit(EXIT_SUCCESS);
		}
	}

	return 0;
}
