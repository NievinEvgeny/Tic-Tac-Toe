#include <server/recovery.h>
#include <server/connection.h>
#include <server/error_handler.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

void recovery(int lis_sockfd, int32_t* games, int* player_count, pthread_mutex_t* mutexcount)
{
    int** players = (int**)malloc(sizeof(int*) * MAX_PLAYERS);

    for (uint8_t i = 0; i < MAX_PLAYERS / 2; i++)
    {
        players[i] = (int*)malloc(sizeof(int) * 2);
    }

    int players_cnt[MAX_PLAYERS / 2] = {0};

    time_t start_time = time(NULL);

    while ((time(NULL) - start_time) <= 1)
    {
        int client_sockfd = accept(lis_sockfd, NULL, NULL);

        if (client_sockfd < 0)
        {
            continue;
        }

        int8_t game_id = -1;
        recv(client_sockfd, &game_id, sizeof(game_id), 0);

        if (game_id < 0)
        {
            continue;
        }

        players[game_id][players_cnt[game_id]++] = client_sockfd;
    }

    for (uint8_t game_id = 0; game_id < MAX_PLAYERS / 2; game_id++)
    {
        if (players_cnt[game_id] > 0)
        {
            pthread_t new_thread;

            pthread_data* data = (pthread_data*)malloc(sizeof(pthread_data));
            data->mutexcount = mutexcount;
            data->player_count = player_count;
            data->cli_sockfd = players[game_id];
            data->game_info = &games[game_id];

            int result = pthread_create(&new_thread, NULL, run_game, (void*)data);

            if (result)
            {
                printf("Thread creation failed with return code %d\n", result);
                exit(-1);
            }
        }
    }
}