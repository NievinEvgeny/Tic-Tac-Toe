#pragma once

#include <sys/types.h>
#include <stdint.h>

typedef struct
{
    int32_t game_state;
    uint8_t game_id;
} game_info;

typedef struct
{
    pthread_mutex_t* mutexcount;
    int* player_count;
    int* cli_sockfd;
    game_info* game_info;
} pthread_data;

void* run_game(void* thread_data);