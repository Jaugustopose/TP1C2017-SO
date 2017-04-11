#ifndef cpu_h
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <commons/config.h>
#include <unistd.h>

struct configuracion{
	int IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* PUERTO_KERNEL;
};

t_config* configCpu;
struct configuracion config;



#endif
