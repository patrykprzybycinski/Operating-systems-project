#include "shared.h"

/* ID kolejki komunikatów System V nadane przez kernel */
int msg_id;

/*
 * Tworzy kolejkę komunikatów i inicjalizuje ją dwoma wolnymi wejściami do bazy.
 * Każdy komunikat w kolejce reprezentuje jedno wolne wejście.
 */
void utworz_kolejke() 
{
    /* Generuje wspólny klucz IPC dla kolejki komunikatów */
    key_t key = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'Q');

    /* Sprawdzenie błędu generowania klucza */
    if (key == -1)
        blad("ftok kolejka");

    /* Tworzy kolejkę komunikatów lub pobiera już istniejącą */
    msg_id = msgget(key, 0600 | IPC_CREAT);

    /* Sprawdzenie błędu utworzenia kolejki */
    if (msg_id == -1)
        blad("msgget");

    /*
     * Inicjalizacja kolejki:
     *  - wysyłamy dwa komunikaty
     *  - każdy komunikat oznacza jedno wolne wejście do bazy
     */
    struct msg_wejscie m1 = {1, 0};
    struct msg_wejscie m2 = {2, 0};

    msgsnd(msg_id, &m1, sizeof(int), 0);
    msgsnd(msg_id, &m2, sizeof(int), 0);
}

/*
 * Zajmuje wejście do bazy.
 * Proces blokuje się, dopóki w kolejce nie pojawi się wolne wejście.
 */
int przejdz_przez_wejscie(int pid) 
{
    struct msg_wejscie m;

    /*
     * Pobiera komunikat z kolejki:
     *  - jeśli kolejka jest pusta, proces czeka
     *  - odebrany komunikat oznacza zajęcie wejścia
     */
    if (msgrcv(msg_id, &m, sizeof(int), 0, 0) == -1) 
    {
        perror("msgrcv błąd");
        return -1;
    }

    /* Zwracamy numer zajętego wejścia */
    return (int)m.mtype;
}

/*
 * Zwalnia wejście do bazy.
 * Wysyła komunikat z powrotem do kolejki, czyniąc wejście znów dostępnym.
 */
void zwolnij_wejscie(int nr_wejscia, int pid) 
{
    /* Przygotowanie komunikatu reprezentującego wolne wejście */
    struct msg_wejscie m = {(long)nr_wejscia, pid};

    /* Odesłanie komunikatu do kolejki */
    if (msgsnd(msg_id, &m, sizeof(int), 0) == -1) 
    {
        perror("msgsnd błąd");
    }
}

/*
 * Usuwa kolejkę komunikatów z systemu.
 * Po tej operacji żaden proces nie może z niej korzystać.
 */
void usun_kolejke() 
{
    /* Trwałe usunięcie kolejki z kernela */
    msgctl(msg_id, IPC_RMID, NULL);
}
