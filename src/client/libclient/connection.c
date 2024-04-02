#include <client/connection.h>
#include <client/error_handler.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

void get_servers_info(const char* filename, server_info* servers_info, uint64_t servers_num)
{
    FILE* servers_file = fopen(filename, "r");

    if (servers_file == NULL)
    {
        error("Unable to open file with servers info");
    }

    char cur_ip_str[INET_ADDRSTRLEN];
    int cur_port = 0;

    for (uint64_t cur_server = 0;
         (fscanf(servers_file, "%s %d", cur_ip_str, &cur_port) != EOF) && (cur_server < servers_num);
         cur_server++)
    {
        inet_pton(AF_INET, cur_ip_str, &servers_info[cur_server].ip);
        servers_info[cur_server].port = cur_port;
    }
}

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

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
    {
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int connect_to_primary_server(server_info* servers_info, uint64_t servers_num, uint64_t* cur_server, bool is_recovery)
{
    uint8_t rec_port_shift;

    if (is_recovery)
    {
        rec_port_shift = 100;
    }
    else
    {
        rec_port_shift = 0;
    }

    int sockfd = -1;

    while ((sockfd = connect_to_server(servers_info[*cur_server].ip, servers_info[*cur_server].port + rec_port_shift))
           == -1)
    {
        (*cur_server)++;

        if (*cur_server == servers_num)
        {
            error("All servers unavailable\n");
        }
    }

    return sockfd;
}