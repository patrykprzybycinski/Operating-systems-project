#include "shared.h"

typedef enum 
{
    LOT,
    POWROT,
    LADOWANIE
} stan_drona_t;

volatile sig_atomic_t atak = 0;

void sig_atak(int sig)
{
    atak = 1;
}

int main() 
{
    char buf[128];   /* JEDYNY BUFOR */
    buf[0] = '\0';

    srand(getpid());
    log_init("system.log");

    if (signal(SIGTERM, sig_atak) == SIG_ERR)
    {
        perror("signal SIGTERM");
    }

    int T1 = rand() % 10 + 10;          
    int T2 = (int)(2.5 * T1);         
    int T_return = (int)(0.2 * T2);   
    if (T_return < 1) T_return = 1;

    int drain = 100 / T2;             
    if (drain < 1) drain = 1;

    int bateria = 100;
    int ladowania = 0;
    int powrot_pozostalo = 0;

    stan_drona_t stan = LOT;

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

    struct stan *s = (struct stan *)adres;

    printf("[DRON %d] START | T1=%ds T2=%ds T_return=%ds drain=%d%%/s\n", getpid(), T1, T2, T_return, drain);

    sprintf(buf, "[DRON %d] START | T1=%ds T2=%ds T_return=%ds drain=%d%%/s\n", getpid(), T1, T2, T_return, drain);
    log_msg(buf);
    buf[0] = '\0';

    while (1) 
    {
        if (atak)
        {
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
            if (sleep(1) != 0 && errno == EINTR)
            {
                perror("sleep");
            }

            bateria -= drain;
            if (bateria < 0) bateria = 0;

            printf("[DRON %d] LOT | bateria=%d%%\n", getpid(), bateria);

            sprintf(buf, "[DRON %d] LOT | bateria=%d%%\n", getpid(), bateria);
            log_msg(buf);
            buf[0] = '\0';

            if (bateria <= 20 && bateria > 0) 
            {
                stan = POWROT;
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
            if (sleep(1) != 0 && errno == EINTR)
            {
                perror("sleep");
            }

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
                semafor_p();
                if (s->drony_w_bazie < P) 
                {
                    s->drony_w_bazie++;
                    semafor_v();

                    printf("[DRON %d] >>> DOTARL DO BAZY\n", getpid());

                    sprintf(buf, "[DRON %d] >>> DOTARL DO BAZY\n", getpid());
                    log_msg(buf);
                    buf[0] = '\0';

                    stan = LADOWANIE;
                } 
                else 
                {
                    semafor_v();

                    printf("[DRON %d] !!! POD BAZA – BRAK MIEJSC\n", getpid());

                    sprintf(buf, "[DRON %d] !!! POD BAZA – BRAK MIEJSC\n", getpid());
                    log_msg(buf);
                    buf[0] = '\0';
                }
            }
        }
        else if (stan == LADOWANIE) 
        {
            printf("[DRON %d] <<< LADOWANIE (%ds)\n", getpid(), T1);

            sprintf(buf, "[DRON %d] <<< LADOWANIE (%ds)\n", getpid(), T1);
            log_msg(buf);
            buf[0] = '\0';

            if (sleep(T1) != 0 && errno == EINTR)
            {
                perror("sleep T1");
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
        }
    }
}
