#include "shared.h"
#include <string.h>

static FILE *log_file = NULL;

void log_init(const char *filename)
{
    log_file = fopen(filename, "a");
    if (!log_file)
    {
        perror("fopen log");
        exit(1);
    }
}

void log_msg(const char *msg)
{
    if (!log_file) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_file, "[%s] %s", timebuf, msg);
    fflush(log_file);
}

void log_close()
{
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;
    }
}
