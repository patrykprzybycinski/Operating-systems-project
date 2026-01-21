#include "shared.h"

pid_t operator_pid;

int wczytaj_int(const char *opis, int min, int max)
{
    int x;
    while (1)
    {
        printf("%s (%d-%d): ", opis, min, max);
        fflush(stdout);

        if (scanf("%d", &x) != 1)
        {
            printf("Niepoprawna wartość\n");
            while (getchar() != '\n');
            continue;
        }

        if (x < min || x > max)
        {
            printf("Wartość poza zakresem\n");
            continue;
        }

        return x;
    }
}


int main()
{
    remove("system.log");

    printf("[DOWODCA] KONFIGURACJA SYSTEMU\n");

    int N = wczytaj_int("Podaj N (liczba dronów)", 1, 100);
    int P = wczytaj_int("Podaj P (pojemność bazy)", 1, (N/2));
    int Tk = wczytaj_int("Podaj Tk (czas uzupełniania)", 1, 60);
    int XI = wczytaj_int("Podaj Xi (liczba ładowań)", 1, 10);

    srand(time(NULL));

    log_init("system.log");

    printf("[DOWODCA] START\n");
    log_msg("[DOWODCA] START\n");

    upd();
    upa();
    utworz_nowy_semafor();
    ustaw_semafor();

    struct stan *s = (struct stan *)adres;

    semafor_p();
    s->N = N;
    s->P = P;
    s->Tk = Tk;
    s->XI = XI;

    s->max_drony = N;
    s->aktywne_drony = 0;
    s->drony_w_bazie = 0;
    semafor_v();

    operator_pid = fork();
    if (operator_pid == -1)
    {
        perror("fork");
        exit(1);
    }

    if (operator_pid == 0)
    {
        execl("./operator", "operator", NULL);
        perror("execl operator");
        exit(1);
    }

    while (1)
    {
        if (sleep(rand() % 5 + 5) != 0 && errno == EINTR)
        {
            perror("sleep");
        }

        int decyzja = rand() % 4;

        if (decyzja == 1)
        {
            printf("[DOWODCA] >>> ROZBUDOWA PLATFORM\n");
            log_msg("[DOWODCA] >>> ROZBUDOWA PLATFORM\n");

            if (kill(operator_pid, SIGUSR1) == -1)
            {
                perror("kill SIGUSR1");
            }
        }
        else if (decyzja == 2)
        {
            printf("[DOWODCA] >>> REDUKCJA PLATFORM\n");
            log_msg("[DOWODCA] >>> REDUKCJA PLATFORM\n");

            if (kill(operator_pid, SIGUSR2) == -1)
            {
                perror("kill SIGUSR2");
            }
        }
        else if (decyzja == 3)
        {
            printf("[DOWODCA] >>> ATAK SAMOBOJCZY\n");
            log_msg("[DOWODCA] >>> ATAK SAMOBOJCZY\n");

            if (kill(operator_pid, SIGUSR1 + 2) == -1)
            {
                perror("kill ATAK");
            }
        }
        else
        {
            printf("[DOWODCA] ... brak decyzji\n");
            log_msg("[DOWODCA] ... brak decyzji\n");
        }
    }
}
