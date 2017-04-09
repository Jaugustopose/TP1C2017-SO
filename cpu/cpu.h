#ifndef cpu_h
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
