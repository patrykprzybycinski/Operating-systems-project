#include "shared.h"

/* Identyfikator semafora System V */
int semafor;

/* Utworzenie semafora binarnego */
void utworz_nowy_semafor()
{
    key_t key_sem = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'S');
    if (key_sem == -1)
    {
        blad("ftok sem");
    }

    semafor = semget(key_sem, 1, 0600 | IPC_CREAT);
    if (semafor == -1)
    {
        blad("semget");
    }
}

/* Ustawienie wartości początkowej semafora na 1 */
void ustaw_semafor()
{
    if (semctl(semafor, 0, SETVAL, 1) == -1) 
    {
        blad("semctl SETVAL");
    }
}

/* Operacja P – wejście do sekcji krytycznej */
void semafor_p()
{
    struct sembuf b = {0, -1, 0};
    if (semop(semafor, &b, 1) == -1) 
    {
        /* Ponowienie operacji po przerwaniu sygnałem */
        if (errno == EINTR) 
        {
            semafor_p();
        }
        else 
        { 
            blad("semop P"); 
        }
    }
}

/* Operacja V – wyjście z sekcji krytycznej */
void semafor_v()
{
    struct sembuf b = {0, 1, 0};
    if (semop(semafor, &b, 1) == -1) 
    {
        /* Ponowienie operacji po przerwaniu sygnałem */
        if (errno == EINTR) 
        {
            semafor_v();
        }
        else 
        { 
            blad("semop V"); 
        }
    }
}

/* Usunięcie semafora z systemu */
void usun_semafor()
{
    semctl(semafor, 0, IPC_RMID);
}
