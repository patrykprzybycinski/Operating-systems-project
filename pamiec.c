#include "shared.h"

int pamiec;
char *adres;

void upd()
{
    key_t key_shm = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT/ipc.key", 'M');
    if (key_shm == -1)
    {
        blad("ftok shm");
    }

    pamiec = shmget(key_shm, 256, 0777 | IPC_CREAT);
    if (pamiec == -1)
    {
        blad("shmget");
    }
}


void upa()
{
    adres = (char *)shmat(pamiec, NULL, 0);
    if (adres == (char *)(-1)) 
    {
        blad("shmat");
    }
}


void odlacz_pamiec()
{
    if (shmdt(adres) == -1)
    {
        perror("shmdt");
    }

    if (shmctl(pamiec, IPC_RMID, 0) == -1)
    {
        perror("shmctl IPC_RMID");
    }
}

void podlacz_pamiec()
{
    key_t key_shm = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT/ipc.key", 'M');
    if (key_shm == -1)
    {
        perror("ftok shm");
        exit(1);
    }

    pamiec = shmget(key_shm, 256, 0);
    if (pamiec == -1)
    {
        blad("shmget attach");
    }


    upa();
}
