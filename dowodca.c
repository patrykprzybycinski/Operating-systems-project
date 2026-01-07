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
        sleep(15);
        printf("[DOWODCA] system działa\n");
    }
}