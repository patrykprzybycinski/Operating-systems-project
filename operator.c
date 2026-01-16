#include "shared.h"
#include <sys/wait.h>

struct stan *s;

pid_t drony[100];
int liczba_dronow = 0;

void sprzatnij_drony()
{
    int status;
    pid_t pid;
    char buf[128];

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < liczba_dronow; i++)
        {
            if (drony[i] == pid)
            {
                drony[i] = drony[liczba_dronow - 1];
                liczba_dronow--;

                printf("[OPERATOR] DRON PID=%d USUNIETY Z LISTY\n", pid);

                sprintf(buf, "[OPERATOR] DRON PID=%d USUNIETY Z LISTY\n", pid);
                log_msg(buf);
                buf[0] = '\0';

                break;
            }
        }
    }
}

void stworz_drona()
{
    char buf[128];

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork dron");
        return;
    }

    if (pid == 0)
    {
        execl("./dron", "dron", NULL);
        perror("execl dron");
        exit(1);
    }

    drony[liczba_dronow++] = pid;

    semafor_p();
    s->aktywne_drony++;
    semafor_v();

    printf("[OPERATOR] +++ NOWY DRON PID=%d\n", pid);

    sprintf(buf, "[OPERATOR] +++ NOWY DRON PID=%d\n", pid);
    log_msg(buf);
    buf[0] = '\0';
}

void sig_plus(int sig)
{
    char buf[128];

    semafor_p();
    s->max_drony = 2 * N;
    semafor_v();

    printf("[OPERATOR] !!! DODANO PLATFORMY (max=%d)\n", s->max_drony);

    sprintf(buf, "[OPERATOR] !!! DODANO PLATFORMY (max=%d)\n", s->max_drony);
    log_msg(buf);
    buf[0] = '\0';
}

void sig_minus(int sig)
{
    char buf[128];

    sprzatnij_drony();

    int do_usuniecia = liczba_dronow / 2;

    printf("[OPERATOR] !!! USUWANIE PLATFORM (-%d DRONOW)\n", do_usuniecia);

    sprintf(buf, "[OPERATOR] !!! USUWANIE PLATFORM (-%d DRONOW)\n", do_usuniecia);
    log_msg(buf);
    buf[0] = '\0';

    for (int i = 0; i < do_usuniecia; i++)
    {
        pid_t pid = drony[liczba_dronow - 1];

        if (kill(pid, SIGTERM) == -1)
        {
            perror("kill SIGTERM");
        }

        liczba_dronow--;

        printf("[OPERATOR] --- DRON PID=%d USUNIETY (SIGTERM)\n", pid);

        sprintf(buf, "[OPERATOR] --- DRON PID=%d USUNIETY (SIGTERM)\n", pid);
        log_msg(buf);
        buf[0] = '\0';
    }

    semafor_p();
    s->max_drony = liczba_dronow;
    semafor_v();
}

void cleanup(int sig)
{
    char buf[128];

    sprintf(buf, "[OPERATOR] ZAKONCZENIE PROGRAMU (sygnal %d)\n", sig);
    log_msg(buf);

    for (int i = 0; i < liczba_dronow; i++)
    {
        kill(drony[i], SIGTERM);
    }

    sleep(1); 

    usun_semafor();
    odlacz_pamiec();

    log_msg("[OPERATOR] USUNIETO SEMAFOR I PAMIEC DZIELONA\n");

    log_close();
    exit(0);
}


int main()
{
    char buf[128];
    buf[0] = '\0';

    srand(time(NULL));
    log_init("system.log");

    signal(SIGINT, cleanup);

    printf("[OPERATOR] START\n");
    log_msg("[OPERATOR] START\n");

    if (signal(SIGUSR1, sig_plus) == SIG_ERR)
    {
        perror("signal SIGUSR1");
    }

    if (signal(SIGUSR2, sig_minus) == SIG_ERR)
    {
        perror("signal SIGUSR2");
    }

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

            sprintf(buf, "[OPERATOR] >>> UZUPELNIANIE: %d -> %d\n", aktywne, aktywne + 1);
            log_msg(buf);
            buf[0] = '\0';

            stworz_drona();
        }

        semafor_p();
        printf("[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", s->aktywne_drony, s->drony_w_bazie, s->max_drony);

        sprintf(buf, "[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", s->aktywne_drony, s->drony_w_bazie, s->max_drony);
        log_msg(buf);
        buf[0] = '\0';
        semafor_v();
    }
}
