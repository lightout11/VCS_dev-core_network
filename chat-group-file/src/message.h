#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

struct message_t
{
    unsigned int type;
    unsigned int option;
    unsigned int size;
    unsigned int broadcast;
    char data[SHRT_MAX];
};

unsigned int message_write(char *buf, struct message_t *message);

void message_read(struct message_t *message, char *buf, unsigned int size);

#endif