#include "shared.h"

pid_t operator_pid;

int main()
{
    srand(time(NULL));

    log_init("system.log");

    printf("[DOWODCA] START\n");
    log_msg("[DOWODCA] START\n");

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
