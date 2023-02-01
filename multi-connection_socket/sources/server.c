#include <stdio.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include "inet_socket.h"

#define BACKLOG 1000

void *handle_connection(void *data)
{
    int cfd = *(int *)data;
    printf("Handle socket %d:\n", cfd);
    while (1)
    {
        char buf[BUFSIZE];
        int numRecv = recv(cfd, buf, BUFSIZ, 0);
        if (numRecv == -1)
        {
            perror("recv()");
            return (void *)-1;
        }
        if (numRecv == 0)
        {
            break;
        }
        
        printf("Server: %s", buf);
        
        if (send(cfd, buf, numRecv, 0) != numRecv)
        {
            perror("send()");
            return (void *)-1;
        }
    }

    return 0;
}

int start_server(int port)
{
    int sfd = inet_listen(port, BACKLOG);

    if (sfd == -1)
    {
        perror("Error: inet_listen()\n");
        return -1;
    }

    while (1)
    {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd == -1)
        {
            perror("accept()");
            return -1;
        }

        pthread_t thread;
        pthread_create(&thread, NULL, handle_connection, (void *)&cfd);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Invalid command!\n");
        return 1;
    }

    int port = atoi(argv[1]);
    if (port == 0)
    {
        printf("Invalid port number!\n");
        return 1;
    }

    return start_server(port);
}