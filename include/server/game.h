#pragma once

#include <sys/types.h>

typedef struct
{
    pthread_mutex_t* mutexcount;
    int* player_count;
    int* cli_sockfd;
} pthread_data;

void* run_game(void* thread_data);