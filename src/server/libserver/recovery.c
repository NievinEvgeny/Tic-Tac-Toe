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

void recovery(int lis_sockfd, game_info* games, int* player_count, pthread_mutex_t* mutexcount)
{
    int players[MAX_PLAYERS][2] = {0};
    int players_cnt[MAX_PLAYERS] = {0};

    time_t start_time = time(NULL);

    while ((time(NULL) - start_time) <= 10)
    {
        int cli_sockfd = accept(lis_sockfd, NULL, NULL);

        if (cli_sockfd < 0)
        {
            continue;
        }

        int8_t game_id = -1;
        recv(cli_sockfd, &game_id, sizeof(game_id), 0);

        if (game_id < 0)
        {
            continue;
        }

        players[game_id][players_cnt[game_id]++] = cli_sockfd;

        if (players_cnt[game_id] == 2)
        {
            pthread_t new_thread;

            int* cli_sockfd = (int*)malloc(2 * sizeof(int));
            memset(cli_sockfd, 0, 2 * sizeof(int));

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