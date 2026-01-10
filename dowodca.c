#include "shared.h"

pid_t operator_pid;

int main() 
{
    printf("[DOWODCA] START\n");

    operator_pid = fork();

    if (operator_pid == 0) 
    {
        execl("./operator", "operator", NULL);
        exit(1);
    }

    while (1)
    {
        sleep(rand() % 5 + 5);

        int decyzja = rand() % 4;
       
        if (decyzja == 1)
        {
            printf("[DOWODCA] >>> ROZBUDOWA PLATFORM\n");
            kill(operator_pid, SIGUSR1);
        }
        else if (decyzja == 2)
        {
            printf("[DOWODCA] >>> REDUKCJA PLATFORM\n");
            kill(operator_pid, SIGUSR2);
        }
        else if (decyzja == 3)
        {
            printf("[DOWODCA] >>> ATAK SAMOBOJCZY (1 DRON)\n");
            kill(operator_pid, SIGUSR1 + 2); 
        }
        else
        {
            printf("[DOWODCA] ... brak decyzji\n");
        }

    }
}