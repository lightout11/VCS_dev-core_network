#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include "inet_socket.h"
#include "client_list.h"
#include "message.h"

unsigned long num_received_messages = 0;

mtx_t num_received_messages_mutex;

mtx_t socket_mutex;

extern mtx_t client_list_mutex;

extern struct client_info_t *client_list;

extern unsigned int max_clients;

struct file_info_t
{
    int fd;
    char filename[UCHAR_MAX];
    unsigned long size;
};

struct send_file_arg_t
{
    int fd;
    char filename[SHRT_MAX];
};

static void send_to_all(char *buf, unsigned int size, int fd)
{
    for (int i = 0; i < max_clients; i++)
    {
        if (client_list[i].fd != -1 && strlen(client_list[i].name) != 0 && client_list[i].fd != fd)
        {
            mtx_lock(&socket_mutex);
            if (send(client_list[i].fd, buf, size, 0) == -1)
            {
                perror("send()");
            }
            mtx_unlock(&socket_mutex);
        }
    }
}

int send_file(void *arg)
{
    int fd = ((struct send_file_arg_t *)arg)->fd;
    char *filename = ((struct send_file_arg_t *)arg)->filename;

    struct message_t *message = malloc(sizeof(struct message_t));
    char *buf = malloc(USHRT_MAX);

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1)
    {
        perror("open()");
        sprintf(message->data, "Error receiving file!");
        message->type = 0;
        message_write(buf, message);
        return -1;
    }

    message->type = 1;
    message->option = 0;
    message->size = strlen(message->data);
    unsigned int num_message_write = message_write(buf, message);
    mtx_lock(&socket_mutex);
    if (send(fd, buf, num_message_write, 0) == -1)
    {
        perror("send()");
        return -1;
    }
    mtx_unlock(&socket_mutex);

    struct stat stat_buf;
    if (stat(filename, &stat_buf) == -1)
    {
        perror("stat()");
        sprintf(message->data, "Error receiving file!");
        message->type = 0;
        message_write(buf, message);
        return -1;
    }

    sprintf(message->data, "%lu", stat_buf.st_size);
    message->type = 1;
    message->option = 1;
    num_message_write = message_write(buf, message);
    mtx_lock(&socket_mutex);
    if (send(fd, buf, num_message_write, 0) == -1)
    {
        perror("send()");
        return -1;
    }
    mtx_unlock(&socket_mutex);
    
    while (1)
    {
        message->size = read(file_fd, message->data, SHRT_MAX);
        if (message->size == -1)
        {
            perror("read()");
            return -1;
        }
        else if (message->size == 0)
        {
            break;
        }
        
        message->type = 1;
        message->option = 2;
        num_message_write = message_write(buf, message);
        mtx_lock(&socket_mutex);
        if (send(fd, buf, num_message_write, 0) == -1)
        {
            perror("send()");
            return -1;
        }        
        mtx_unlock(&socket_mutex);

        if (message->size < SHRT_MAX)
        {
            break;
        }
    }

    close(file_fd);
}

void handle_file_message(struct message_t *message, struct file_info_t *file_info, unsigned long *file_received_bytes)
{
    switch (message->option)
    {
    case 0:
    {
        char *path = malloc(message->size);
        strcpy(path, message->data);
        file_received_bytes = 0;
        file_info->fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (file_info->fd == -1)
        {
            perror("open()");
        }
    }
    break;

    case 1:
    {
        file_info->size = atoi(message->data);
    }
    break;

    case 2:
    {
        if (write(file_info->fd, message->data, message->size) == -1)
        {
            perror("write()");
            break;
        }
        *file_received_bytes += message->size;
        if (*file_received_bytes == file_info->size)
        {
            close(file_info->fd);
        }
    }

    default:
        break;
    }
}

static void handle_text_message(struct message_t *message, struct client_info_t *client_info, char *buf)
{
    switch (message->broadcast)
    {
    case 0:
    {
        unsigned int buf_size = message_write(buf, message);
        mtx_lock(&socket_mutex);
        if (send(client_info->fd, buf, buf_size, 0) == -1)
        {
            perror("send()");
        }
        mtx_unlock(&socket_mutex);
    }
    break;

    case 1:
    {
        char *tmp = malloc(USHRT_MAX);
        strcpy(tmp, message->data);
        sprintf(message->data, "%s: %s", client_info->name, tmp);
        unsigned int buf_size = message_write(buf, message);
        send_to_all(buf, buf_size, client_info->fd);
    }
    break;

    default:
        break;
    }
}

static int handle_connection(void *arg)
{
    struct client_info_t *client_info = (struct client_info_t *)arg;
    char *buf = malloc(USHRT_MAX);

    struct file_info_t *file_info = malloc(sizeof(struct file_info_t));
    file_info->fd = -1;
    strcpy(file_info->filename, "");
    file_info->size = 0;

    unsigned long file_received_bytes = 0;

    // Receive name
    unsigned int num_receive = recv(client_info->fd, buf, USHRT_MAX, 0);
    if (num_receive == -1)
    {
        perror("recv()");
        return -1;
    }
    if (num_receive == 0)
    {
        printf("Client %d closed.\n", client_info->fd);
        close(client_info->fd);
        client_list_remove(client_info->fd);
        free(client_info);
        free(buf);
        return -1;
    }

    struct message_t *message = malloc(sizeof(struct message_t));
    message_read(message, buf, num_receive);
    client_list_update_name(client_info->fd, message->data);

    while (1)
    {
        num_receive = recv(client_info->fd, buf, USHRT_MAX, 0);
        if (num_receive == -1)
        {
            perror("recv");
            return -1;
        }
        if (num_receive == 0)
        {
            printf("Client %d closed.\n", client_info->fd);
            client_list_remove(client_info->fd);
            free(client_info);
            free(buf);
            return -1;
        }

        mtx_lock(&num_received_messages_mutex);
        ++num_received_messages;
        mtx_unlock(&num_received_messages_mutex);

        switch (message->type)
        {
        case 0:
        {
            handle_text_message(message, client_info, buf);
        }
        break;

        case 1:
        {
            handle_file_message(message, file_info, &file_received_bytes);
            if (file_info->size == file_received_bytes)
            {
                sprintf(message->data, "%s has uploaded %s", client_info->name, file_info->filename);
                message->type = 0;
                message->size = strlen(message->data);
                unsigned int buf_size = message_write(buf, message);
                send_to_all(buf, buf_size, -1);
            }
        }

        case 2:
        {
            struct send_file_arg_t *arg = malloc(sizeof(struct send_file_arg_t));
            arg->fd = client_info->fd;
            strcpy(arg->filename, message->data);
            thrd_t thread;
            thrd_create(&thread, send_file, arg);
        }

        default:
            break;
        }
    }

    return 0;
}

int start_server(int port)
{
    int lfd = inet_listen(port, max_clients);
    if (lfd == -1)
    {
        return -1;
    }

    while (1)
    {
        int fd = accept(lfd, NULL, NULL);
        if (fd == -1)
        {
            perror("accept()");
            return fd;
        }

        struct client_info_t *client_info = malloc(sizeof(struct client_info_t));
        client_info->fd = fd;

        thrd_t thread;
        thrd_create(&thread, handle_connection, (void *)client_info);
    }
}

void mutex_init(int num_clients)
{
    mtx_init(&client_list_mutex, mtx_plain);
    mtx_init(&num_received_messages_mutex, mtx_plain);
    mtx_init(&socket_mutex, mtx_plain);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
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

    max_clients = atoi(argv[2]);
    if (max_clients == 0)
    {
        printf("Invalid max number of clients!\n");
        return 1;
    }

    client_list_init();

    return start_server(port);
}