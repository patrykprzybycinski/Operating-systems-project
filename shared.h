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

struct msg_wejscie 
{
    long mtype;    
    int dron_pid;  
};

extern int msg_id;

struct stan 
{
    int aktywne_drony;
    int drony_w_bazie;
    int max_drony;

    int N;
    int P;
    int Tk;
    int XI;

    int wejscie1;
    int wejscie2;
};


extern int pamiec;
extern char *adres;
extern int semafor;

void upd();
void upa();
void odlacz_pamiec();

void utworz_nowy_semafor();
void ustaw_semafor();
void semafor_p();
void semafor_v();
void usun_semafor();
void blad(const char *msg);
void podlacz_pamiec();

void log_init(const char *filename);
void log_msg(const char *msg);
void log_close();

void utworz_kolejke();
void usun_kolejke();
int przejdz_przez_wejscie(int pid);
void zwolnij_wejscie(int nr_wejscia, int pid);

#endif
