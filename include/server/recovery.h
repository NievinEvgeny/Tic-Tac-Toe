#pragma once

#include <server/game.h>

typedef struct
{
    int sockfd;
    int32_t* games;
    int* player_count;
    pthread_mutex_t* mutexcount;
} recovery_data;

void* recovery(void* thread_data);
