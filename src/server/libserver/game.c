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
    int msg_len = recv(cli_sockfd[PLAYER_ID(game_state)], move, sizeof(short), 0);

    if (msg_len < 0)
    {
        error("Can't get move from client");
    }

    if (msg_len == 0)
    {
        error("Player disconnected");
    }
}

static int32_t update_game_state(int32_t game_state, short turn)
{
    if (PLAYER_ID(game_state))
    {
        return game_state | (1 << (turn + 16));
    }
    return game_state | (1 << turn);
}

static bool check_move_valid(int32_t game_state, short move)
{
    return (game_state | (game_state >> 16)) & (1 << move) ? 0 : 1;
}

static void send_move_validity(int* cli_sockfd, int32_t game_state, bool move_valid)
{
    if (send(cli_sockfd[PLAYER_ID(game_state)], &move_valid, sizeof(bool), 0) == -1)
    {
        error("Can't send move validity to server");
    }
}

void* run_game(void* thread_data)
{
    int* cli_sockfd = (int*)thread_data;

    // 31 - state, 15 - which turn
    int32_t game_state = 0x80000000;

    setup_players_id(cli_sockfd);

    while (game_on(game_state))
    {
        short move = 0;
        bool move_valid = 0;

        send_game_update(cli_sockfd, game_state);

        do
        {
            recv_move(cli_sockfd, game_state, &move);
            move_valid = check_move_valid(game_state, move);
            send_move_validity(cli_sockfd, game_state, move_valid);
        } while (!move_valid);

        game_state = update_game_state(game_state, move);

        SWITCH_TURN(game_state);
    }

    close(cli_sockfd[0]);
    close(cli_sockfd[1]);

    free(cli_sockfd);
    pthread_exit(NULL);
}