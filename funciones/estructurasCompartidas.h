
#ifndef ESTRUCTURASCOMPARTIDAS_H_
#define ESTRUCTURASCOMPARTIDAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>

typedef enum {

	SOYCONSOLA, //0
	SOYKERNEL, //1
	SOYCPU, //2
	SOYMEMORIA //3

}t_identidad;


typedef enum {
	AccionObtenerPCB, //0
	AccionPedirSentencia, //1
	AccionFinInstruccion, //2
	AccionFinProceso //2
}t_accion;

typedef struct {
	int nroPagina;
	int offset;
	int size;
}t_pedido;

typedef t_list t_stack;

typedef struct {
	int pos;
	t_list* argumentos;
	t_dictionary* identificadores;
	t_puntero posRetorno;
	t_pedido valRetorno;
} t_elemento_stack;

typedef struct {
	int PID;
	int cantidadPaginas;
	int contadorPrograma;
	t_list* indiceCodigo;
	t_stack* stackPointer;
	int exitCode;
}t_PCB;

/*typedef struct {
	int ConsolaDuenio;
	int CpuDuenio;
	t_PCB PCB;
}t_proceso;*/

typedef struct{
	int inicio;
	int fin;
}t_sentencia;

#endif /* ESTRUCTURASCOMPARTIDAS_H_ */
