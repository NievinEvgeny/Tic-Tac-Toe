#pragma once

#include <sys/types.h>

typedef struct
{
    int* player_count;
    int* cli_sockfd;
    pthread_mutex_t* mutexcount;
} pthread_data;

void* run_game(void* thread_data);