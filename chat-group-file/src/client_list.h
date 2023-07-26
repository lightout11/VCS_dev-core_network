#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

#include <threads.h>

struct client_info_t
{
    int fd;
    char name[256];
};

void client_list_init();

struct client_info_t *client_list_add(int fd);

void client_list_remove(int fd);

void client_list_update_name(int fd, char *name);

#endif