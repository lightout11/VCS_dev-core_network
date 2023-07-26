#include "client_list.h"
#include <threads.h>
#include <stdlib.h>
#include <string.h>

struct client_info_t *client_list;

unsigned int max_clients;

mtx_t client_list_mutex;

void client_list_init()
{
    mtx_init(&client_list_mutex, mtx_plain);
    client_list = calloc(max_clients, sizeof(struct client_info_t));
    for (int i = 0; i < max_clients; i++)
    {
        client_list[i].fd = -1;
    }
}

struct client_info_t *client_list_add(int fd)
{
    mtx_lock(&client_list_mutex);
    for (int i = 0; i < max_clients; i++)
    {
        if (client_list[i].fd == -1)
        {
            client_list[i].fd = fd;
            mtx_unlock(&client_list_mutex);
            return &client_list[i];
        }
    }
    mtx_unlock(&client_list_mutex);
    return NULL;
}

void client_list_remove(int fd)
{
    mtx_lock(&client_list_mutex);
    for (int i = 0; i < max_clients; i++)
    {
        if (client_list[i].fd == fd)
        {
            client_list[i].fd = -1;
            break;
        }
    }
    mtx_unlock(&client_list_mutex);
}

void client_list_update_name(int fd, char *name)
{
    mtx_lock(&client_list_mutex);
    for (int i = 0; i < max_clients; i++)
    {
        if (client_list[i].fd == fd)
        {
            strcpy(client_list[i].fd, name);
            break;
        }
    }
    mtx_unlock(&client_list_mutex);
}