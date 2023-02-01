#ifndef INET_SOCKET_H
#define INET_SOCKET_H

#define BUFSIZE 512

int inet_connect(char *ip, int port, int type);

int inet_bind(int port, int type);

int inet_listen(int port, int backlog);

#endif