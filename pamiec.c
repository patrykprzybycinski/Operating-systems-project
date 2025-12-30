#include "shared.h"

int pamiec;
char *adres;

void upd()
{
    pamiec = shmget(11, 256, 0777 | IPC_CREAT);

    if (pamiec == -1) 
    {
        perror("shmget");
        exit(1);
    }
}

void upa()
{
    adres = (char *)shmat(pamiec, NULL, 0);
    
    if (adres == (char *)(-1)) 
    {
        perror("shmat");
        exit(1);
    }
}

void odlacz_pamiec()
{
    shmdt(adres);
    shmctl(pamiec, IPC_RMID, 0);
}
