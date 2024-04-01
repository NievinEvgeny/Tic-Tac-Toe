#include <client/connection.h>
#include <client/game.h>
#include <client/error_handler.h>
#include <stdbool.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#define PLAYER_ID(game_state) (game_state & 0x8000) >> 15

#define CHECK_DRAW(game_state) (game_state & 0x20000000) >> 29

#define CHECK_WIN(game_state) (game_state & 0x40000000) >> 30

static bool game_on(int32_t game_state)
{
    return game_state & 0x80000000;
}

static void print_board(int32_t game_state)
{
    printf("\e[H\e[2J\e[3J");
    for (short j = 0; j < 3; j++)
    {
        for (short i = 1; i != 8; i <<= 1)
        {
            if (game_state & (i << j * 3))
            {
                printf("X ");
                continue;
            }
            if (game_state & (i << (j * 3 + 16)))
            {
                printf("O ");
                continue;
            }
            printf("- ");
        }
        printf("\n");
    }
}

static int8_t recv_int8(int sockfd)
{
    int8_t data = 0;
    int msg_len = recv(sockfd, &data, sizeof(data), 0);

    return msg_len > 0 ? data : -1;
}

static bool get_game_update(int sockfd, int32_t* game_state)
{
    int msg_len = recv(sockfd, game_state, sizeof(*game_state), 0);

    return msg_len > 0;
}

static short get_move()
{
    while (true)
    {
        short move = -1;

        int result = scanf("%hd", &move);

        if (result == EOF)
        {
            error("Can't read from stdin");
        }
        if (result == 0)
        {
            while (fgetc(stdin) != '\n')
            {
            }
        }
        if (move >= 0 && move <= 8)
        {
            return move;
        }

        printf("Invalid input, try again\n");
    }
}

static void send_move(int sockfd, short move)
{
    send(sockfd, &move, sizeof(move), 0);
}

static bool process_player_move(int sockfd)
{
    while (true)
    {
        send_move(sockfd, get_move());

        int8_t move_valid = recv_int8(sockfd);

        if (move_valid == 0)
        {
            printf("This field is already occupied, change your move\n");
            continue;
        }

        return move_valid != -1;
    }
}

void play_game(server_info* servers_info, uint64_t servers_num)
{
    uint64_t cur_server = 0;

    int sockfd = connect_to_primary_server(servers_info, servers_num, &cur_server);

    int32_t game_state = 0;

    const int8_t player_id = recv_int8(sockfd);
    const int8_t game_id = recv_int8(sockfd);

    if ((player_id == -1) || (game_id == -1))
    {
        sockfd = connect_to_primary_server(servers_info, servers_num, &cur_server);
    }

    while (!game_on(game_state))
    {
        if (!get_game_update(sockfd, &game_state))
        {
            sockfd = connect_to_primary_server(servers_info, servers_num, &cur_server);
            send(sockfd, &game_id, sizeof(game_id), 0);
        }

        print_board(game_state);
    }

    while (game_on(game_state))
    {
        if (PLAYER_ID(game_state) == player_id)
        {
            printf("Your turn\n");

            if (!process_player_move(sockfd))
            {
                sockfd = connect_to_primary_server(servers_info, servers_num, &cur_server);
                send(sockfd, &game_id, sizeof(game_id), 0);
            }
        }

        if (!get_game_update(sockfd, &game_state))
        {
            sockfd = connect_to_primary_server(servers_info, servers_num, &cur_server);
            send(sockfd, &game_id, sizeof(game_id), 0);
        }

        print_board(game_state);
    }

    if (CHECK_DRAW(game_state))
    {
        printf("Draw\n");
        return;
    }

    if (CHECK_WIN(game_state) == player_id)
    {
        printf("You won\n");
        return;
    }

    printf("You lost\n");
}