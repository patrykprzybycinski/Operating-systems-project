#include "shared.h"
#include <string.h>

/* Wskaźnik na aktualnie otwarty plik logu */
static FILE *log_file = NULL;

/* Inicjalizacja logowania – otwarcie pliku w trybie dopisywania */
void log_init(const char *filename)
{
    log_file = fopen(filename, "a");
    if (!log_file)
    {
        perror("fopen log");
        exit(1);
    }
}

/* Zapis pojedynczej wiadomości do logu z aktualnym timestampem */
void log_msg(const char *msg)
{
    /* Jeśli log nie został zainicjalizowany – nic nie rób */
    if (!log_file) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char timebuf[64];
    /* Format czasu: YYYY-MM-DD HH:MM:SS */
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_file, "[%s] %s", timebuf, msg);
    fflush(log_file);
}

/* Zamknięcie pliku logu i zwolnienie zasobu */
void log_close()
{
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;
    }
}
