#include <client/game.h>
#include <client/connection.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Error arguments\nExample: %s \"file_with_servers_info.txt\"\n", argv[0]);
        exit(0);
    }

    const uint64_t servers_num = 3;

    server_info servers_info[servers_num];

    get_servers_info(argv[1], servers_info, servers_num);

    play_game(servers_info, servers_num);

    return 0;
}