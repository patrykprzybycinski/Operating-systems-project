#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

/* Struktura komunikatu używana w kolejce wejść do bazy */
struct msg_wejscie 
{
    long mtype;    /* numer wejścia */
    int dron_pid;  /* PID drona korzystającego z wejścia */
};

/* Identyfikator kolejki komunikatów */
extern int msg_id;

/* Wspólny stan systemu przechowywany w pamięci dzielonej */
struct stan 
{
    int aktywne_drony;   /* liczba aktywnych dronów */
    int drony_w_bazie;   /* liczba dronów w bazie */
    int max_drony;       /* maksymalna liczba dronów */

    int N;               /* początkowa liczba dronów */
    int P;               /* pojemność bazy */
    int Tk;              /* czas uzupełniania */
    int XI;              /* maksymalna liczba ładowań */

    pid_t cel_ataku;     /* Cel ataku ( Sygnal 3)*/
};

/* Zmienne IPC współdzielone między plikami */
extern int pamiec;
extern char *adres;
extern int semafor;

/* Obsługa pamięci dzielonej */
void upd();
void upa();
void odlacz_pamiec();
void podlacz_pamiec();

/* Obsługa semafora */
void utworz_nowy_semafor();
void ustaw_semafor();
void semafor_p();
void semafor_v();
void usun_semafor();

/* Obsługa błędów */
void blad(const char *msg);

/* Obsługa logowania */
void log_init(const char *filename);
void log_msg(const char *msg);
void log_close();

/* Obsługa kolejki komunikatów (wejścia do bazy) */
void utworz_kolejke();
void usun_kolejke();
int przejdz_przez_wejscie(int pid);
void zwolnij_wejscie(int nr_wejscia, int pid);

#endif
