

#ifndef CLIENTE_SERVIDOR_H_
#define CLIENTE_SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/string.h>

int crearSocket();
void reusarSocket(int sockServ, int yes);
void bind_w(int sockServ, const struct sockaddr_in* mi_addr);
void listen_w(int sockServ);
struct sockaddr_in crearDireccionServidor(unsigned short PORT);



#endif /* CLIENTE_SERVIDOR_H_ */
