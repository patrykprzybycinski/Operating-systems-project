#include "shared.h"

int main() 
{
    printf("[DOWODCA] Uruchomiono dowódce\n");

    pid_t pid = fork();

    if (pid < 0) 
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0) 
    {
        execl("./operator", "operator", NULL);
        perror("exec operator");
        exit(1);
    }

    while (1) 
    {
        sleep(5);
        printf("[DOWODCA] System działa...\n");
    }

    return 0;
}
