
#include "logger.h"


void crearLog(char* nombreLog, char* nombreProceso, int muestraPantalla)
{
	bool muestra = false;

	if(muestraPantalla > 1 || muestraPantalla < 0){
			perror("Valor incorrecto de muestraPantalla");
		}else{
			switch(muestraPantalla){

				case 1:
					muestra = true;
					break;
				case 0:
					break;
			}
		}


	infoLog = log_create(string_from_format("%s.log",nombreLog),nombreProceso,muestra,LOG_LEVEL_INFO);
	traceLog = log_create(string_from_format("%s.log",nombreLog),nombreProceso,muestra,LOG_LEVEL_INFO);
	debugLog = log_create(string_from_format("%s.log",nombreLog),nombreProceso,muestra,LOG_LEVEL_DEBUG);
	warningLog =  log_create(string_from_format("%s.log",nombreLog),nombreProceso,muestra,LOG_LEVEL_WARNING);
	errorLog =  log_create(string_from_format("%s.log",nombreLog),nombreProceso,muestra,LOG_LEVEL_ERROR);

	log_info(infoLog,"\n\n***** Log de %s *****\n\n",infoLog->program_name);
}

void desactivarLogs(){

	log_info(infoLog, "Desactivando logs...");

	infoLog->detail=LOG_LEVEL_TRACE;
	traceLog->detail=LOG_LEVEL_TRACE;
	debugLog->detail=LOG_LEVEL_TRACE;
	warningLog->detail=LOG_LEVEL_TRACE;
	errorLog->detail=LOG_LEVEL_TRACE;
}


void reactivarLogs(){

	traceLog->detail=LOG_LEVEL_INFO;
	infoLog->detail=LOG_LEVEL_INFO;
	debugLog->detail=LOG_LEVEL_DEBUG;
	warningLog->detail=LOG_LEVEL_WARNING;
	errorLog->detail=LOG_LEVEL_ERROR;

	log_info(infoLog, "Logs reactivados");
}


void destruirLogs(){

	log_destroy(infoLog);
	log_destroy(traceLog);

}
