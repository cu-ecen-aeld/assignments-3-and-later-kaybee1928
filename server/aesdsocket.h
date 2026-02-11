//
// Created by kashribha on 2/11/26.
//
#include <stddef.h>

#ifndef ASSIGNMENTS_3_AND_LATER_KAYBEE1928_AESDSOCKET_H
#define ASSIGNMENTS_3_AND_LATER_KAYBEE1928_AESDSOCKET_H

#define PORT "9000"
#define BACKLOG 5

void signal_handler(int signum);
char* append_char(char *buffer, size_t *current_size, size_t *used_size, char c);
void server();

#endif //ASSIGNMENTS_3_AND_LATER_KAYBEE1928_AESDSOCKET_H