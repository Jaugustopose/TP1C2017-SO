/*
 * capaConexionCPU.h
 *
 *  Created on: 27/4/2017
 *      Author: utnso
 */

#ifndef CAPACONEXIONCPU_H_
#define CAPACONEXIONCPU_H_

#include <commons/string.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <parser/parser.h>
#include <string.h>

#include "handshake.h"
#include "cliente-servidor.h"
#include "cpu.h"

//Se conecta como cliente de los siguientes procesos

int memoria;
int kernel;
struct sockaddr_in* dirKernel;
struct sockaddr_in* dirMemoria;

int getHandshake(int cliente);
void conectarConKernel();
void conectarConMemoria();
int charToInt(char *c);

#endif
