#include <stdio.h>
#include <stdlib.h>
#include "message.h"

unsigned int message_write(char *buf, struct message_t *message)
{
    sprintf(buf, "%u\r\n%u\r\n%u\r\n%u\r\n", message->type, message->option, message->size, message->broadcast);
    unsigned int header = strlen(buf);
    memcpy(buf + header, message->data, message->size);
    return header + message->size;
}

void message_read(struct message_t *message, char *buf, unsigned int size)
{
    sscanf(buf, "%[^\r\n]u%[^\r\n]u%[^\r\n]u%[^\r\n]u", &message->type, &message->option, &message->size, &message->broadcast);
    unsigned int header = size - message->size;
    memcpy(message->data, buf + header, message->size);
}
