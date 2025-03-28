// Module for UDP server
#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

#include <stdbool.h>

void UDPServer_init(bool* stop_var);
void UDPServer_cleanup(void);

#endif