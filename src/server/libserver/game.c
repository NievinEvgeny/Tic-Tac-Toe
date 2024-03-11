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

static bool win_check(int32_t game_state)
{
    const int win_states[8] = {0x7, 0x38, 0x1C0, 0x124, 0x92, 0x49, 0x111, 0x54};

    for (short i = 0; i < 8; i++)
    {
        if ((((game_state >> 16) & win_states[i]) == win_states[i]) || ((game_state & win_states[i]) == win_states[i]))
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

static bool setup_players_id(int* cli_sockfd)
{
    const bool idX = 0, idO = 1;

    return (send(cli_sockfd[0], &idX, sizeof(bool), 0) == -1) || (send(cli_sockfd[1], &idO, sizeof(bool), 0) == -1);
}

static bool send_game_state(int cli_sockfd, int32_t game_state)
{
    return send(cli_sockfd, &game_state, sizeof(int32_t), 0) == -1;
}

static bool send_game_update(int* cli_sockfd, int32_t game_state)
{
    return send_game_state(cli_sockfd[0], game_state) || send_game_state(cli_sockfd[1], game_state);
}

static void recv_move(pthread_data* data, int32_t game_state, short* move)
{
    int msg_len = recv(data->cli_sockfd[PLAYER_ID(game_state)], move, sizeof(short), 0);

    if (msg_len < 0)
    {
        game_error("Can't get move from client", data);
    }

    if (msg_len == 0)
    {
        END_GAME(game_state);

        if (PLAYER_ID(game_state) ^ 1)
        {
            WIN_O(game_state);
        }

        send_game_state(data->cli_sockfd[PLAYER_ID(game_state) ^ 1], game_state);

        game_error("Player disconnected", data);
    }
}

static int32_t process_move(int32_t game_state, short move)
{
    return PLAYER_ID(game_state) ? game_state | (1 << (move + 16)) : game_state | (1 << move);
}

static bool check_move_valid(int32_t game_state, short move)
{
    return (game_state | (game_state >> 16)) & (1 << move) ? 0 : 1;
}

static bool send_move_validity(int cli_sockfd, bool move_valid)
{
    return send(cli_sockfd, &move_valid, sizeof(bool), 0) == -1;
}

void* run_game(void* thread_data)
{
    pthread_data* data = (pthread_data*)thread_data;

    // 31 - state, 30 - who won (1 - O, 0 - X), 29 - draw, 15 - which turn
    int32_t game_state = 0x80000000;

    if (setup_players_id(data->cli_sockfd))
    {
        game_error("Can't send ids to clients", data);
    }

    while (game_on(game_state))
    {
        if (send_game_update(data->cli_sockfd, game_state))
        {
            game_error("Can't send update to client", data);
        }

        short move = 0;
        bool move_valid = 0;

        do
        {
            recv_move(data, game_state, &move);
            move_valid = check_move_valid(game_state, move);

            if (send_move_validity(data->cli_sockfd[PLAYER_ID(game_state)], move_valid))
            {
                game_error("Can't send move validity to client", data);
            }

        } while (!move_valid);

        game_state = process_move(game_state, move);

        if (win_check(game_state))
        {
            END_GAME(game_state);

            if (PLAYER_ID(game_state))
            {
                WIN_O(game_state);
            }

            send_game_update(data->cli_sockfd, game_state);
            break;
        }

        if (draw_check(game_state))
        {
            END_GAME(game_state);
            DRAW(game_state);
            send_game_update(data->cli_sockfd, game_state);
            break;
        }

        SWITCH_TURN(game_state);
    }

    close(data->cli_sockfd[0]);
    close(data->cli_sockfd[1]);

    pthread_mutex_lock(data->mutexcount);
    *data->player_count -= 2;
    pthread_mutex_unlock(data->mutexcount);

    free(data->cli_sockfd);
    free(data);
    pthread_exit(NULL);
}