#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <libgen.h>
#include "message.h"
#include "inet_socket.h"

mtx_t socket_mutex;

struct file_info_t
{
    int fd;
    char filename[UCHAR_MAX];
    unsigned long size;
};

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
        if (file_received_bytes == file_info->size)
        {
            close(file_info->fd);
        }
    }

    default:
        break;
    }
}

void handle_text_message(struct message_t *message)
{
    printf("%s\n", message->data);
}

void receive_handle_message(void *arg)
{
    int fd = *(int *)arg;

    struct message_t *message = malloc(sizeof(struct message_t));
    char *buf = malloc(sizeof(USHRT_MAX));
    struct file_info_t *file_info = malloc(sizeof(struct file_info_t));

    unsigned long file_received_bytes = 0;

    while (1)
    {
        int num_recv = recv(fd, buf, USHRT_MAX, 0);
        if (num_recv == -1)
        {
            perror("recv()");
        }
        if (num_recv == 0)
        {
            printf("Server closed.\n");
            return;
        }

        message_read(message, buf, num_recv);

        switch (message->type)
        {
        case 0:
        {
            handle_text_message(message);
            break;
        }

        case 1:
        {
            handle_file_message(message, file_info, &file_received_bytes);
            break;
        }
        }
    }
}

void send_file(int fd, char *path)
{
    struct message_t *message = malloc(sizeof(struct message_t));
    char *buf = malloc(USHRT_MAX);

    int file_fd = open(path, O_RDONLY);
    if (file_fd == -1)
    {
        perror("open()");
        sprintf(message->data, "Error receiving file!");
        message->type = 0;
        message_write(buf, message);
        return;
    }

    sprintf(message->data, "%s", basename(path));
    message->type = 1;
    message->option = 0;
    message->size = strlen(message->data);
    unsigned int num_message_write = message_write(buf, message);
    mtx_lock(&socket_mutex);
    if (send(fd, buf, num_message_write, 0) == -1)
    {
        perror("send()");
        return;
    }
    mtx_unlock(&socket_mutex);

    struct stat stat_buf;
    if (stat(path, &stat_buf) == -1)
    {
        perror("stat()");
        sprintf(message->data, "Error receiving file!");
        message->type = 0;
        message_write(buf, message);
        return;
    }

    sprintf(message->data, "%u", stat_buf.st_size);
    message->type = 1;
    message->option = 1;
    num_message_write = message_write(buf, message);
    mtx_lock(&socket_mutex);
    if (send(fd, buf, num_message_write, 0) == -1)
    {
        perror("send()");
        return;
    }
    mtx_unlock(&socket_mutex);
    
    while (1)
    {
        message->size = read(file_fd, message->data, SHRT_MAX);
        if (message->size == -1)
        {
            perror("read()");
            return;
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
            return;
        }        
        mtx_unlock(&socket_mutex);

        if (message->size < SHRT_MAX)
        {
            break;
        }
    }

    close(file_fd);
}

void interact(int fd)
{
    struct message_t *message = calloc(1, sizeof(struct message_t));
    char *buf = malloc(USHRT_MAX);
    char *tmp = malloc(SHRT_MAX);

    while (1)
    {
        printf("Enter your name: ");
        scanf("%s", message->data);
        if (strlen(message->data) > 0)
        {
            break;
        }
    }

    message->size = strlen(message->data);
    unsigned int num_message_write = message_write(buf, message);
    mtx_lock(&socket_mutex);
    if (send(fd, buf, num_message_write, 0) == -1)
    {
        perror("send()");
        return;
    }
    mtx_unlock(&socket_mutex);

    while (1)
    {
        scanf("%[^\n]s", tmp);
        getchar();
        if (strlen(tmp) == 0)
        {
            continue;
        }
        
        if (strlen(tmp) > strlen("/upload "))
        {
            if (strncmp(tmp, "/upload ", strlen("/upload ")) == 0)
            {
                char *path = malloc(SHRT_MAX);
                strcpy(path, tmp + strlen("/upload "));
                send_file(fd, path);
            }
        }
        else if (strlen(tmp) > strlen("/download ") == 0)
        {
            if (strncmp(tmp, "/download ", strlen("/download ")) == 0)
            {
                strcpy(message->data, tmp + strlen("/download "));
                message->size = strlen(message->data);
                message->type = 1;
                num_message_write = message_write(buf, message);
                mtx_lock(&socket_mutex);
                if (send(fd, buf, num_message_write, 0) == -1)
                {
                    perror("send()");
                }
                mtx_unlock(&socket_mutex);
            }
        }
    }
}

int start_client(char *ip, int port)
{
    int fd = inet_connect(ip, port, SOCK_STREAM);
    if (fd == -1)
    {
        return -1;
    }

    int *arg = malloc(sizeof(int));

    thrd_t thread;
    thrd_create(thread, receive_handle_message, arg);

    interact(fd);

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