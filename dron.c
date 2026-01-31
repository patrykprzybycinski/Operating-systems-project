#include "shared.h"

typedef enum 
{
    LOT,
    POWROT,
    LADOWANIE
} stan_drona_t;

volatile sig_atomic_t atak = 0;
volatile sig_atomic_t stan_global;

void sig_atak(int sig)
{
    atak = 1;
}

int main() 
{
    char buf[128];  
    buf[0] = '\0';

    srand(getpid());
    log_init("system.log");

    struct sigaction sa;
    sa.sa_handler = sig_atak;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1) 
    {
        perror("sigaction SIGTERM");
        exit(1);
    }

    int T1 = (rand() % 34) + 7;          
    int T2 = (int)(2.5 * T1);            
    int T_return = (int)(0.2 * T2);   
    if (T_return < 1) T_return = 1;

    int drain = 100 / T2;             
    if (drain < 1) drain = 1;
    if (drain > 6) drain = 6;

    int bateria = 100;
    int ladowania = 0;
    int powrot_pozostalo = 0;

    stan_drona_t stan = LOT;
    stan_global = LOT;

    podlacz_pamiec();

    key_t key_sem = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT/ipc.key", 'S');
    if (key_sem == -1)
    {
        perror("ftok sem dron");
        exit(1);
    }

    semafor = semget(key_sem, 1, 0);
    if (semafor == -1)
    {
        blad("semget dron");
    }

    key_t key_q = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT/ipc.key", 'Q');
    if (key_q == -1) 
    {
        perror("ftok msg dron");
        exit(1);
    }

    msg_id = msgget(key_q, 0); 
    if (msg_id == -1) 
    {
        perror("msgget dron");
        exit(1);
    }

    struct stan *s = (struct stan *)adres;

    int P, XI;
    semafor_p();
    P = s->P;
    XI = s->XI;
    semafor_v();


    printf("[DRON %d] START | T1=%ds T2=%ds T_return=%ds drain=%d%%/s\n", getpid(), T1, T2, T_return, drain);

    sprintf(buf, "[DRON %d] START | T1=%ds T2=%ds T_return=%ds drain=%d%%/s\n", getpid(), T1, T2, T_return, drain);
    log_msg(buf);
    buf[0] = '\0';

    while (1) 
    {
        if (atak)
        {
            if (stan_global == LADOWANIE)
            {
                semafor_p();
                s->drony_w_bazie--;
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ZNISZCZONY W BAZIE (ATAK)\n", getpid());

                sprintf(buf, "[DRON %d] !!! ZNISZCZONY W BAZIE (ATAK)\n", getpid());
                log_msg(buf);
                buf[0] = '\0';

                exit(0);
            }
            
            if (bateria >= 20)
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ATAK SAMOBOJCZY\n", getpid());

                sprintf(buf, "[DRON %d] !!! ATAK SAMOBOJCZY\n", getpid());
                log_msg(buf);
                buf[0] = '\0';

                exit(0);
            }
            else
            {
                printf("[DRON %d] ATAK ZIGNOROWANY (bateria < 20%%)\n", getpid());

                sprintf(buf, "[DRON %d] ATAK ZIGNOROWANY (bateria < 20%%)\n", getpid());
                log_msg(buf);
                buf[0] = '\0';

                atak = 0;
            }
        }

        if (stan == LOT) 
        {
            sleep(1);

            bateria -= drain;
            if (bateria < 0) bateria = 0;

            printf("[DRON %d] LOT | bateria=%d%%\n", getpid(), bateria);

            sprintf(buf, "[DRON %d] LOT | bateria=%d%%\n", getpid(), bateria);
            log_msg(buf);
            buf[0] = '\0';

            if (bateria <= 20 && bateria > 0) 
            {
                stan = POWROT;
                stan_global = POWROT;
                powrot_pozostalo = T_return;

                printf("[DRON %d] >>> ROZPOCZYNAM POWROT (czas=%ds)\n", getpid(), powrot_pozostalo);

                sprintf(buf, "[DRON %d] >>> ROZPOCZYNAM POWROT (czas=%ds)\n", getpid(), powrot_pozostalo);
                log_msg(buf);
                buf[0] = '\0';
            }

            if (bateria <= 0) 
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ZNISZCZONY W LOCIE (bateria=0%%)\n", getpid());

                sprintf(buf, "[DRON %d] !!! ZNISZCZONY W LOCIE (bateria=0%%)\n", getpid());
                log_msg(buf);
                buf[0] = '\0';

                exit(0);
            }
        }
        else if (stan == POWROT) 
        {

            sleep(1);

            bateria -= drain;
            powrot_pozostalo--;

            if (bateria < 0) bateria = 0;

            printf("[DRON %d] POWROT | bateria=%d%% | pozostalo=%ds\n", getpid(), bateria, powrot_pozostalo);

            sprintf(buf, "[DRON %d] POWROT | bateria=%d%% | pozostalo=%ds\n", getpid(), bateria, powrot_pozostalo);
            log_msg(buf);
            buf[0] = '\0';

            if (bateria <= 0) 
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ZNISZCZONY W TRAKCIE POWROTU\n", getpid());

                sprintf(buf, "[DRON %d] !!! ZNISZCZONY W TRAKCIE POWROTU\n", getpid());
                log_msg(buf);
                buf[0] = '\0';

                exit(0);
            }

            if (powrot_pozostalo <= 0) 
            {
                int mozna_wejsc = 0;

                semafor_p();
                if (s->drony_w_bazie < s->P) 
                {
                    mozna_wejsc = 1;
                }
                semafor_v();

                if (mozna_wejsc) 
                {
                    struct msg_wejscie m;
                    
                    if (msgrcv(msg_id, &m, sizeof(int), 0, IPC_NOWAIT) == -1) 
                        {
                            if (errno == ENOMSG) 
                            {
                                printf("[DRON %d] !!! WEJŚCIA ZAJĘTE - KRĄŻĘ (bateria: %d%%)\n", getpid(), bateria);
                                continue; 
                            } 
                            else 
                            {
                                perror("Błąd msgrcv");
                                continue;
                            }
                        }

                    long nr_wejscia = m.mtype;

                    printf("[DRON %d] >>> PRZECHODZE PRZEZ WEJSCIE %ld\n", getpid(), nr_wejscia);

                    sleep(1); 

                    semafor_p();
                    if (s->drony_w_bazie < s->P) 
                    {
                        s->drony_w_bazie++;
                        semafor_v();

                        m.mtype = nr_wejscia;
                        m.dron_pid = getpid();
                        msgsnd(msg_id, &m, sizeof(int), 0);

                        printf("[DRON %d] >>> DOTARL DO BAZY (Wejscie %ld)\n", getpid(), nr_wejscia);
                        sprintf(buf, "[DRON %d] >>> DOTARL DO BAZY (Wejscie %ld)\n", getpid(), nr_wejscia);
                        log_msg(buf);

                        stan = LADOWANIE;
                        stan_global = LADOWANIE;
                    }
                    else 
                    {
                        semafor_v();
                        
                        m.mtype = nr_wejscia;
                        msgsnd(msg_id, &m, sizeof(int), 0);
                        
                        printf("[DRON %d] !!! BAZA ZAPEŁNIONA W OSTATNIEJ CHWILI - KRĄŻĘ DALEJ\n", getpid());
                    }
                }
                else 
                {
                    printf("[DRON %d] !!! BRAK MIEJSC W BAZIE - OCZEKIWANIE (Bateria: %d%%)\n", getpid(), bateria);
                    
                    sleep(1);
                    bateria -= drain; 
                    
                    if (bateria <= 0) 
                    {
                        semafor_p();
                        s->aktywne_drony--;
                        semafor_v();
                        printf("[DRON %d] !!! ROZBITTY POD BAZĄ (0%% Baterii)\n", getpid());
                        exit(0);
                    }
                }
            }
        }
        else if (stan == LADOWANIE) 
        {
            printf("[DRON %d] <<< LADOWANIE (%ds)\n", getpid(), T1);

            sprintf(buf, "[DRON %d] <<< LADOWANIE (%ds)\n", getpid(), T1);
            log_msg(buf);
            buf[0] = '\0';

           unsigned int left = sleep(T1);
            
            if (left > 0 && atak) 
            {
                continue; 
            }

            bateria = 100;
            ladowania++;

            semafor_p();
            s->drony_w_bazie--;
            semafor_v();

            printf("[DRON %d] <<< WYLOT Z BAZY | bateria=100%% | ladowania=%d\n", getpid(), ladowania);

            sprintf(buf, "[DRON %d] <<< WYLOT Z BAZY | bateria=100%% | ladowania=%d\n", getpid(), ladowania);
            log_msg(buf);
            buf[0] = '\0';

            if (ladowania >= XI) 
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] XXX UTYLIZACJA (Xi=%d)\n", getpid(), XI);

                sprintf(buf, "[DRON %d] XXX UTYLIZACJA (Xi=%d)\n", getpid(), XI);
                log_msg(buf);
                buf[0] = '\0';

                exit(0);
            }

            stan = LOT;
            stan_global = LOT;
        }
    }
}

