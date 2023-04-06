#include <server/error_handler.h>
#include <stdio.h>
#include <pthread.h>

void error(const char* msg)
{
    perror(msg);
    pthread_exit(NULL);
}