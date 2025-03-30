#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stdbool.h>

// Manages the UDP connection, responding to various commands

void commands_init(bool *main_thread_stop_var);

void commands_cleanup(void);

#endif