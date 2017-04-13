/*
 * memo.h
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */
#ifndef MEMO_H_
#define MEMO_H_
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "commons/config.h"
//#include "memo.cfg"

typedef struct configMemo {
	char* ipKernel;
	int puertoKernel;
} config_t;

t_config* configMemo;
config_t config;

#endif /* MEMO_H_ */
