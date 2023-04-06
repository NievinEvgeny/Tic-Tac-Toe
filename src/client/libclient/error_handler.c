#include <client/error_handler.h>
#include <stdio.h>
#include <stdlib.h>

void error(const char* msg)
{
    perror(msg);
    printf("Disconnect.\nGame over.\n");
    exit(0);
}