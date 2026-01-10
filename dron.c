#include "shared.h"

typedef enum {
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
    srand(getpid());

    signal(SIGTERM, sig_atak);

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

    upd();
    upa();

    semafor = semget(130, 1, 0);

    if (semafor == -1) 
    {
        perror("semget dron");
        exit(1);
    }

    struct stan *s = (struct stan *)adres;

    printf("[DRON %d] START | T1=%ds T2=%ds T_return=%ds drain=%d%%/s\n", getpid(), T1, T2, T_return, drain);

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
                exit(0);
            }
            else
            {
                printf("[DRON %d] ATAK ZIGNOROWANY (bateria < 20%%)\n", getpid());
                atak = 0;
            }
        }

        if (stan == LOT) 
        {
            sleep(1);
            bateria -= drain;
            if (bateria < 0) bateria = 0;

            printf("[DRON %d] LOT | bateria=%d%%\n", getpid(), bateria);

            if (bateria <= 20 && bateria > 0) 
            {
                stan = POWROT;
                powrot_pozostalo = T_return;
                printf("[DRON %d] >>> ROZPOCZYNAM POWROT (czas=%ds)\n", getpid(), powrot_pozostalo);
            }

            if (bateria <= 0) 
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ZNISZCZONY W LOCIE (bateria=0%%)\n", getpid());
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

            if (bateria <= 0) 
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ZNISZCZONY W TRAKCIE POWROTU\n", getpid());
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
                    stan = LADOWANIE;
                } 
                else 
                {
                    semafor_v();
                    printf("[DRON %d] !!! POD BAZA – BRAK MIEJSC\n", getpid());
                }
            }
        }

        else if (stan == LADOWANIE) 
        {
            printf("[DRON %d] <<< LADOWANIE (%ds)\n", getpid(), T1);
            sleep(T1);

            bateria = 100;
            ladowania++;

            semafor_p();
            s->drony_w_bazie--;
            semafor_v();

            printf("[DRON %d] <<< WYLOT Z BAZY | bateria=100%% | ladowania=%d\n", getpid(), ladowania);

            if (ladowania >= XI) 
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] XXX UTYLIZACJA (Xi=%d)\n", getpid(), XI);
                exit(0);
            }

            stan = LOT;
        }
    }
}
