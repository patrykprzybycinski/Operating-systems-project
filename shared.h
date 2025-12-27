#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>

#define MAX_DRONES 100

typedef struct 
{
    int current_drones;
    int max_drones;
    int drones_in_base;
} shared_data_t;

typedef struct 
{
    long mtype;
    int value;
} message_t;

static void poczatek(void);
static void utworz_nowy_semafor(void);
static void ustaw_semafor(void);
static void semafor_p(void);
static void semafor_v(void);
static void usun_semafor(void);

#endif
