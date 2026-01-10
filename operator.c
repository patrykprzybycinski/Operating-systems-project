#include "shared.h"
#include <sys/wait.h>

struct stan *s;

pid_t drony[100];
int liczba_dronow = 0;

void sprzatnij_drony()
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < liczba_dronow; i++)
        {
            if (drony[i] == pid)
            {
                drony[i] = drony[liczba_dronow - 1];
                liczba_dronow--;
                printf("[OPERATOR] DRON PID=%d USUNIETY Z LISTY\n", pid);
                break;
            }
        }
    }
}

void stworz_drona()
{
    pid_t pid = fork();
    
    if (pid == 0)
    {
        execl("./dron", "dron", NULL);
        exit(1);
    }

    drony[liczba_dronow++] = pid;

    semafor_p();
    s->aktywne_drony++;
    semafor_v();

    printf("[OPERATOR] +++ NOWY DRON PID=%d\n", pid);
}

void sig_plus(int sig)
{
    semafor_p();
    s->max_drony = 2 * N;
    semafor_v();

    printf("[OPERATOR] !!! DODANO PLATFORMY (max=%d)\n", s->max_drony);
}

void sig_minus(int sig)
{
    sprzatnij_drony();

    int do_usuniecia = liczba_dronow / 2;
    printf("[OPERATOR] !!! USUWANIE PLATFORM (-%d DRONOW)\n", do_usuniecia);

    for (int i = 0; i < do_usuniecia; i++)
    {
        pid_t pid = drony[liczba_dronow - 1];
        kill(pid, SIGTERM);
        liczba_dronow--;

        printf("[OPERATOR] --- DRON PID=%d USUNIETY (SIGTERM)\n", pid);
    }


    semafor_p();
    s->max_drony = liczba_dronow;
    semafor_v();
}

int main()
{
    srand(time(NULL));
    printf("[OPERATOR] START\n");

    signal(SIGUSR1, sig_plus);
    signal(SIGUSR2, sig_minus);

    upd();
    upa();
    utworz_nowy_semafor();
    ustaw_semafor();

    s = (struct stan *)adres;

    semafor_p();
    s->aktywne_drony = 0;
    s->drony_w_bazie = 0;
    s->max_drony = N;
    semafor_v();

    for (int i = 0; i < N; i++)
    {
        stworz_drona();
        usleep(300000);
    }

    while (1)
    {
        sleep(TK);
        sprzatnij_drony();

        semafor_p();
        int aktywne = s->aktywne_drony;
        int max = s->max_drony;
        int w_bazie = s->drony_w_bazie;
        semafor_v();

        if (aktywne < max && w_bazie < P)
        {
            printf("[OPERATOR] >>> UZUPELNIANIE: %d -> %d\n", aktywne, aktywne + 1);
            stworz_drona();
        }

        semafor_p();
        printf("[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", s->aktywne_drony, s->drony_w_bazie, s->max_drony);
        semafor_v();
    }
}
