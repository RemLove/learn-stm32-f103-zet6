#ifndef TCP_H
#define TCP_H

#include "socket.h"
#include "eth.h"
#include "string.h"
#include "stdio.h"

#define CLIENT 0
#define SERVER 1
#define ROLE SERVER
//socket端口号
#define Sn 0
void TCP_ServerStart(void);

void TCP_SendData(uint8_t data[], uint16_t len);

void TCP_ReceiveData(uint8_t data[], uint16_t* len);

void TCP_ClientStart(void);

#endif /* TCP_H */
