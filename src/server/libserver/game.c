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

#define END_GAME(game_state) game_state ^= 0x80000000

#define WIN_O(game_state) game_state |= 0x40000000

#define DRAW(game_state) game_state |= 0x20000000

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
        END_GAME(game_state);
        send_game_update(cli_sockfd, game_state);
        error("Player disconnected");
    }
}

static int32_t process_move(int32_t game_state, short turn)
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

static bool win_check(int32_t game_state)
{
    const int win_states[8] = {0x7, 0x38, 0x1C0, 0x124, 0x92, 0x49, 0x111, 0x54};

    for (short i = 0; i < 8; i++)
    {
        if (((game_state >> 16) & win_states[i]) == win_states[i])
        {
            return 1;
        }
        if ((game_state & win_states[i]) == win_states[i])
        {
            return 1;
        }
    }
    return 0;
}

static bool draw_check(int32_t game_state)
{
    const int draw_state = 0x01FF;

    return ((game_state | (game_state >> 16)) & draw_state) == draw_state ? 1 : 0;
}

void* run_game(void* thread_data)
{
    int* cli_sockfd = (int*)thread_data;

    // 31 - state, 30 - who won (1 - O, 0 - X), 29 - draw, 15 - which turn
    int32_t game_state = 0x80000000;

    setup_players_id(cli_sockfd);

    while (game_on(game_state))
    {
        send_game_update(cli_sockfd, game_state);

        short move = 0;
        bool move_valid = 0;

        do
        {
            recv_move(cli_sockfd, game_state, &move);
            move_valid = check_move_valid(game_state, move);
            send_move_validity(cli_sockfd, game_state, move_valid);
        } while (!move_valid);

        game_state = process_move(game_state, move);

        if (win_check(game_state))
        {
            END_GAME(game_state);

            if (PLAYER_ID(game_state))
            {
                WIN_O(game_state);
            }
            send_game_update(cli_sockfd, game_state);
            break;
        }

        if (draw_check(game_state))
        {
            END_GAME(game_state);
            DRAW(game_state);
            send_game_update(cli_sockfd, game_state);
            break;
        }

        SWITCH_TURN(game_state);
    }

    close(cli_sockfd[0]);
    close(cli_sockfd[1]);

    free(cli_sockfd);
    pthread_exit(NULL);
}