#include "memo.h"

int main(void) {

	cargarConfigFile();

	//Cliente
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.ipKernel);
	direccionServidor.sin_port = htons(config.puertoKernel);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);


	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("Error al conectar");
		return 1;
	}

	while(1){
		char mensaje[1000];
		fgets(mensaje, sizeof mensaje, stdin);
		send(cliente, mensaje, strlen(mensaje), 0);
	}
	return 0;
}

void cargarConfigFile(){
	t_config* configMemo;
	configMemo = config_create("../memo.cfg");
//	char* pat = string_new();
//	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
//	string_append(&pat,getcwd(cwd,sizeof(cwd)));
//	string_append(&pat,"/memo.cfg");
//	t_config* configMemo = config_create(pat);
	if (config_has_property(configMemo, "IP_KERNEL"))
		config.puertoKernel = config_get_int_value(configMemo, "PUERTO_KERNEL");
	if (config_has_property(configMemo, "IP_KERNEL"))
		config.ipKernel = config_get_string_value(configMemo, "IP_KERNEL");
}

