#include <client/connection.h>
#include <client/game.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Error arguments\nExample: %s ip port\n", argv[0]);
        exit(0);
    }

    int sockfd = connect_to_server(argv[1], atoi(argv[2]));

    play_game(sockfd);

    close(sockfd);
    return 0;
}