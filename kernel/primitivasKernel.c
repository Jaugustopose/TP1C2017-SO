

void primitivaSignal(int cliente, char* semaforoID)
{
	t_semaforo* semaforo = (t_semaforo*)dictionary_get(tablaSemaforos, semaforoID);

		if (!queue_is_empty(semaforo->colaSemaforo)){

			t_proceso* proceso = queue_pop(semaforo->colaSemaforo);
			//TODO: IMPLEMENTAR! desbloquearProceso(proceso);
		}
		else{
			semaforo->valorSemaforo++;
		}
}

void primitivaWait(int cliente, char* semaforoID)
{
	t_semaforo* semaforo = (t_semaforo*)dictionary_get(tablaSemaforos, semaforoID);

	if (semaforo->valorSemaforo > 0){
		semaforo->valorSemaforo--;
		}
	else{
		//TODO: IMPLEMENTAR! bloquearProcesoSem(cliente, semaforoID);
	}
}
int primitivaDevolverCompartida()
{

}
void primitivaAsignarCompartida()
{

}
void imprimir()
{

}
