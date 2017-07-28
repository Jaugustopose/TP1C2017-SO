

#ifndef GESTIONDEPROCESOS_H_
#define GESTIONDEPROCESOS_H_

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "deserializador.h"
#include "serializador.h"
#include "kernel.h"
#include "estructurasCompartidas.h"
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include <math.h>

/**
 * Prototipos
 */
void stack_PCB_main(t_PCB* pcb);
void liberarRecursos(t_proceso* proceso);
void planificarExpulsion(t_proceso* proceso);
void escucharConsolaKernel();
void realizarOperacionSobreProceso(t_proceso* proceso);
void rafagasPorProceso(t_proceso* unProceso);
void privilegiadasPorProceso(t_proceso* unProceso);
void obtenerInfoHeapParaProceso(t_proceso* proceso);
void alocacionHEAPPorProceso(t_proceso* unProceso);
void liberacionHEAPPorProceso(t_proceso* unProceso);
void modificarGradoDeMultiprogramacion();


#endif /* GESTIONDEPROCESOS_H_ */
