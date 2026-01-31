#include "shared.h"
#include <sys/wait.h>

pid_t operator_pid;

void obsluga_zakonczenia_operatora(int sig)
{
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0) 
    {
        printf("\n[DOWODCA] Odebrano sygnał zakończenia od Operatora (PID: %d).\n", pid);
        printf("[DOWODCA] System został zamknięty poprawnie.\n");
        exit(0);
    }
}

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

    int N = wczytaj_int("Podaj N (liczba dronów)", 1, 10000);
    int P = wczytaj_int("Podaj P (pojemność bazy)", 0, (N/2));
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

    struct sigaction sa_exit;
    sa_exit.sa_handler = obsluga_zakonczenia_operatora;
    sigemptyset(&sa_exit.sa_mask);
    sa_exit.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_exit, NULL) == -1) 
    {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    printf("\n=== SYSTEM STEROWANIA DRONAMI ===\n");
    printf("1 - [SIGUSR1] Rozbudowa platform (max drony x2)\n");
    printf("2 - [SIGUSR2] Redukcja platform (liczba dronow -50%%)\n");
    printf("3 - [SIGWINCH] Atak samobójczy (losowy dron)\n");
    printf("q - Zakoncz symulacje\n");
    printf("=================================\n");

    int wybor;
    while (1)
    {
        printf("\nDecyzja dowodcy > ");
        fflush(stdout);

        wybor = getchar();
        if (wybor == '\n') continue;
        while (getchar() != '\n'); 

        if (wybor == '1')
        {
            printf("[DOWODCA] Rozkaz: ROZBUDOWA\n");
            log_msg("[DOWODCA] Sygnal ROZBUDOWA (manual)\n");
            kill(operator_pid, SIGUSR1);
        }
        else if (wybor == '2')
        {
            printf("[DOWODCA] Rozkaz: REDUKCJA\n");
            log_msg("[DOWODCA] Sygnal REDUKCJA (manual)\n");
            kill(operator_pid, SIGUSR2);
        }
        else if (wybor == '3')
        {
            printf("[DOWODCA] Rozkaz: ATAK SAMOBOJCZY\n");
            log_msg("[DOWODCA] Sygnal ATAK (manual)\n");
            kill(operator_pid, SIGWINCH); 
        }
        else if (wybor == 'q')
        {
            printf("[DOWODCA] Konczenie symulacji...\n");
            kill(operator_pid, SIGINT);
            break;
        }
    }
}
