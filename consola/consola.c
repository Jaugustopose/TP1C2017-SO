#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "consola.h"


int cargarConfiguracion(){

	struct configuracion config;
	t_config* configConsola = config_create("consola.cfg");
	config.IP_KERNEL = config_get_int_value(configConsola, "IP_KERNEL");
	config.PUERTO_KERNEL = config_get_string_value(configConsola, "PUERTO_KERNEL");
     return 1;
}

int main (void){
	if(cargarConfiguracion() == 1){
		printf("esta Ok");
		return 0;
	}
}
