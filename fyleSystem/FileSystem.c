/*
 * FileSystem.c
 *
 *  Created on: 7/4/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include "fileSystem.h"
#include <sys/stat.h>
#include <sys/types.h>

int conectarSocket(int socket, struct sockaddr_in* dirServidor)
{
 return connect(socket, (struct sockaddr*) &*dirServidor, sizeof(struct sockaddr));
}

void cargarConfiguracion()
{
	char* pat = string_new();
	//char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	//string_append(&pat,getcwd(cwd,sizeof(cwd)));
	string_append(&pat,"/home/utnso/projects/tp-2017-1c-No-Se-Recursa/fyleSystem");
	string_append(&pat,"/FileSystem.cfg");
	t_config* configFs = config_create(pat);

	printf("El directorio sobre el que se esta trabajando es %s\n", pat);
	free(pat);

	if (config_has_property(configFs, "PUERTO")){
		config.PUERTO = config_get_int_value(configFs,"PUERTO");
		printf("config.PUERTO: %i\n", config.PUERTO);
	}
	if (config_has_property(configFs, "PUNTO_MONTAJE")){
		config.PUNTO_MONTAJE = config_get_string_value(configFs,"PUNTO_MONTAJE");
		printf("config.PUNTO_MONTAJE: %s\n", config.PUNTO_MONTAJE);
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
}

void leerMetadata(){
	t_config* config = config_create(paths.Metadata);

	if (config_has_property(config, "CANTIDAD_BLOQUES")){
		metadata.CANTIDAD_BLOQUES = config_get_int_value(config,"CANTIDAD_BLOQUES");
		printf("metadata.CANTIDAD_BLOQUES: %i\n", metadata.CANTIDAD_BLOQUES);
	}
	if (config_has_property(config, "CANTIDAD_BLOQUES")){
		metadata.TAMANIO_BLOQUES = config_get_int_value(config,"TAMANIO_BLOQUES");
		printf("metadata.TAMANIO_BLOQUES: %i\n", metadata.TAMANIO_BLOQUES);
	}
	if (config_has_property(config, "CANTIDAD_BLOQUES")){
		metadata.MAGIC_NUMBER = config_get_string_value(config,"MAGIC_NUMBER");
		printf("metadata.MAGIC_NUMBER: %s\n", metadata.MAGIC_NUMBER);
	}
}

/***********************************BITMAP**************************************/

void crearBitmap(){
	char * bitarray = (char*) malloc(sizeof(char)*metadata.CANTIDAD_BLOQUES/8);
	bitmap = bitarray_create_with_mode(bitarray,metadata.CANTIDAD_BLOQUES/8, LSB_FIRST);
}

void leerBitmap(){
	FILE *archivo = fopen(paths.Bitmap,"rb");
	crearBitmap();
	fread(bitmap->bitarray, sizeof(char), metadata.CANTIDAD_BLOQUES/8, archivo);
	fclose(archivo);
}

void escribirBitmap(){
	FILE *archivo = fopen(paths.Bitmap,"wb");
	fwrite(bitmap->bitarray, sizeof(char), metadata.CANTIDAD_BLOQUES/8, archivo);
	fclose(archivo);
}

void destruirBitmap(t_bitarray *bitmap){
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);
}

/***********************************BLOQUES*************************************/

int buscarBloqueLibre(){
	int i;
	for (i = 0; i < metadata.CANTIDAD_BLOQUES; ++i) {
		if(bitarray_test_bit(bitmap, i)==0){
			break;
		}
	}
	return (i == metadata.CANTIDAD_BLOQUES)? -1 : i;
}

void liberarBloque(int index){
	bitarray_clean_bit(bitmap, index);
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



/***********************************OPERACIONES FS******************************/


int validarArchivo(char *path)
{
	archivo_t *archivo = newArchivo();
	return leerArchivo(path, archivo);
	free(archivo);
}

int crearArchivo(char *path)
{
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
				if (errno != EEXIST)
					return -1;
			}

			*p = '/';
			free(pat);
		}
	}
	pat = string_new();
	string_append(&pat,paths.Archivos);
	string_append(&pat,path);
	FILE *archivo = fopen(pat,"w");
	free(pat);
	free(pathLocal);
	int bloque = buscarBloqueLibre();
	if(bloque<0){
		return -1;
	}
	bitarray_set_bit(bitmap,bloque);
	escribirBitmap();
	fprintf(archivo, "TAMANIO=0\n");
	fprintf(archivo, "BLOQUES=[%i]", bloque);
	fclose(archivo);
	return 0;
}

void borrarArchivo(char *path)
{
	archivo_t *archivo = newArchivo();
	int res = leerArchivo(path, archivo);
	if(res<0){
		puts("Error al borrar el archivo");
		return;
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
	string_append(&path, path);
	remove(pat);
	free(pat);
}

char* obtenerDatos(char *path, int offset, int size)
{
	//TODO: Implementar lectura de archivo
	char *texto = "Algo";
	return texto;
}

void guardarDatos(char *path, int offset, int size, char* buffer)
{

}

// Programa Principal
int main(void) {
	//printf("Dentro del main\n");

	//char* buffer = malloc(5);

	cargarConfiguracion();//Cargo configuracion
	leerMetadata();
	leerBitmap();

	printf("Cantidad de bloques en bitmap = %i\n", bitarray_get_max_bit(bitmap));

	struct sockaddr_in direccionServidor; // Información sobre mi dirección
	struct sockaddr_in direccionCliente; // Información sobre la dirección del cliente
	int addrlen; // El tamaño de la direccion del cliente
	int sockServ; // Socket de nueva conexion aceptada
	int sockClie; // Socket a la escucha
	int cantBytesRecibidos;

	sockServ = crearSocket();
	reusarSocket(sockServ, 1);
	direccionServidor = crearDireccionServidor(config.PUERTO);
	bind_w(sockServ, &direccionServidor);
	listen_w(sockServ);
	printf("Escuchando nuevas solicitudes tcp en el puerto %d...\n", config.PUERTO);

	for (;;) {
		if ((sockClie = accept(sockServ, (struct sockaddr*) &direccionCliente, &addrlen)) == -1) {
			perror("Error en el accept");
		} else {
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
					char* bytesAEscribir;
					char* bytesSolicitados;
					int resultAccion;
					int pidAFinalizar;

					switch (codAccion) {

					case accionAbrirArchivo:
						int largoMsg;
						recv(sockClie, &largoMsg, sizeof(largoMsg), 0);
						char *path = (char*) malloc(sizeof(char)*largoMsg);
						recv(sockClie, path, largoMsg, 0);
						int res = validarArchivo(path);
						send(sockClie, &res, sizeof(res),0);
						printf("Recibida solicitud de apertura de archivo: %s\n", path);
						break;

					case accionBorrarArchivo:
						int largoMsg;
						recv(sockClie, &largoMsg, sizeof(largoMsg), 0);
						char *path = (char*) malloc(sizeof(char)*largoMsg);
						recv(sockClie, path, largoMsg, 0);
						int res = borrarArchivo(path);
						send(sockClie, &res, sizeof(res),0);
						printf("Recibida solicitud de borrado de archivo: %s\n", path);
						break;

					case accionCrearArchivo:
						int largoMsg;
						recv(sockClie, &largoMsg, sizeof(largoMsg), 0);
						char *path = (char*) malloc(sizeof(char)*largoMsg);
						recv(sockClie, path, largoMsg, 0);
						int res = crearArchivo(path);
						send(sockClie, &res, sizeof(res),0);
						printf("Recibida solicitud de creacion de archivo: %s\n", path);
						break;

					case accionObtenerDatosArchivo:
						int largoMsg;
						recv(sockClie, &largoMsg, sizeof(largoMsg), 0);
						char *path = (char*) malloc(sizeof(char)*largoMsg);
						recv(sockClie, path, largoMsg, 0);
						int offset;
						int size;
						recv(sockClie, &offset, sizeof(offset), 0);
						recv(sockClie, &size, sizeof(size), 0);
						char * datos = obtenerDatos(path, offset, size);
						send(sockClie, datos, size,0);
						printf("Recibida solicitud de lectura de archivo: %s\n", path);
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

/*
    //Creo Cliente
	struct sockaddr_in direccionServidor;//Estructura con la direccion del servidor
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.IP_KERNEL);//IP a la que se conecta
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(config.PUERTO_KERNEL);//Puerto al que se conecta

	memset(&(direccionServidor.sin_zero), '\0', 8);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);//Pido un Socket
	printf("cliente: %d\n", cliente);

	if(conectarSocket(cliente, &direccionServidor) == -1)
	 {
	  perror("No se pudo conectar");
	  exit(1);
	 }

	//Recibo mensajes y muestro en pantalla
	while (1) {
		int bytesRecibidos = recv(cliente, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			perror("El socket se desconecto\n");
			return 1;
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes --> %s\n", bytesRecibidos, buffer);
	}

	free(buffer);

	//Envio mensajes
	while (1) {
		char mensaje[1000];
		scanf("%s", mensaje);

		send(cliente, mensaje, strlen(mensaje), 0);
	}

	close(cliente);
*/


	return 0;
}
