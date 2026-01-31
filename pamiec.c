#include "shared.h"

/* Identyfikator segmentu pamięci dzielonej */
int pamiec;

/* Adres przyłączonej pamięci dzielonej */
char *adres;

/* Utworzenie (lub pobranie) segmentu pamięci dzielonej */
void upd()
{
    key_t key_shm = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'M');
    if (key_shm == -1)
    {
        blad("ftok shm");
    }

    pamiec = shmget(key_shm, 256, 0600 | IPC_CREAT);
    if (pamiec == -1)
    {
        blad("shmget");
    }
}

/* Przyłączenie pamięci dzielonej do przestrzeni adresowej procesu */
void upa()
{
    adres = (char *)shmat(pamiec, NULL, 0);
    if (adres == (char *)(-1)) 
    {
        blad("shmat");
    }
}

/* Odłączenie i usunięcie pamięci dzielonej */
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

/* Podłączenie do już istniejącej pamięci dzielonej */
void podlacz_pamiec()
{
    key_t key_shm = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'M');
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
