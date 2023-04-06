#include <server/connection.h>
#include <server/error_handler.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>

int setup_listener(int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("Can't open listener socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Can't bind listener socket");
    }

    return sockfd;
}

void get_clients(int lis_sockfd, int* cli_sockfd)
{
    socklen_t clilen;
    struct sockaddr_in cli_addr;

    int num_conn = 0;

    while (num_conn < 2)
    {
        listen(lis_sockfd, MAX_PLAYERS - player_count);

        memset(&cli_addr, 0, sizeof(cli_addr));

        clilen = sizeof(cli_addr);

        cli_sockfd[num_conn] = accept(lis_sockfd, (struct sockaddr*)&cli_addr, &clilen);

        if (cli_sockfd[num_conn] < 0)
        {
            error("Can't accept connection from a client");
        }

        if (send(cli_sockfd[num_conn], &num_conn, sizeof(int), 0) == -1)
        {
            error("Can't send to client");
        }

        pthread_mutex_lock(&mutexcount);
        player_count++;
        pthread_mutex_unlock(&mutexcount);

        num_conn++;
    }
}

void* debug_func(void* thread_data)
{
    int* cli_sockfd = (int*)thread_data;

    char msg[6] = "CHECK";
    char exit_msg[6] = "EXIT0";
    int recv_int0 = 0;
    int recv_int1 = 0;

    if (send(cli_sockfd[0], msg, sizeof(char) * strlen(msg), 0) == -1)
    {
        error("Can't send to client 0");
    }
    while (recv_int0 != 1)
    {
        if (recv(cli_sockfd[0], &recv_int0, sizeof(int), 0) == -1)
        {
            error("Can't recv from client 0");
        }
    }
    if (send(cli_sockfd[1], msg, sizeof(char) * strlen(msg), 0) == -1)
    {
        error("Can't send to client 1");
    }
    while (recv_int1 != 1)
    {
        if (recv(cli_sockfd[1], &recv_int1, sizeof(int), 0) == -1)
        {
            error("Can't recv from client 1");
        }
    }
    if (send(cli_sockfd[0], exit_msg, sizeof(char) * strlen(msg), 0) == -1)
    {
        error("Can't send to client 0");
    }
    if (send(cli_sockfd[1], exit_msg, sizeof(char) * strlen(msg), 0) == -1)
    {
        error("Can't send to client 1");
    }

    close(cli_sockfd[0]);
    close(cli_sockfd[1]);

    free(cli_sockfd);
    pthread_exit(NULL);
}