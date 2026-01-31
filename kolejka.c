#include "shared.h"

/* Identyfikator kolejki komunikatów System V */
int msg_id;

/* Utworzenie kolejki komunikatów i inicjalizacja wolnych wejść do bazy */
void utworz_kolejke() 
{
    key_t key = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'Q');
    if (key == -1) blad("ftok kolejka");

    msg_id = msgget(key, 0600 | IPC_CREAT);
    if (msg_id == -1) blad("msgget");

    /* Dwa komunikaty oznaczające dwa wolne wejścia */
    struct msg_wejscie m1 = {1, 0}; 
    struct msg_wejscie m2 = {2, 0}; 
    
    msgsnd(msg_id, &m1, sizeof(int), 0);
    msgsnd(msg_id, &m2, sizeof(int), 0);
}

/* Zajęcie wejścia – pobranie komunikatu z kolejki */
int przejdz_przez_wejscie(int pid) 
{
    struct msg_wejscie m;
    if (msgrcv(msg_id, &m, sizeof(int), 0, 0) == -1) {
        perror("msgrcv błąd"); 
        return -1;
    }
    return (int)m.mtype;
}

/* Zwolnienie wejścia – odesłanie komunikatu do kolejki */
void zwolnij_wejscie(int nr_wejscia, int pid) 
{
    struct msg_wejscie m = {(long)nr_wejscia, pid};
    if (msgsnd(msg_id, &m, sizeof(int), 0) == -1) 
    {
        perror("msgsnd błąd");
    }
}

/* Usunięcie kolejki komunikatów */
void usun_kolejke() 
{
    msgctl(msg_id, IPC_RMID, NULL);
}
