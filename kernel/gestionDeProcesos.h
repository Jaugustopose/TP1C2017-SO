#ifndef GESTIONDEPROCESOS_H_
#define GESTIONDEPROCESOS_H_

#include "kernel.h"

void transformarCodigoToMetadata(t_proceso* proceso);
t_PCB* crearPCB(t_proceso* proceso, int32_t cantidadDePaginas);


void desasignarCPU(t_proceso* proceso);
t_proceso* buscarProcesoPorPID(int PID);
t_proceso* obtenerProceso(int cliente);
t_proceso* buscarProcesoPorPID(int PID);
void stack_PCB_main(t_PCB* pcb);
t_proceso* crearProceso(int pid, int consolaDuenio, char* codigo, int tamanioScript);
void destructorProcesos(t_proceso* unProceso);
void finalizarProceso(t_proceso* proceso);
void bloquearProcesoSem(int cliente, char* semid);
void bloquearProceso(t_proceso* proceso);
void desbloquearProceso(t_proceso* proceso);
void cambiarEstado(t_proceso* proceso, int estado);
void rafagaProceso(int cliente);
void continuarProceso(t_proceso* proceso);
bool terminoQuantum(t_proceso* proceso);
void desasignarCPU(t_proceso* proceso);
void actualizarPCB(t_proceso* proceso, t_PCB* PCB);
t_PCB* recibirPCBDeCPU(int cpu);
void expulsarProceso(t_proceso* proceso);
void liberarRecursos(t_proceso* proceso);
void planificarExpulsion(t_proceso* proceso);
void asignarCPU(t_proceso* proceso, int cpu);
void ejecutarProceso(t_proceso* proceso, int cpu);
void recibirFinalizacion(int cliente);
void recibirAccionDelUsuarioKernel(int32_t orden);
void modificarGradoDeMultiprogramacion(int nuevoGradoMulti);
void queue_iterate(t_queue* self, void (*closure)(void*));
void imprimirPIDenCola(t_proceso* procesoEnCola);
void imprimirPIDenLista(t_proceso* procesoEnLista);
void imprimirColaReady();
void imprimirColaNew();
void imprimirColaBlock();
void imprimirColaExit();
void imprimirTodosLosProcesos();
void imprimir(t_proceso_estado estado);
void rafagasPorProceso(t_proceso* unProceso);
void imprimirRafagas();
void privilegiadasPorProceso(t_proceso* unProceso);
void imprimirPrivilegiadas();
void alocacionHEAPPorProceso(t_proceso* unProceso);
void imprimirAlocacionPaginasHeap();
void liberacionHEAPPorProceso(t_proceso* unProceso);
void imprimirLiberacionPaginasHeap();
void imprimirTablaGlobalDeArchivos(int pid);


#endif /* GESTIONDEPROCESOS_H_ */
