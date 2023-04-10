#include <client/connection.h>
#include <client/error_handler.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

int connect_to_server(char* hostname, int port)
{
    struct sockaddr_in serv_addr;
    struct hostent* server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("Can't open socket for server");
    }

    server = gethostbyname(hostname);

    if (server == NULL)
    {
        fprintf(stderr, "Can't find host with name: %s\n", hostname);
        exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    memmove(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Can't connect to server");
    }

    return sockfd;
}