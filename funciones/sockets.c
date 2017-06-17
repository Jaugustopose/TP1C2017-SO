
#include "sockets.h"

t_paquete* crear_paquete(void* data)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->tamanio = strlen(data);
	paquete->contenido = malloc(paquete->tamanio);
	strcpy(paquete->contenido,data);

	return paquete;
}

int32_t tamanio_paquete(t_paquete* paquete)
{
	return paquete->tamanio + 2*sizeof(int32_t);
}

void* serializar_paquete(t_paquete* paquete)
{
	char *serializado = malloc(tamanio_paquete(paquete)); //Malloc del tamaño a guardar

	int32_t offset = 0;
	int32_t tamanio_a_enviar;

	tamanio_a_enviar = sizeof(u_int32_t);
	memcpy(serializado + offset, &(paquete->tamanio), tamanio_a_enviar);
	offset += tamanio_a_enviar;

	tamanio_a_enviar = paquete->tamanio;
	memcpy(serializado + offset, paquete->contenido, tamanio_a_enviar);
	offset += tamanio_a_enviar;

	return serializado;
}

t_paquete* deserializar_paquete(char* serializado)
{
	u_int32_t tamanio_recibido;
	int32_t offset = 0;
	t_paquete* package = malloc(sizeof(t_paquete));

	memcpy(&tamanio_recibido, serializado, sizeof(u_int32_t));
	package->tamanio = tamanio_recibido;

	offset += sizeof(u_int32_t);
	package->contenido = malloc(package->tamanio);
	memcpy(package->contenido, serializado + offset, package->tamanio);

	return package;
}

int32_t _receive(t_sock* socket, void* buffer, uint32_t len)
{
	return recv(socket->fd, buffer, len, 0);
}

int32_t _send(t_sock* socket, void* buffer, uint32_t len)
{
	return send(socket->fd, buffer, len, 0);
}

int32_t send_bytes(t_sock* socket, void* buffer, u_int32_t len)
{
	//me aseguro que se envie toda la informacion

    int32_t total = 0;
    int32_t bytesLeft = len;
    int32_t n;

    while(total<len)
    {
        n = send(socket->fd, buffer+total, bytesLeft, 0);
        if(n==-1) break;
        total+=n;
        bytesLeft-=n;
    }

    len = total;

    return n==-1?-1:0;
}



int32_t recv_bytes(t_sock* sock, void* bufferSalida, uint32_t lenBuffer)
{
	int32_t n = 0;
	int32_t bytesLeft = lenBuffer;
	int32_t recibido = 0;

	while(recibido < lenBuffer)
	{
		n = _receive(sock, bufferSalida+recibido, lenBuffer-recibido);
		if(n==-1 || n==0) break;
		recibido += n;
//		bytesLeft -= n;
	}

	lenBuffer = recibido;

	return (n==-1 || n==0)?-1:0;
}


int32_t send_seguro(t_sock* sock, void* msg)
{
	t_paquete* package = crear_paquete(msg);

	void* serializado = serializar_paquete(package);

	int32_t serializado_len = tamanio_paquete(package);

	int32_t n = send_bytes(sock, serializado, serializado_len);

	return n==-1?-1:0;
}
