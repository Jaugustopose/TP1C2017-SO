

#ifndef PRIMITIVASKERNEL_H_
#define PRIMITIVASKERNEL_H_

void recibirWait(int cliente);
void recibirSignal(int cliente);
void primitivaSignal(int cliente, char* semaforoID);
void primitivaWait(int cliente, char* semaforoID);
void obtenerValorCompartida(int cliente);
void obtenerAsignarCompartida(int cliente);
int devolverCompartida(char* compartida);
int asignarCompartida(char* compartida, int valor, int cliente);
void atenderSolicitudMemoriaDinamica();
void atenderLiberacionMemoriaDinamica();

#endif /* PRIMITIVASKERNEL_H_ */
