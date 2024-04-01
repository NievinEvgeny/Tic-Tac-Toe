#include <server/connection.h>
#include <server/error_handler.h>
#include <server/game.h>
#include <server/synchronization.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Error arguments\nExample: %s port\n", argv[0]);
        exit(1);
    }

    int player_count = 0;

    pthread_mutex_t mutexcount;

    game_info games[MAX_PLAYERS / 2] = {0};

    for (short id = 0; id < MAX_PLAYERS / 2; id++)
    {
        games[id].game_id = id;
    }

    bool is_primary = false;
    const char* servs_filename = "servers_info.dat";
    const uint64_t servs_num = 3;

    synchronization_data sync_data = {servs_filename, servs_num, atoi(argv[1]), &is_primary, games};

    pthread_t sync_thread;

    int sync_result = pthread_create(&sync_thread, NULL, synchronization, (void*)&sync_data);

    if (sync_result)
    {
        printf("Thread creation failed with return code %d\n", sync_result);
        exit(-1);
    }

    while (!is_primary)
    {
    }

    // TODO recovery

    int lis_sockfd = setup_listener(atoi(argv[1]));
    pthread_mutex_init(&mutexcount, NULL);

    for (uint8_t cur_game_id = 0;; cur_game_id++)
    {
        if ((player_count < MAX_PLAYERS) && (!game_on(games[cur_game_id % (MAX_PLAYERS / 2)].game_state)))
        {
            int* cli_sockfd = (int*)malloc(2 * sizeof(int));
            memset(cli_sockfd, 0, 2 * sizeof(int));

            get_clients(lis_sockfd, cli_sockfd, &player_count, &mutexcount);

            pthread_t new_thread;

            pthread_data* data = (pthread_data*)malloc(sizeof(pthread_data));
            data->mutexcount = &mutexcount;
            data->player_count = &player_count;
            data->cli_sockfd = cli_sockfd;
            data->game_info = &games[cur_game_id % (MAX_PLAYERS / 2)];

            int result = pthread_create(&new_thread, NULL, run_game, (void*)data);

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