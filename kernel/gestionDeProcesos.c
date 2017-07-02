#include "gestionDeProcesos.h"


void transformarCodigoToMetadata(t_PCB* pcb, char* cod)

{
	t_metadata_program* metadata = metadata_desde_literal(cod);


	    //Llena indice de codigo
		int i;
		for (i = 0; i < metadata->instrucciones_size; i++) {
				t_sentencia sentencia;
				sentencia.inicio = metadata->instrucciones_serializado[i].start;
				sentencia.fin = sentencia.inicio + metadata->instrucciones_serializado[i].offset;
				list_add(pcb->indiceCodigo, &sentencia);
			}

		//Llena indice de etiquetas
		int longitud = 0;

			for (i = 0; i < metadata->etiquetas_size; i++) {

				if (metadata->etiquetas[i] == '\0') {

					char* etiqueta = malloc(longitud + 1);
					memcpy(etiqueta, metadata->etiquetas + i - longitud, longitud + 1);

					int* salto = malloc(sizeof(int));
					memcpy(salto, metadata->etiquetas + i + 1, sizeof(int));

					dictionary_put(pcb->indiceEtiquetas, etiqueta, salto);
					i = i + sizeof(int);
					longitud = 0;

					free(etiqueta);
					free(salto);

				} else
					longitud++;
			}


		free(metadata);
}

t_PCB* crearPCB()
{
	t_PCB* pcb = malloc(sizeof(t_PCB));

	pcb->PID=0;
	pcb->contadorPrograma = 0;
	pcb->cantidadPaginas = 0;

	pcb->stackPointer = stack_crear();
	pcb->indiceCodigo = list_create();
	pcb->indiceEtiquetas = dictionary_create();

	return pcb;
}

t_proceso* crearProceso(int pid, int consolaDuenio, char* codigo)
{
	t_proceso* proceso = malloc(sizeof(t_PCB));
	proceso->PCB = crearPCB();
	proceso->PCB->PID = pid;
	proceso->ConsolaDuenio = consolaDuenio;
	proceso->CpuDuenio = -1;


	transformarCodigoToMetadata(proceso->PCB, codigo);

	return proceso;
}
