#include "shared.h"

/* ID segmentu pamięci dzielonej nadane przez kernel */
int pamiec;

/* Adres segmentu pamięci w przestrzeni procesu */
char *adres;

/* Tworzy lub pobiera segment pamięci dzielonej */
void upd()
{
    /* Generuje wspólny klucz IPC na podstawie pliku i znaku */
    key_t key_shm = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'M');

    /* Jeśli nie udało się wygenerować klucza, kończymy program */
    if (key_shm == -1)
        blad("ftok shm");

    /* Tworzy segment pamięci lub pobiera istniejący */
    pamiec = shmget(key_shm, 256, 0600 | IPC_CREAT);

    /* Sprawdzenie błędu utworzenia segmentu */
    if (pamiec == -1)
        blad("shmget");
}

/* Podłącza pamięć dzieloną do procesu */
void upa()
{
    /* Mapuje segment pamięci do przestrzeni adresowej procesu */
    adres = (char *)shmat(pamiec, NULL, 0);

    /* Sprawdzenie poprawności podłączenia */
    if (adres == (char *)(-1))
        blad("shmat");
}

/* Odłącza pamięć od procesu i zgłasza ją do usunięcia */
void odlacz_pamiec()
{
    /* Odłączenie pamięci od przestrzeni adresowej procesu */
    shmdt(adres);

    /* Oznaczenie segmentu do usunięcia po odłączeniu wszystkich procesów */
    shmctl(pamiec, IPC_RMID, 0);
}

/* Dołącza proces do już istniejącej pamięci dzielonej */
void podlacz_pamiec()
{
    /* Generuje ten sam klucz IPC co proces tworzący pamięć */
    key_t key_shm = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'M');

    /* Błąd generowania klucza */
    if (key_shm == -1)
        blad("ftok shm");

    /* Pobranie ID istniejącego segmentu pamięci */
    pamiec = shmget(key_shm, 256, 0);

    /* Sprawdzenie błędu pobrania segmentu */
    if (pamiec == -1)
        blad("shmget attach");

    /* Podłączenie pamięci do procesu */
    upa();
}
