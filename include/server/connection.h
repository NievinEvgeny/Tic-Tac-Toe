#pragma once

#include <sys/types.h>
#include <stdint.h>

#define MAX_PLAYERS 256

int connect_to_server(uint32_t ip, int port);

int setup_listener(int port);

void get_clients(int lis_sockfd, int* cli_sockfd, int* player_count, pthread_mutex_t* mutexcount);