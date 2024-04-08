#include <server/connection.h>
#include <server/error_handler.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

int connect_to_server(uint32_t ip, int port)
{
    struct sockaddr_in serv_addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("Can't open socket for server");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ip;
    serv_addr.sin_port = htons(port);

    return connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == 0 ? sockfd : -1;
}

int setup_listener(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("Can't open listener socket");
    }

    struct sockaddr_in serv_addr;

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

void get_clients(int lis_sockfd, int* cli_sockfd, int* player_count, pthread_mutex_t* mutexcount)
{
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    int num_conn = 0;

    while (num_conn < 2)
    {
        listen(lis_sockfd, MAX_PLAYERS - *player_count);

        memset(&cli_addr, 0, sizeof(cli_addr));

        cli_sockfd[num_conn] = accept(lis_sockfd, (struct sockaddr*)&cli_addr, &clilen);

        if (cli_sockfd[num_conn] < 0)
        {
            error("Can't accept connection from a client");
        }

        pthread_mutex_lock(mutexcount);
        (*player_count)++;
        pthread_mutex_unlock(mutexcount);

        num_conn++;
    }
}