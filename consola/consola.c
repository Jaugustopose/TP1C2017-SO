#include "consola.h"
#include "cliente-servidor.h"
#include "serializador.h"
#include "estructurasCompartidas.h"
#include <time.h>
#include <sys/timeb.h>


char* transformarTime(struct timeb time) {
	struct tm *log_tm = malloc(sizeof(struct tm));
	char *str_time = string_duplicate("hh:mm:ss:mmmm");
	time_t log_time;

	log_time = time.time;
	localtime_r(&log_time, log_tm);

	char *partial_time = string_duplicate("hh:mm:ss");
	strftime(partial_time, 127, "%H:%M:%S", log_tm);
	sprintf(str_time, "%s:%hu", partial_time, time.millitm);
	free(partial_time);
	free(log_tm);

	//Adjust memory allocation
	str_time = realloc(str_time, strlen(str_time) + 1);
	return str_time;
}

char* calcularDuracion(struct timeb inicio, struct timeb fin) {
	struct tm* tmInicio = malloc(sizeof(struct tm));
	struct tm* tmFin = malloc(sizeof(struct tm));
	char *string = string_new();

	localtime_r(&inicio.time, tmInicio);
	localtime_r(&fin.time, tmFin);

	int mili;
	int seg;
	int min;
	int hora;
	int carry=0;

	if(fin.millitm >= inicio.millitm){
		mili = fin.millitm - inicio.millitm;
	}else{
		mili = fin.millitm + 1000 - inicio.millitm;
		carry = 1;
	}
	if(tmFin->tm_sec - carry >= tmInicio->tm_sec){
		seg = tmFin->tm_sec - tmInicio->tm_sec - carry;
		carry = 0;
	}else{
		seg = tmFin->tm_sec + 60 - tmInicio->tm_sec - carry;
		carry = 1;
	}
	if(tmFin->tm_min - carry >= tmInicio->tm_min){
		min = tmFin->tm_min - tmInicio->tm_min - carry;
		carry = 0;
	}else{
		min = tmFin->tm_min + 60 - tmInicio->tm_min - carry;
		carry = 1;
	}
	hora = tmFin->tm_hour - tmInicio->tm_hour - carry;

	string_append(&string,string_itoa(hora));
	string_append(&string,":");
	string_append(&string,string_itoa(min));
	string_append(&string,":");
	string_append(&string,string_itoa(seg));
	string_append(&string,":");
	string_append(&string,string_itoa(mili));

	free(tmFin);
	free(tmInicio);

	return string;
}

void cargarConfiguracion(){

	char* pat = string_new();
		char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
		string_append(&pat, getcwd(cwd, sizeof(cwd)));
		if (string_contains(pat, "/Debug")) {
			string_append(&pat, "/consola.cfg");
		} else {
			string_append(&pat, "/Debug/consola.cfg");
		}
		t_config* configConsola = config_create(pat);
		free(pat);
		if (config_has_property(configConsola, "IP_KERNEL")){
				config.IP_KERNEL = config_get_string_value(configConsola,"IP_KERNEL");
		printf("config.IP_KERNEL: %s\n", config.IP_KERNEL);
		}

		if (config_has_property(configConsola, "PUERTO_KERNEL")){
				config.PUERTO_KERNEL = config_get_int_value(configConsola,"PUERTO_KERNEL");
		printf("config.PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);
		}
}

char* convertirArchivoACodigo()
{
	char* contenido = string_new();

	size_t largo = 0;
	char* linea = NULL;
	ssize_t lectura = getline(&linea, &largo, programa);

	   //Mientras haya algo que leer, entonces lee
		while (lectura != -1) {

			string_append(&contenido, linea);

			//free(linea);

			largo = 0; //no se si es necesario... pero nunca sobra
			lectura = getline(&linea, &largo, programa);
		}

		//free(linea);

		string_append(&contenido, "\0");


		return contenido;

}

int conectarConKernel() {

	//Handshake
	int socketKernel = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in direccionServ;
	direccionServ.sin_family = AF_INET;
	direccionServ.sin_port = htons(config.PUERTO_KERNEL); // short, Ordenación de bytes de la red
	direccionServ.sin_addr.s_addr = inet_addr(config.IP_KERNEL);
	memset(&(direccionServ.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
	connect(socketKernel, (struct sockaddr*) &direccionServ, sizeof(struct sockaddr));
	send(socketKernel,&identidad, sizeof(int),0);

	return socketKernel;

}

void limpiaMensajes()
{
	system("clear");
}

void imprimeMenuUsuario()
{
	   printf("Ingrese una acción a realizar\n");
			puts("1: Iniciar Programa");
			puts("2: Finalizar Programa");
			puts("3: Desconectar Consola");
			puts("4: Limpiar Mensajes");
}

void* pidePathAlUsuario(char* path)
{

  programa = fopen(path,"rb");
  if(programa == NULL){
	  return NULL;
    }
  else
    {
	  char* archivoTransformado = convertirArchivoACodigo();
	  return archivoTransformado;
    }
}

void recibeOrden(int32_t accion, int socketKernel, struct timeb inicio, struct timeb fin, int pid){
	int32_t tamanioTexto;
	void* buffer;
	int32_t exitCode;
	char* horaInicio;
	char* horaFin;
	char* tiempoTranscurrido;

	switch (accion) {

		case accionError:
			break;

		case accionImprimirTextoConsola:
			recv(socketKernel, &tamanioTexto, sizeof(int32_t), 0);
			buffer = malloc(tamanioTexto);
			recv(socketKernel, buffer, tamanioTexto, 0);
			printf("Impresion por pantalla PID: %d | Mensaje : %s", pid, (char*)buffer);
			free(buffer);
			break;

		case accionConsolaFinalizarNormalmente:
			ftime(&fin);
			horaInicio = transformarTime(inicio);
			horaFin = transformarTime(fin);
			printf("Proceso finalizado correctamente PID: %d\n", pid);
			printf("Inicio: %s\n",horaInicio);
			printf("Fin: %s\n", horaFin);
			tiempoTranscurrido = calcularDuracion(inicio, fin);
			printf("Duracion: %s", tiempoTranscurrido);
			free(horaInicio);
			free(horaFin);
			free(tiempoTranscurrido);
			break;

		case accionConsolaFinalizarErrorInstruccion:
			recv(socketKernel, &exitCode, sizeof(int32_t), 0);
			ftime(&fin);
			horaInicio = transformarTime(inicio);
			horaFin = transformarTime(fin);
			printf("Proceso finalizado con error PID: %d | Exit Code: %d\n", pid, exitCode);
			printf("Inicio: %s\n",horaInicio);
			printf("Fin: %s\n", horaFin);
			tiempoTranscurrido = calcularDuracion(inicio, fin);
			printf("Duracion: %s", tiempoTranscurrido);
			free(horaInicio);
			free(horaFin);
			free(tiempoTranscurrido);
			break;

		default:

			exit(EXIT_FAILURE);
			break;

	}
}

void atenderAcciones(char* programaSolicitado)
{
	struct timeb inicio, fin;
	ftime(&inicio);

	int socketKernel = conectarConKernel();

	int accion = envioScript;
	int longitudPrograma = strlen(programaSolicitado);
	int tamanioBufferCrearPrograma = sizeof(int32_t)+sizeof(int32_t)+(strlen(programaSolicitado));

	void* bufferCrearPrograma = malloc(tamanioBufferCrearPrograma);
	memcpy(bufferCrearPrograma, &accion, sizeof(int32_t));
	memcpy(bufferCrearPrograma + sizeof(int32_t), &longitudPrograma,sizeof(int32_t));
	memcpy(bufferCrearPrograma + sizeof(int32_t)*2, programaSolicitado,strlen(programaSolicitado));

	send(socketKernel,bufferCrearPrograma,tamanioBufferCrearPrograma,0);
	int pidRecibido;
	free(bufferCrearPrograma);
	free(programaSolicitado);
	recv(socketKernel, &pidRecibido, sizeof(int32_t), 0);

	infoThread_t* infoThread = malloc(sizeof(infoThread));
	infoThread->threadId = pthread_self();
	infoThread->socket = socketKernel;
	dictionary_put(infoThreads, string_itoa(pidRecibido), infoThread);

	printf("Proceso iniciado con PID: %d\n", pidRecibido);

	list_add(listaPIDs, pidRecibido);

	while(1)
	{
			int codAccion;
			if(recv(socketKernel, &codAccion, sizeof(int32_t), 0)>0){
				recibeOrden(codAccion, socketKernel, inicio ,fin, pidRecibido);
			}else{
				printf("Proceso con PID %d finalizado por desconexion con Kernel/n", pidRecibido);
				break;
			}

	}
}

void crearPrograma(char* programaSolicitado)
{
	pthread_t hiloPrograma;
	pthread_create(&hiloPrograma, NULL, (void*)atenderAcciones, (void*)programaSolicitado);
}

/****Esta funcion es para probar nada mas**/
void imprimirPIDs(int pid){

	char* new = string_from_format("PID:%d ", pid);
	string_append(&programasExec, new);
	free(new);
}

void imprimirProgramasEnEjecucion(){
	programasExec = string_new();
	list_iterate(listaPIDs,(void*)imprimirPIDs);
	printf("PIDs Impresos: %s", programasExec);
	free(programasExec);
}

void escucharUsuario()
{
	int32_t pid;
	char* programaSolicitado;
	int codAccion;
	int salir=0;

	 while(!salir){

		 if(scanf("%d", &codAccion)==0){
			 scanf("%s", &path); //Lo hago para que borre los caracteres que quedan
			 printf("Numero de operacion invalido\n");
			 continue;
		}

		switch (codAccion) {

			case iniciarPrograma:
				//crea un hilo (programa)

				  printf("Ingresar archivo ansisop: \n");
				  scanf("%s", &path);
				if ((programaSolicitado = pidePathAlUsuario(path)) == NULL){
					puts("No se encontró el archivo\n");
					break;
				}else {
					crearPrograma(programaSolicitado);
					//limpiaMensajes();
					//imprimeMenuUsuario();
					break;
				}

			case finalizarPrograma:
				printf("Ingresar PID: \n");
				if(scanf("%d", &pid)==0){
					scanf("%s", &path);
					printf("PID invalido\n");
				}else{
					infoThread_t* infoThread = dictionary_get(infoThreads, string_itoa(pid));
					if(infoThread==NULL){
						printf("No hay proceso en ejecucion con ese PID\n");
					}else{
						pthread_cancel(infoThread->threadId);
						int32_t codigo = finalizarProgramaAccion;
						void *buffer = malloc(sizeof(int32_t)*2);
						memcpy(buffer,&codigo,sizeof(int32_t));
						memcpy(buffer+sizeof(int32_t),&pid,sizeof(int32_t));
						send(infoThread->socket,buffer,sizeof(int32_t)*2,0);
						free(buffer);
					}
				}
			break;

			case desconectarConsola:
				salir = 1;
			break;

			case limpiarMensajes:
				limpiaMensajes();
			break;

			default:
				printf("Numero de operacion invalido\n");
				break;

			}
	 }
}

void inicializarContexto()
{
	listaPIDs = list_create();
	infoThreads = dictionary_create();
	//sem_init(&mutexCreaPrograma, 1, 1);
	//sem_init(&mutexB, 1, 0);
}

int main(void){

	inicializarContexto();

	limpiaMensajes();

    cargarConfiguracion();

    imprimeMenuUsuario();

    escucharUsuario();

	return EXIT_SUCCESS;
}


