#include "shared.h"

struct stan *s;

int main() 
{
    printf("[OPERATOR] START\n");

    upd(); 
    upa();
    utworz_nowy_semafor();
    ustaw_semafor();

    s = (struct stan *)adres;

    semafor_p();
    s->aktywne_drony = N;
    s->drony_w_bazie = 0;
    s->max_drony = P;
    semafor_v();

    for (int i = 0; i < N; i++) 
    {
        if (fork() == 0) 
        {
            execl("./dron", "dron", NULL);
            exit(1);
        }
        usleep(300000);
    }

    while (1) 
    {
        sleep(5);
        semafor_p();

        printf("[OPERATOR] aktywne=%d baza=%d max=%d\n", s->aktywne_drony, s->drony_w_bazie, s->max_drony);
        semafor_v();
    }
}