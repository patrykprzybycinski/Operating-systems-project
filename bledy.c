#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void blad(const char *msg)
{
    perror(msg);
    exit(1);
}
