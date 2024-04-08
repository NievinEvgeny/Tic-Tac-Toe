#include <server/synchronization.h>
#include <server/error_handler.h>
#include <server/connection.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/select.h>
#include <pthread.h>

static uint32_t get_eth0_ip()
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "eth0");

    int s = socket(AF_INET, SOCK_DGRAM, 0);

    if (s == -1)
    {
        error("Can't create socket");
    }

    if (ioctl(s, SIOCGIFADDR, &ifr) == -1)
    {
        close(s);
        error("Ioctl error");
    }

    close(s);

    struct sockaddr_in* sa = (struct sockaddr_in*)&ifr.ifr_addr;

    if (sa == NULL)
    {
        error("Failed to retrieve address");
    }

    return sa->sin_addr.s_addr;
}

static uint64_t get_servers_info(const char* filename, int port, uint64_t servers_num, server_info* servers_info)
{
    const uint32_t ip = get_eth0_ip();

    FILE* servers_file = fopen(filename, "r");

    if (servers_file == NULL)
    {
        error("Unable to open file with servers info");
    }

    char cur_ip_str[INET_ADDRSTRLEN];
    uint32_t cur_ip = 0;
    int cur_port = 0;

    for (uint64_t priority = 0;
         (fscanf(servers_file, "%s %d", cur_ip_str, &cur_port) != EOF) && (priority < servers_num);
         priority++)
    {
        inet_pton(AF_INET, cur_ip_str, &cur_ip);

        if (cur_ip == ip && cur_port == port)
        {
            fclose(servers_file);
            return priority;
        }

        servers_info[priority].ip = cur_ip;
        servers_info[priority].port = cur_port;
    }

    fclose(servers_file);
    return UINT64_MAX;
}

static void recv_games_updates(synchronization_data* data, uint8_t lis_port_shift)
{
    server_info servers_info[data->servers_num];

    uint64_t serv_priority = get_servers_info(data->servers_filename, data->port, data->servers_num, servers_info);

    if (serv_priority == UINT64_MAX)
    {
        error("Server isn't registered in database");
    }

    if (serv_priority != 0)
    {
        *data->was_slave = true;

        uint64_t cur_priority = 0;

        int cur_serv_sock_fd
            = connect_to_server(servers_info[cur_priority].ip, servers_info[cur_priority].port + lis_port_shift);

        while (true)
        {
            if (recv(cur_serv_sock_fd, data->games, sizeof(int32_t) * MAX_PLAYERS / 2, 0) <= 0)
            {
                close(cur_serv_sock_fd);

                cur_priority++;

                if (cur_priority == serv_priority)
                {
                    break;
                }

                cur_serv_sock_fd = connect_to_server(
                    servers_info[cur_priority].ip, servers_info[cur_priority].port + lis_port_shift);
            }
        }
    }
}

static void send_games_updates(int lis_sockfd, synchronization_data* data)
{
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    struct sockaddr_in serv_addr;
    socklen_t servlen = sizeof(serv_addr);

    conn_server_info connected_servers[data->servers_num];

    while (true)
    {
        for (uint64_t i = 0; i < data->servers_num; i++)
        {
            if (!connected_servers[i].is_connected)
            {
                continue;
            }

            if (send(connected_servers[i].sock_fd, data->games, sizeof(int32_t) * MAX_PLAYERS / 2, 0) == -1)
            {
                connected_servers[i].is_connected = false;
            }
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(lis_sockfd, &readfds);

        if (select(lis_sockfd + 1, &readfds, NULL, NULL, &timeout) <= 0 || !FD_ISSET(lis_sockfd, &readfds))
        {
            continue;
        }

        memset(&serv_addr, 0, servlen);

        int conn_sockfd = accept(lis_sockfd, (struct sockaddr*)&serv_addr, &servlen);

        if (conn_sockfd > 0)
        {
            for (uint64_t i = 0; i < data->servers_num; i++)
            {
                if (!connected_servers[i].is_connected)
                {
                    connected_servers[i].sock_fd = conn_sockfd;
                    connected_servers[i].is_connected = true;
                    break;
                }
            }
        }
    }
}

void* synchronization(void* thread_data)
{
    const uint8_t lis_port_shift = 50;

    synchronization_data* data = (synchronization_data*)thread_data;

    const int listen_port = data->port + lis_port_shift;
    int lis_sockfd = setup_listener(listen_port);
    listen(lis_sockfd, data->servers_num);

    recv_games_updates(data, lis_port_shift);  // until this server isn't primary

    *data->is_primary = true;

    send_games_updates(lis_sockfd, data);
}