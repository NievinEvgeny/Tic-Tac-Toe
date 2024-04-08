#include <server/connection.h>
#include <server/error_handler.h>
#include <server/game.h>
#include <server/synchronization.h>
#include <server/recovery.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Error arguments\nExample: %s \"file_with_servers_info.txt\" port\n", argv[0]);
        exit(1);
    }

    int player_count = 0;

    pthread_mutex_t mutexcount;

    int32_t games[MAX_PLAYERS / 2] = {0};

    bool was_slave = false;
    bool is_primary = false;
    const int port = atoi(argv[2]);
    const char* servs_filename = argv[1];
    const uint64_t servs_num = 3;

    synchronization_data sync_data = {servs_filename, servs_num, port, &is_primary, &was_slave, games};

    pthread_t sync_thread;

    int sync_result = pthread_create(&sync_thread, NULL, synchronization, (void*)&sync_data);

    if (sync_result)
    {
        printf("Thread creation failed with return code %d\n", sync_result);
        exit(-1);
    }

    int rec_lis_sockfd = setup_listener(port + 100);
    listen(rec_lis_sockfd, MAX_PLAYERS);

    while (!is_primary)
    {
    }

    if (was_slave)
    {
        recovery_data rec_data = {rec_lis_sockfd, games, &player_count, &mutexcount};

        pthread_t rec_thread;

        int rec_result = pthread_create(&rec_thread, NULL, recovery, (void*)&rec_data);

        if (rec_result)
        {
            printf("Thread creation failed with return code %d\n", rec_result);
            exit(-1);
        }
    }

    int lis_sockfd = setup_listener(port);
    pthread_mutex_init(&mutexcount, NULL);

    for (uint8_t cur_game_id = 0;; cur_game_id++)
    {
        if ((player_count < MAX_PLAYERS) && (!game_on(games[cur_game_id % (MAX_PLAYERS / 2)])))
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
            data->game_id = cur_game_id;

            int result = pthread_create(&new_thread, NULL, run_game, (void*)data);

            if (result)
            {
                printf("Thread creation failed with return code %d\n", result);
                exit(-1);
            }
        }
    }

    close(rec_lis_sockfd);
    close(lis_sockfd);

    pthread_mutex_destroy(&mutexcount);
    pthread_exit(NULL);
}