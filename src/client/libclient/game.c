#include <client/game.h>
#include <client/error_handler.h>
#include <stdbool.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#define PLAYER_ID(game_state) (game_state & 0x8000) >> 15

static bool game_on(int32_t game_state)
{
    return game_state & 0x80000000;
}

static void recv_id(int sockfd, bool* id)
{
    if (recv(sockfd, id, sizeof(bool), 0) == -1)
    {
        error("Can't recv id from server");
    }
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
    if (recv(sockfd, game_state, sizeof(int32_t), 0) == -1)
    {
        error("Can't get update of game from server");
    }
}

static short get_move()
{
    while (true)
    {
        short move = 0;

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

static void send_move(int sockfd, short* move)
{
    if (send(sockfd, move, sizeof(short), 0) == -1)
    {
        error("Can't send move to server");
    }
}

static void recv_move_validity(int sockfd, bool* move_validity)
{
    if (recv(sockfd, move_validity, sizeof(bool), 0) == -1)
    {
        error("Can't get move validity from server");
    }
}

static void process_player_move(int sockfd)
{
    while (true)
    {
        short move = get_move();
        send_move(sockfd, &move);

        bool move_valid = 0;
        recv_move_validity(sockfd, &move_valid);
        if (move_valid)
        {
            break;
        }

        printf("This field is already occupied, change your move\n");
    }
}

void play_game(int sockfd)
{
    int32_t game_state = 0;
    bool id;

    recv_id(sockfd, &id);

    while (!game_on(game_state))
    {
        get_game_update(sockfd, &game_state);
    }

    while (game_on(game_state))
    {
        print_board(game_state);

        if (PLAYER_ID(game_state) == id)
        {
            printf("Your turn\n");
            process_player_move(sockfd);
        }

        get_game_update(sockfd, &game_state);
    }
}