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

void cargarConfiguracion(char *path){

		/*char* path = string_new();
		char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
		string_append(&path, getcwd(cwd, sizeof(cwd)));
		if (string_contains(path, "/Debug")) {
			string_append(&path, "/consola.cfg");
		} else {
			string_append(&path, "/Debug/consola.cfg");
		}*/
		t_config* configConsola = config_create(path);
		//free(path);
		if (config_has_property(configConsola, "IP_KERNEL")){
				config.IP_KERNEL = config_get_string_value(configConsola,"IP_KERNEL");
				log_info(ConsolaConsoleLogger, "config.IP_KERNEL: %s", config.IP_KERNEL);
		}

		if (config_has_property(configConsola, "PUERTO_KERNEL")){
				config.PUERTO_KERNEL = config_get_int_value(configConsola,"PUERTO_KERNEL");
				log_info(ConsolaConsoleLogger, "config.PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);
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
		log_info(consolaLogger, "Se ha convertido a código un archivo ansisop");

}

int conectarConKernel() {

	//Handshake
	int seConecto = 0;
	int socketKernel = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in direccionServ;
	direccionServ.sin_family = AF_INET;
	direccionServ.sin_port = htons(config.PUERTO_KERNEL); // short, Ordenación de bytes de la red
	direccionServ.sin_addr.s_addr = inet_addr(config.IP_KERNEL);
	memset(&(direccionServ.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
	seConecto = connect(socketKernel, (struct sockaddr*) &direccionServ, sizeof(struct sockaddr));
	send(socketKernel,&identidad, sizeof(int),MSG_WAITALL);
	if(seConecto == 0){
		log_info(consolaLogger, "Consola conectada con Kernel exitosamente");
	}
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
//	char* ruta = strdup("/home/utnso/scripts/");
//	string_append(&ruta, path);
//	string_append(&ruta, ".ansisop");
	programa = fopen(path,"rb");
	//free(ruta);
	if(programa == NULL){
		return NULL;
    }
	else
    {
		char* archivoTransformado = convertirArchivoACodigo();
		return archivoTransformado;
    }
	log_info(consolaLogger, "Se ha introducido la ruta del programa ansisop: %s", path);
}

void recibeOrden(int32_t accion, int socketKernel, struct timeb inicio, struct timeb fin, int pid, int* seguir, int* cantPrintf){
	int32_t tamanioTexto;
	void* buffer;
	int32_t exitCode;
	char* horaInicio;
	char* horaFin;
	char* tiempoTranscurrido;

	switch (accion) {

		case accionImprimirTextoConsola:
			recv(socketKernel, &tamanioTexto, sizeof(int32_t), MSG_WAITALL);
			buffer = malloc(tamanioTexto);
			recv(socketKernel, buffer, tamanioTexto, MSG_WAITALL);
			log_info(ConsolaConsoleLogger, "Impresion por pantalla PID: %d | Mensaje : ", pid);
			char *texto = buffer;
			int i;
			for (i = 0; i < tamanioTexto; ++i) {
				if(i==0)
					log_info(ConsolaConsoleLogger, "|");
				log_info(ConsolaConsoleLogger, "%d|",(int)texto[i]);
			}
			log_info(ConsolaConsoleLogger,"\n");
			free(buffer);
			(*cantPrintf)++;
			break;

		case accionConsolaFinalizarNormalmente:
			ftime(&fin);
			horaInicio = transformarTime(inicio);
			horaFin = transformarTime(fin);
			log_info(ConsolaConsoleLogger, "Proceso finalizado correctamente PID: %d\n", pid);
			log_info(ConsolaConsoleLogger, "Inicio: %s\n",horaInicio);
			log_info(ConsolaConsoleLogger, "Fin: %s\n", horaFin);
			tiempoTranscurrido = calcularDuracion(inicio, fin);
			log_info(ConsolaConsoleLogger, "Duracion: %s\n", tiempoTranscurrido);
			log_info(ConsolaConsoleLogger, "Cantidad de impresiones en pantalla: %d\n", *cantPrintf);
			free(horaInicio);
			free(horaFin);
			free(tiempoTranscurrido);
			*seguir = 0;
			break;

		case accionConsolaFinalizarErrorInstruccion:
			recv(socketKernel, &exitCode, sizeof(int32_t), MSG_WAITALL);
			ftime(&fin);
			horaInicio = transformarTime(inicio);
			horaFin = transformarTime(fin);
			log_info(ConsolaConsoleLogger, "Proceso finalizado con error PID: %d | Exit Code: %d\n", pid, exitCode);
			log_info(ConsolaConsoleLogger, "Inicio: %s\n",horaInicio);
			log_info(ConsolaConsoleLogger, "Fin: %s\n", horaFin);
			tiempoTranscurrido = calcularDuracion(inicio, fin);
			log_info(ConsolaConsoleLogger, "Duracion: %s\n", tiempoTranscurrido);
			log_info(ConsolaConsoleLogger, "Cantidad de impresiones en pantalla: %d\n", *cantPrintf);
			free(horaInicio);
			free(horaFin);
			free(tiempoTranscurrido);
			*seguir = 0;
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

	send(socketKernel,bufferCrearPrograma,tamanioBufferCrearPrograma, MSG_WAITALL);
	int pidRecibido;
	free(bufferCrearPrograma);
	free(programaSolicitado);
	recv(socketKernel, &pidRecibido, sizeof(int32_t), MSG_WAITALL);

	infoThread_t* infoThread = malloc(sizeof(infoThread));
	infoThread->threadId = pthread_self();
	infoThread->socket = socketKernel;
	dictionary_put(infoThreads, string_itoa(pidRecibido), infoThread);

	log_info(consolaLogger, "Proceso iniciado con PID: %d\n", pidRecibido);

	//list_add(listaPIDs, pidRecibido);
	int* seguir = malloc(sizeof(int));
	*seguir = 1;

	int* cantPrintf= malloc(sizeof(int));
	*cantPrintf = 0;

	while(*seguir)
	{
			int codAccion;
			int recibido = recv(socketKernel, &codAccion, sizeof(int32_t), MSG_WAITALL);
			if(recibido >0){
				recibeOrden(codAccion, socketKernel, inicio ,fin, pidRecibido, seguir, cantPrintf);
			}else{
				log_info(ConsolaConsoleLogger, "Proceso con PID %d finalizado por desconexion con Kernel/n", pidRecibido);
				break;
			}

	}
	free(cantPrintf);
	free(seguir);
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
	log_info(ConsolaConsoleLogger, "PIDs Impresos: %s", programasExec);
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
			 log_info(consolaLogger, "El usuario ingreso un número de operacion invalido\n");
			 continue;
		}

		switch (codAccion) {

			case iniciarPrograma:
				//crea un hilo (programa)

				  printf("Ingresar archivo ansisop: \n");
				  scanf("%s", &path);
				if ((programaSolicitado = pidePathAlUsuario(path)) == NULL){
					log_info(ConsolaConsoleLogger, "No se encontró el archivo\n");
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
					log_info(ConsolaConsoleLogger, "Se ha querida finalizar un programa con PID invalido\n");
				}else{
					infoThread_t* infoThread = dictionary_get(infoThreads, string_itoa(pid));
					if(infoThread==NULL){
						log_info(ConsolaConsoleLogger, "No existe proceso en ejecucion con ese PID\n");
					}else{
						//pthread_cancel(infoThread->threadId);
						int32_t codigo = finalizarProgramaAccion;
						void *buffer = malloc(sizeof(int32_t)*2);
						memcpy(buffer,&codigo,sizeof(int32_t));
						memcpy(buffer+sizeof(int32_t),&pid,sizeof(int32_t));
						send(infoThread->socket,buffer,sizeof(int32_t)*2,MSG_WAITALL);
						free(buffer);
					}
				}
			break;

			case desconectarConsola:
				salir = 1;
				log_info(consolaLogger, "El usuario ha desconectado la consola");
				log_destroy(consolaLogger);
				log_destroy(ConsolaConsoleLogger);
			break;

			case limpiarMensajes:
				limpiaMensajes();
			break;

			default:
				log_info(ConsolaConsoleLogger, "Numero de operacion invalido\n");
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
	log_info(consolaLogger, "Se ha inicializado el contexto correctamente");
}

void inicializarLog() {
	char* directorioOutputConsola = "output";
	char* consolaLogFileName = "consola";
	char* filepath = string_new();
	string_append(&filepath, directorioOutputConsola);
	string_append(&filepath, "/");
	string_append(&filepath, consolaLogFileName);
	consolaLogger = log_create(string_from_format("%s.log", filepath), "Consola", false, LOG_LEVEL_INFO);
	ConsolaConsoleLogger = log_create(string_from_format("%s.log", filepath), "Consola", true, LOG_LEVEL_INFO);
	free (filepath);
}

int main(int argc, char *argv[]){

	if(argc>1){

		/****Si no existe el directorio de output lo crea****/
		struct stat st = {0};
		if (stat(directorioOutputConsola, &st) == -1) {
			mkdir(directorioOutputConsola, 0700);
		}
		/****************************************************/

		inicializarLog();

		inicializarContexto();

		limpiaMensajes();

		cargarConfiguracion(argv[1]);
		imprimeMenuUsuario();

		escucharUsuario();
	}else{
		printf("Por favor, ejecute el programa especificando la ruta del archivo .cfg\n");
	}

	return EXIT_SUCCESS;
}


