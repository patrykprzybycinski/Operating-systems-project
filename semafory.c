#include "shared.h"

int semafor;

void utworz_nowy_semafor()
{
    key_t key_sem = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT/ipc.key", 'S');
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

void ustaw_semafor()
{
    if (semctl(semafor, 0, SETVAL, 1) == -1) 
    {
        blad("semctl SETVAL");
    }
}

void semafor_p()
{
    struct sembuf b = {0, -1, 0};
    if (semop(semafor, &b, 1) == -1) 
    {
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

void semafor_v()
{
    struct sembuf b = {0, 1, 0};
    if (semop(semafor, &b, 1) == -1) 
    {
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


void usun_semafor()
{
    semctl(semafor, 0, IPC_RMID);
}
