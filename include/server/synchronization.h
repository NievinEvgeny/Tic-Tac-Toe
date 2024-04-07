#pragma once

#include <server/game.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t ip;
    int port;
} server_info;

typedef struct
{
    int sock_fd;
    bool is_connected;
} conn_server_info;

typedef struct
{
    const char* servers_filename;
    uint64_t servers_num;
    int port;
    bool* is_primary;
    bool* was_slave;
    int32_t* games;
} synchronization_data;

void* synchronization(void* thread_data);