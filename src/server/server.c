#include <server/connection.h>
#include <server/error_handler.h>
#include <server/game.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int player_count = 0;

    pthread_mutex_t mutexcount;

    if (argc != 2)
    {
        fprintf(stderr, "Error arguments\nExample: %s port\n", argv[0]);
        exit(1);
    }

    int lis_sockfd = setup_listener(atoi(argv[1]));
    pthread_mutex_init(&mutexcount, NULL);

    while (1)
    {
        if (player_count < MAX_PLAYERS)
        {
            int* cli_sockfd = (int*)malloc(2 * sizeof(int));
            memset(cli_sockfd, 0, 2 * sizeof(int));

            get_clients(lis_sockfd, cli_sockfd, &player_count, &mutexcount);

            pthread_t new_thread;

            int result = pthread_create(&new_thread, NULL, run_game, (void*)cli_sockfd);

            if (result)
            {
                printf("Thread creation failed with return code %d\n", result);
                exit(-1);
            }
        }
    }

    close(lis_sockfd);

    pthread_mutex_destroy(&mutexcount);
    pthread_exit(NULL);
}