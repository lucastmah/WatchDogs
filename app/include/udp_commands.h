#ifndef UDP_COMMANDS_H_
#define UDP_COMMANDS_H_

#include <stdbool.h>

// Manages the UDP connection, responding to various commands

void UDPCommands_init(bool *main_thread_stop_var);

void UDPCommands_cleanup(void);

#endif