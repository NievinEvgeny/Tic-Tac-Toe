#pragma once

#include <sys/types.h>

#define MAX_PLAYERS 256

int setup_listener(int port);

void get_clients(int lis_sockfd, int* cli_sockfd, int* player_count, pthread_mutex_t* mutexcount);