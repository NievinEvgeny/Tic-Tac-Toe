#include <server/error_handler.h>
#include <server/game.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void error(const char* msg)
{
    perror(msg);
    pthread_exit(NULL);
}

void game_error(const char* msg, pthread_data* data)
{
    perror(msg);

    close(data->cli_sockfd[0]);
    close(data->cli_sockfd[1]);

    pthread_mutex_lock(data->mutexcount);
    *data->player_count -= 2;
    pthread_mutex_unlock(data->mutexcount);

    free(data->cli_sockfd);
    free(data);
    pthread_exit(NULL);
}