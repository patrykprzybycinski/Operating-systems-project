#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/*
 * Funkcja pomocnicza do obsługi błędów systemowych.
 *
 * msg:
 *  - tekstowy opis kontekstu błędu (np. nazwa funkcji systemowej),
 *    który zostanie przekazany do perror().
 *
 * perror():
 *  - wypisuje msg oraz opis błędu na podstawie zmiennej errno.
 *
 * exit(1):
 *  - natychmiastowe zakończenie programu z kodem błędu,
 *    aby nie kontynuować działania w niepoprawnym stanie.
 */
void blad(const char *msg)
{
    perror(msg);
    exit(1);
}
