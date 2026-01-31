#include "shared.h"
#include <sys/wait.h>

struct stan *s;

pid_t *drony = NULL;
int liczba_dronow = 0;
int pojemnosc_dronow = 0;

void przekaz_atak(int sig)
{
    if (liczba_dronow > 0)
    {
        int index = rand() % liczba_dronow;
        pid_t wybrany_dron = drony[index];
        
        printf("[OPERATOR] Przekazuje rozkaz ataku do drona PID=%d \n", wybrany_dron);
        log_msg("[OPERATOR] Przekazanie SIGTERM i redukcja max_drony\n");
        
        kill(wybrany_dron, SIGTERM);
    }
}

void alokacja_dronow()
{
    if (liczba_dronow < pojemnosc_dronow)
        return;

    int nowa = (pojemnosc_dronow == 0) ? 16 : pojemnosc_dronow * 2;

    pid_t *tmp = realloc(drony, nowa * sizeof(pid_t));
    if (!tmp)
    {
        perror("realloc drony");
        exit(1);
    }

    drony = tmp;
    pojemnosc_dronow = nowa;
}

void sprzatnij_drony(int sig)
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

    alokacja_dronow();
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
    s->max_drony = 2 * s->N;
    semafor_v();

    printf("[OPERATOR] !!! DODANO PLATFORMY (max=%d)\n", s->max_drony);

    sprintf(buf, "[OPERATOR] !!! DODANO PLATFORMY (max=%d)\n", s->max_drony);
    log_msg(buf);
    buf[0] = '\0';
}

void sig_minus(int sig)
{
    char buf[128];

    int stare_max;

    semafor_p();
    stare_max = s->max_drony;
    semafor_v();

    if (liczba_dronow <= 0)
    {
        printf("[OPERATOR] !!! BRAK DRONOW DO USUNIECIA\n");
    }

    int do_usuniecia = stare_max / 2;
    if (do_usuniecia == 0 && stare_max > 0)
    {
        do_usuniecia = 1;
    }


    printf("[OPERATOR] !!! USUWANIE PLATFORM (-%d DRONOW)\n", do_usuniecia);

    sprintf(buf, "[OPERATOR] !!! USUWANIE PLATFORM (-%d DRONOW)\n", do_usuniecia);
    log_msg(buf);
    buf[0] = '\0';

    int wyslane = 0;

    for (int i = 0; i < liczba_dronow && wyslane < do_usuniecia; i++)
    {
        pid_t pid = drony[i];

        if (kill(pid, SIGTERM) == -1)
        {
            if (errno != ESRCH)
                perror("kill SIGTERM");
            continue;
        }

        printf("[OPERATOR] --- DRON PID=%d USUNIETY (SIGTERM)\n", pid);
        sprintf(buf, "[OPERATOR] --- DRON PID=%d USUNIETY (SIGTERM)\n", pid);
        log_msg(buf);
        buf[0] = '\0';

        wyslane++;
    }

    semafor_p();
    s->max_drony -= do_usuniecia;
    if (s->max_drony < 0)
    {
        s->max_drony = 0;
    }
    semafor_v();

    if (s->max_drony == 0)
    {
        printf("[OPERATOR] !!! LICZBA PLATFORM OSIAGNELA ZERO\n");
    }

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

    free(drony);
    drony = NULL;
    pojemnosc_dronow = 0;

    usun_semafor();
    odlacz_pamiec();

    log_msg("[OPERATOR] USUNIETO SEMAFOR I PAMIEC DZIELONA\n");

    if (msgctl(msg_id, IPC_RMID, NULL) == -1) 
    {
        perror("Błąd usuwania kolejki"); 
    }
    
    log_msg("[OPERATOR] USUNIETO KOLEJKE KOMUNIKATOW\n");

    log_close();
    exit(0);
}

int main()
{
    char buf[128];
    buf[0] = '\0';

    srand(time(NULL));
    log_init("system.log");
    key_t key_q = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT/ipc.key", 'Q');
    msg_id = msgget(key_q, 0600 | IPC_CREAT); 

    struct msg_wejscie m1 = {1, 0}; 
    struct msg_wejscie m2 = {2, 0}; 

    msgsnd(msg_id, &m1, sizeof(int), 0);
    msgsnd(msg_id, &m2, sizeof(int), 0); 

    struct sigaction sa;
    sa.sa_handler = sprzatnij_drony;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    signal(SIGINT, cleanup);

    printf("[OPERATOR] START\n");
    log_msg("[OPERATOR] START\n");

    struct sigaction sa_p;
    sa_p.sa_handler = sig_plus;
    sigemptyset(&sa_p.sa_mask);
    sa_p.sa_flags = 0; 
    sigaction(SIGUSR1, &sa_p, NULL);

    struct sigaction sa_m;
    sa_m.sa_handler = sig_minus;
    sigemptyset(&sa_m.sa_mask);
    sa_m.sa_flags = 0; 
    sigaction(SIGUSR2, &sa_m, NULL);

    struct sigaction sa_a;
    sa_a.sa_handler = przekaz_atak;
    sigemptyset(&sa_a.sa_mask);
    sa_a.sa_flags = 0;
    sigaction(SIGWINCH, &sa_a, NULL);

    upd();
    upa();
    utworz_nowy_semafor();
    ustaw_semafor();

    s = (struct stan *)adres;

    semafor_p();
    int N = s->N;
    int P = s->P;
    int TK = s->Tk;
    semafor_v();

    for (int i = 0; i < N; i++)
    {
        stworz_drona();
    }

    time_t nastepne_uzupelnienie = time(NULL) + TK;

    while (1)
    {
        semafor_p();
        int aktualne_max = s->max_drony;
        int aktualne_aktywne = s->aktywne_drony;
        semafor_v();

        if (aktualne_max <= 0 && aktualne_aktywne <= 0)
        {
            printf("\n[OPERATOR] ########################################\n");
            printf("[OPERATOR] !!! BRAK DOSTEPNYCH PLATFORM I DRONOW !!!\n");
            printf("[OPERATOR] !!! ZAMYKANIE SYSTEMU PRZEZ OPERATORA !!!\n");
            printf("[OPERATOR] ########################################\n\n");
            log_msg("[OPERATOR] Koniec symulacji - max_drony = 0\n");
            cleanup(0); 
        }

        time_t teraz = time(NULL);
        if (teraz - nastepne_uzupelnienie >= TK)
        {
            semafor_p();
            int aktywne = s->aktywne_drony;
            int max = s->max_drony;
            int w_bazie = s->drony_w_bazie;
            int limit_P = s->P;
            semafor_v();

            if (aktywne < max && w_bazie < limit_P)
            {
                printf("[OPERATOR] >>> UZUPELNIANIE: %d -> %d\n", aktywne, aktywne + 1);
                sprintf(buf, "[OPERATOR] >>> UZUPELNIANIE: %d -> %d\n", aktywne, aktywne + 1);
                log_msg(buf);
                stworz_drona();
            }

            nastepne_uzupelnienie = teraz;

            semafor_p();
            printf("[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", s->aktywne_drony, s->drony_w_bazie, s->max_drony);
            sprintf(buf, "[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", s->aktywne_drony, s->drony_w_bazie, s->max_drony);
            log_msg(buf);
            semafor_v();
        }

        sleep(1); 
    }
}