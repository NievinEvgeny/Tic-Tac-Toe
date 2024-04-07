#pragma once

#include <server/game.h>

void recovery(int port, int32_t* games, int* player_count, pthread_mutex_t* mutexcount);
