#include "shared.h"

int main() 
{
    printf("[OPERATOR] Start operatora\n");

    int shm_id = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0600);
    if (shm_id == -1) 
    {
        perror("shmget");
        exit(1);
    }

    shared_data_t *data = shmat(shm_id, NULL, 0);
    if (data == (void *)-1) 
    {
        perror("shmat");
        exit(1);
    }

    data->current_drones = 0;
    data->max_drones = 5;
    data->drones_in_base = 0;

    printf("[OPERATOR] Pamięć dzielona utworzona\n");

    pid_t pid = fork();
    if (pid == 0) 
    {
        execl("./dron", "dron", NULL);
        perror("exec dron");
        exit(1);
    }

    while (1) 
    {
        sleep(5);
        printf("[OPERATOR] Aktywne drony: %d\n", data->current_drones);
    }

    return 0;
}
