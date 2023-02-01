#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include "inet_socket.h"

int communicate(int cfd)
{
    while (1)
    {
        printf("> ");
        char buf[BUFSIZE];
        scanf("%s", buf);
        if (send(cfd, buf, strlen(buf) + 1, 0) == -1)
        {
            perror("send()");
            return -1;
        }
        int numRecv = recv(cfd, buf, BUFSIZE, 0);
        if (numRecv == -1)
        {
            perror("recv()");
            return -1;
        }
        printf("%s\n", buf);
    }
}

int start_client(char *ip, int port)
{
    int cfd = inet_connect(ip, port, SOCK_STREAM);
    if (cfd == -1)
    {
        perror("Error: inet_connect()\n");
        return -1;
    }

    communicate(cfd);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Invalid command!\n");
        return 1;
    }

    int port = atoi(argv[2]);
    if (port == 0)
    {
        printf("Invalid port number!\n");
        return 1;
    }

    return start_client(argv[1], port);
}
