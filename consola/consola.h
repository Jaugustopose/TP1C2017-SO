#ifndef consola_h
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <commons/config.h>


struct configuracion{
	int IP_KERNEL;
	char* PUERTO_KERNEL;
};

t_config* configConsola;
struct configuracion config;

#endif
