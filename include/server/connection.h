#pragma once

#include <sys/types.h>

#define MAX_PLAYERS 256

extern int player_count;

extern pthread_mutex_t mutexcount;

int setup_listener(int port);

void get_clients(int lis_sockfd, int* cli_sockfd);

void* debug_func(void* thread_data);