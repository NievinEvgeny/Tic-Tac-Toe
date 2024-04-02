#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t ip;
    int port;
} server_info;

void get_servers_info(const char* filename, server_info* servers_info, uint64_t servers_num);

int connect_to_server(uint32_t ip, int port);

int connect_to_primary_server(server_info* servers_info, uint64_t servers_num, uint64_t* cur_server, bool is_recovery);