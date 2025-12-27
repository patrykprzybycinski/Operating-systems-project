#include "shared.h"

static void poczatek(void)
{
    printf("Semafory.\n");
}

static void utworz_nowy_semafor(void)
{
    semafor=semget(130,1,0777|IPC_CREAT);
    
    if (semafor==-1)
    {
        printf("Nie moglem utworzyc nowego semafora.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Semafor zostal utworzony : %d\n",semafor);
    }
}

static void ustaw_semafor(void)
{
    int ustaw_sem;
    ustaw_sem=semctl(semafor,0,SETVAL,0);

    if (ustaw_sem==-1)
    {
        printf("Nie mozna ustawic semafora.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Semafor zostal ustawiony.\n");
    }
}


static void semafor_p(void)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=0;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semafor,&bufor_sem,1);

    if (zmien_sem==-1) 
    {
        if(errno == EINTR)
        {
            semafor_p();
        }
        else
        {
            printf("Nie moglem zamknac semafora.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("Semafor zostal zamkniety.\n");
    }
}

static void semafor_v(void)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=0;
    bufor_sem.sem_op=1;
    bufor_sem.sem_flg=SEM_UNDO;
    zmien_sem=semop(semafor,&bufor_sem,1);

    if (zmien_sem==-1) 
    {
        printf("Nie moglem otworzyc semafora.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Semafor zostal otwarty.\n");
    }
}

static void usun_semafor(void)  
{
    int sem;
    sem=semctl(semafor,0,IPC_RMID);

    if (sem==-1)
    {
        printf("Nie mozna usunac semafora.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Semafor zostal usuniety : %d\n",sem);
    }
}