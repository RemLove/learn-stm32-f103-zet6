#ifndef UDP_H
#define UDP_H

#include "socket.h"
#include "eth.h"
#include "string.h"
#include "stdio.h"


//socket端口号
#define Sn 0
void UDP_Start(void);

void UDP_SendData(uint8_t data[], uint16_t len, uint8_t dest_ip[4], uint16_t dest_port);

void UDP_ReceiveData(uint8_t data[], uint16_t* len, uint8_t* src_ip, uint16_t* src_port);


#endif /* UDP_H */
