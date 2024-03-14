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

static bool recv_player_id(int sockfd)
{
    bool id = false;
    int msg_len = recv(sockfd, &id, sizeof(id), 0);

    if (msg_len < 0)
    {
        error("Can't get id from server");
    }

    if (msg_len == 0)
    {
        error("Server is shutdown");
    }

    return id;
}

static uint8_t recv_game_id(int sockfd)
{
    uint8_t id = 0;
    int msg_len = recv(sockfd, &id, sizeof(id), 0);

    if (msg_len < 0)
    {
        error("Can't get id from server");
    }

    if (msg_len == 0)
    {
        error("Server is shutdown");
    }

    return id;
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

static void get_game_update(int sockfd, int32_t* game_state)
{
    int msg_len = recv(sockfd, game_state, sizeof(*game_state), 0);

    if (msg_len < 0)
    {
        error("Can't get update of game from server");
    }

    if (msg_len == 0)
    {
        error("Server is shutdown");
    }
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
    if (send(sockfd, &move, sizeof(move), 0) == -1)
    {
        error("Can't send move to server");
    }
}

static int8_t recv_move_validity(int sockfd)
{
    int8_t move_validity = 0;
    int msg_len = recv(sockfd, &move_validity, sizeof(move_validity), 0);

    if (msg_len < 0)
    {
        error("Can't get move validity from server");
    }

    if (msg_len == 0)
    {
        error("Server is shutdown");
    }

    return move_validity;
}

static void process_player_move(int sockfd)
{
    while (true)
    {
        send_move(sockfd, get_move());

        int8_t move_valid = recv_move_validity(sockfd);

        if (move_valid == 1)
        {
            return;
        }

        if (move_valid == 0)
        {
            printf("This field is already occupied, change your move\n");
            continue;
        }

        if (move_valid == -1)
        {
            printf("You lost on time\n");
            return;
        }
    }
}

void play_game(int sockfd)
{
    int32_t game_state = 0;

    const bool player_id = recv_player_id(sockfd);
    const uint8_t game_id = recv_game_id(sockfd);

    while (!game_on(game_state))
    {
        get_game_update(sockfd, &game_state);
        print_board(game_state);
    }

    while (game_on(game_state))
    {
        if (PLAYER_ID(game_state) == player_id)
        {
            printf("Your turn\n");
            process_player_move(sockfd);
        }

        get_game_update(sockfd, &game_state);
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