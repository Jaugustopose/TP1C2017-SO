
#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/string.h>



typedef struct sock
{
	int32_t fd;
	struct sockaddr_in* sockaddr;
} t_sock;

typedef struct paquete
{
	int32_t tamanio;
	void* contenido;
}t_paquete;



#endif /* SOCKETS_H_ */
