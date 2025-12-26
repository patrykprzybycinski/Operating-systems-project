#include "shared.h"

int main() 
{
    printf("[DRON %d] Uruchomiony\n", getpid());

    int life = 0;

    while (1) 
    {
        printf("[DRON %d] Leci... (%d)\n", getpid(), life++);
        sleep(2);

        if (life >= 5) 
        {
            printf("[DRON %d] Zakończenie pracy\n", getpid());
            break;
        }
    }

    return 0;
}
