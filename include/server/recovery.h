#pragma once

#include <server/game.h>

void recovery(int port, game_info* games, int* player_count, pthread_mutex_t* mutexcount);
