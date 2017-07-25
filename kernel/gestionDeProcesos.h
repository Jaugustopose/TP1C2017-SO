

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

void desasignarCPU(t_proceso* proceso);


#endif /* GESTIONDEPROCESOS_H_ */
