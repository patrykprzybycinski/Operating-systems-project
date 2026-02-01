#include "shared.h"

/* ID semafora System V nadane przez kernel */
int semafor;

/* Tworzy (lub pobiera) semafor binarny */
void utworz_nowy_semafor()
{
    /* Generuje wspólny klucz IPC dla semafora */
    key_t key_sem = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'S');

    /* Sprawdzenie błędu generowania klucza */
    if (key_sem == -1)
        blad("ftok sem");

    /* Tworzy semafor z jednym licznikiem */
    semafor = semget(key_sem, 1, 0600 | IPC_CREAT);

    /* Sprawdzenie błędu utworzenia semafora */
    if (semafor == -1)
        blad("semget");
}

/* Ustawia semafor w stan początkowy „wolny” */
void ustaw_semafor()
{
    /* Ustawia wartość semafora na 1 */
    if (semctl(semafor, 0, SETVAL, 1) == -1)
        blad("semctl SETVAL");
}

/* Operacja P – blokuje dostęp do sekcji krytycznej */
void semafor_p()
{
    /* Dekrementacja semafora (czekanie, aż będzie wolny) */
    struct sembuf b = {0, -1, 0};

    /* Wykonanie operacji P */
    if (semop(semafor, &b, 1) == -1)
    {
        /* Jeśli przerwane sygnałem – próbuj ponownie */
        if (errno == EINTR)
            semafor_p();
        else
            blad("semop P");
    }
}

/* Operacja V – zwalnia sekcję krytyczną */
void semafor_v()
{
    /* Inkrementacja semafora (zwolnienie dostępu) */
    struct sembuf b = {0, 1, 0};

    /* Wykonanie operacji V */
    if (semop(semafor, &b, 1) == -1)
    {
        /* Jeśli przerwane sygnałem – próbuj ponownie */
        if (errno == EINTR)
            semafor_v();
        else
            blad("semop V");
    }
}

/* Usuwa semafor z systemu */
void usun_semafor()
{
    /* Trwale usuwa semafor z kernela */
    semctl(semafor, 0, IPC_RMID);
}
