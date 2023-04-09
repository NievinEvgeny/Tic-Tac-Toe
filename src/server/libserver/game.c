#include <server/game.h>
#include <server/error_handler.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <netinet/in.h>

#define PLAYER_ID(game_state) (game_state & 0x8000) >> 15

#define SWITCH_TURN(game_state) game_state ^= 0x8000

static bool game_on(int32_t game_state)
{
    return game_state & 0x80000000;
}

static void setup_players_id(int* cli_sockfd)
{
    bool idX = 0, idO = 1;

    if ((send(cli_sockfd[0], &idX, sizeof(bool), 0) == -1) || (send(cli_sockfd[1], &idO, sizeof(bool), 0) == -1))
    {
        error("Can't send ids to clients");
    }
}

static void send_game_update(int* cli_sockfd, int32_t game_state)
{
    if ((send(cli_sockfd[0], &game_state, sizeof(int32_t), 0) == -1)
        || (send(cli_sockfd[1], &game_state, sizeof(int32_t), 0) == -1))
    {
        error("Can't send update to clients");
    }
}

static void recv_move(int* cli_sockfd, int32_t game_state, short* move)
{
    if (recv(cli_sockfd[PLAYER_ID(game_state)], move, sizeof(short), 0) == -1)
    {
        error("Can't get move from client");
    }
}

void* run_game(void* thread_data)
{
    int* cli_sockfd = (int*)thread_data;

    // 31 - state, 15 - which turn
    int32_t game_state = 0x80008000;

    short move;

    setup_players_id(cli_sockfd);

    while (game_on(game_state))
    {
        send_game_update(cli_sockfd, game_state);

        recv_move(cli_sockfd, game_state, &move);

        SWITCH_TURN(game_state);
    }

    close(cli_sockfd[0]);
    close(cli_sockfd[1]);

    free(cli_sockfd);
    pthread_exit(NULL);
}