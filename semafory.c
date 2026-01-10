#include "shared.h"

int semafor;

void utworz_nowy_semafor()
{
    semafor = semget(130, 1, 0777 | IPC_CREAT);

    if (semafor == -1) 
    {
        perror("semget");
        exit(1);
    }
}

void ustaw_semafor()
{
    if (semctl(semafor, 0, SETVAL, 1) == -1) 
    {
        perror("semctl");
        exit(1);
    }
}

void semafor_p()
{
    struct sembuf b = {0, -1, 0};

    if (semop(semafor, &b, 1) == -1) 
    {
        if (errno == EINTR) semafor_p();
        
        else 
        { 
            perror("semop P"); exit(1); 
        }
    }
}

void semafor_v()
{
    struct sembuf b = {0, 1, 0};
    
    if (semop(semafor, &b, 1) == -1) 
    {
        perror("semop V");
        exit(1);
    }
}

void usun_semafor()
{
    semctl(semafor, 0, IPC_RMID);
}
