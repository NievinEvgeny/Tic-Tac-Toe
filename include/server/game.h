#pragma once

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    pthread_mutex_t* mutexcount;
    int* player_count;
    int* cli_sockfd;
    int32_t* game_info;
    int8_t game_id;
} pthread_data;

bool game_on(int32_t game_state);

void* run_game(void* thread_data);