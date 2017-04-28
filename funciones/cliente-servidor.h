/*
 * cliente-servidor.h
 *
 *  Created on: 27/4/2017
 *      Author: utnso
 */

#ifndef CLIENTE_SERVIDOR_H_
#define CLIENTE_SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <commons/string.h>
#include <string.h>

#define MAXCLIENTS 100

struct sockaddr_in direccion;
int socketNuevasConexiones, tamanioDireccion, activado, mayorDescriptor;
fd_set socketsParaLectura;

typedef struct {
	struct sockaddr_in addr;
	socklen_t addrlen;
	int socket;
	int identidad;
	bool atentido;
} t_cliente;

t_cliente clientes[MAXCLIENTS];

#endif /* CLIENTE_SERVIDOR_H_ */
