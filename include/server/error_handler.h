#pragma once

#include <server/game.h>

void conn_error(const char* msg);

void game_error(const char* msg, pthread_data* data);