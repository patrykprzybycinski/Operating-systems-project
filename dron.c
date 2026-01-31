#include "shared.h"

typedef enum 
{
    LOT,
    POWROT,
    LADOWANIE
} stan_drona_t; // Definicja stanów automatu skończonego drona

volatile sig_atomic_t atak = 0; // Flaga sygnału ataku (bezpieczna dla operacji asynchronicznych)
volatile sig_atomic_t stan_global; // Zmienna pomocnicza do śledzenia stanu w handlerze

/* Obsługa sygnału SIGTERM - ustawienie flagi ataku samobójczego */
void sig_atak(int sig)
{
    atak = 1; // Zmiana flagi - dron sprawdzi ją w głównej pętli
}

int main() 
{
    char buf[128];  
    buf[0] = '\0';

    srand(getpid()); // Ziarno losowości unikalne dla każdego drona (używamy PID)
    log_init("system.log"); // Podłączenie do wspólnego pliku logów

    // Konfiguracja obsługi sygnału SIGTERM (rozkaz ataku od Operatora)
    struct sigaction sa;
    sa.sa_handler = sig_atak;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1) 
    {
        perror("sigaction SIGTERM");
        exit(1);
    }

    /* Generowanie unikalnych parametrów fizycznych drona */
    int T1 = (rand() % 34) + 7;          // Czas ładowania (7-40s)
    int T2 = (int)(2.5 * T1);            // Czas lotu na pełnej baterii (17-100s)
    int T_return = (int)(0.2 * T2);      // Czas potrzebny na powrót do bazy
    if (T_return < 1) T_return = 1;

    int drain = 100 / T2;                // Zużycie baterii na sekundę
    if (drain < 1) drain = 1;
    if (drain > 6) drain = 6;

    int bateria = 100; // Startujemy z pełną energią
    int ladowania = 0; // Licznik cykli ładowania
    int powrot_pozostalo = 0; // Odliczanie czasu powrotu

    stan_drona_t stan = LOT; // Początkowy stan drona
    stan_global = LOT;

    podlacz_pamiec(); // Podłączenie segmentu pamięci współdzielonej (shmat)

    // Uzyskanie dostępu do semaforów systemowych
    key_t key_sem = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'S');
    if (key_sem == -1)
    {
        perror("ftok sem dron");
        exit(1);
    }

    semafor = semget(key_sem, 1, 0); // Pobranie ID istniejącego zestawu semaforów
    if (semafor == -1)
    {
        blad("semget dron");
    }

    // Uzyskanie dostępu do kolejki komunikatów (mechanizm bramek bazy)
    key_t key_q = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'Q');
    if (key_q == -1) {
        perror("ftok msg dron");
        exit(1);
    }

    msg_id = msgget(key_q, 0); // Podłączenie do kolejki stworzonej przez Operatora
    if (msg_id == -1) {
        perror("msgget dron");
        exit(1);
    }

    struct stan *s = (struct stan *)adres; // Rzutowanie pamięci na strukturę

    int P, XI;
    semafor_p(); // Pobranie limitów systemowych (sekcja krytyczna)
    P = s->P;
    XI = s->XI;
    semafor_v();

    printf("[DRON %d] START | T1=%ds T2=%ds T_return=%ds drain=%d%%/s\n", getpid(), T1, T2, T_return, drain);

    sprintf(buf, "[DRON %d] START | T1=%ds T2=%ds T_return=%ds drain=%d%%/s\n", getpid(), T1, T2, T_return, drain);
    log_msg(buf);
    buf[0] = '\0';

    while (1) 
    {
        /* REAKCJA NA SYGNAŁ ATAKU */
        if (atak)
        {
            if (stan_global == LADOWANIE) // Dron trafiony podczas ładowania w bazie
            {
                semafor_p(); // Aktualizacja statystyk w pamięci dzielonej
                s->drony_w_bazie--;
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ZNISZCZONY W BAZIE (ATAK)\n", getpid());
                sprintf(buf, "[DRON %d] !!! ZNISZCZONY W BAZIE (ATAK)\n", getpid());
                log_msg(buf);
                exit(0); // Proces kończy życie
            }
            
            if (bateria >= 20) // Warunek wykonania ataku samobójczego
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ATAK SAMOBOJCZY\n", getpid());
                sprintf(buf, "[DRON %d] !!! ATAK SAMOBOJCZY\n", getpid());
                log_msg(buf);
                exit(0); // Proces ginie w chwale
            }
            else
            {
                printf("[DRON %d] ATAK ZIGNOROWANY (bateria < 20%%)\n", getpid());
                sprintf(buf, "[DRON %d] ATAK ZIGNOROWANY (bateria < 20%%)\n", getpid());
                log_msg(buf);
                atak = 0; // Reset flagi, dron ignoruje rozkaz przez niską energię
            }
        }

        /* LOGIKA STANU: LOT (PATROL) */
        if (stan == LOT) 
        {
            sleep(1); // Symulacja upływu sekundy lotu

            bateria -= drain; // Zmniejszenie energii
            if (bateria < 0) bateria = 0;

            printf("[DRON %d] LOT | bateria=%d%%\n", getpid(), bateria);
            sprintf(buf, "[DRON %d] LOT | bateria=%d%%\n", getpid(), bateria);
            log_msg(buf);

            if (bateria <= 20 && bateria > 0) // Krytyczny poziom baterii - powrót
            {
                stan = POWROT;
                stan_global = POWROT;
                powrot_pozostalo = T_return; // Ustawienie czasu dolotu do bazy

                printf("[DRON %d] >>> ROZPOCZYNAM POWROT (czas=%ds)\n", getpid(), powrot_pozostalo);
                sprintf(buf, "[DRON %d] >>> ROZPOCZYNAM POWROT (czas=%ds)\n", getpid(), powrot_pozostalo);
                log_msg(buf);
            }

            if (bateria <= 0) // Całkowite rozładowanie w powietrzu
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ZNISZCZONY W LOCIE (bateria=0%%)\n", getpid());
                sprintf(buf, "[DRON %d] !!! ZNISZCZONY W LOCIE (bateria=0%%)\n", getpid());
                log_msg(buf);
                exit(0); // Dron spada
            }
        }
        /* LOGIKA STANU: POWRÓT DO BAZY */
        else if (stan == POWROT) 
        {
            sleep(1);

            bateria -= drain; // Dron nadal zużywa energię wracając
            powrot_pozostalo--; // Zbliżanie się do bazy

            if (bateria < 0) bateria = 0;

            printf("[DRON %d] POWROT | bateria=%d%% | pozostalo=%ds\n", getpid(), bateria, powrot_pozostalo);
            sprintf(buf, "[DRON %d] POWROT | bateria=%d%% | pozostalo=%ds\n", getpid(), bateria, powrot_pozostalo);
            log_msg(buf);

            if (bateria <= 0) // Rozładowanie tuż przed bazą
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] !!! ZNISZCZONY W TRAKCIE POWROTU\n", getpid());
                sprintf(buf, "[DRON %d] !!! ZNISZCZONY W TRAKCIE POWROTU\n", getpid());
                log_msg(buf);
                exit(0);
            }

            if (powrot_pozostalo <= 0) // Dron dotarł pod bramę bazy
            {
                int mozna_wejsc = 0;

                semafor_p(); // Sprawdzenie ogólnej dostępności miejsc w bazy
                if (s->drony_w_bazie < s->P) 
                {
                    mozna_wejsc = 1;
                }
                semafor_v();

                if (mozna_wejsc) 
                {
                    /* MECHANIZM WĄSKIEGO GARDŁA (Kolejka komunikatów jako bramki) */
                    struct msg_wejscie m;
                    
                    // Próba pobrania żetonu (wejścia). IPC_NOWAIT sprawia, że dron nie stoi w miejscu
                    if (msgrcv(msg_id, &m, sizeof(int), 0, IPC_NOWAIT) == -1) 
                    {
                        if (errno == ENOMSG) {
                            // Obie bramki zajęte - dron musi krążyć kolejną sekundę
                            printf("[DRON %d] !!! WEJŚCIA ZAJĘTE - KRĄŻĘ (bateria: %d%%)\n", getpid(), bateria);
                            continue; 
                        } else {
                            perror("Błąd msgrcv");
                            continue;
                        }
                    }

                    long nr_wejscia = m.mtype; // Pobranie numeru bramki, którą dron wchodzi
                    printf("[DRON %d] >>> PRZECHODZE PRZEZ WEJSCIE %ld\n", getpid(), nr_wejscia);

                    sleep(1); // Czas fizycznego przelotu przez bramkę

                    semafor_p(); // LOCK - ostateczna aktualizacja stanu bazy
                    if (s->drony_w_bazie < s->P) 
                    {
                        s->drony_w_bazie++; // Oficjalne zajęcie miejsca w bazie
                        semafor_v(); // UNLOCK

                        // Zwolnienie bramki (żetonu) dla innych dronów
                        m.mtype = nr_wejscia;
                        m.dron_pid = getpid();
                        msgsnd(msg_id, &m, sizeof(int), 0);

                        printf("[DRON %d] >>> DOTARL DO BAZY (Wejscie %ld)\n", getpid(), nr_wejscia);
                        sprintf(buf, "[DRON %d] >>> DOTARL DO BAZY (Wejscie %ld)\n", getpid(), nr_wejscia);
                        log_msg(buf);

                        stan = LADOWANIE; // Zmiana stanu na ładowanie
                        stan_global = LADOWANIE;
                    }
                    else 
                    {
                        // Wyścig (Race Condition): inny dron zajął miejsce, gdy my lecieliśmy bramką
                        semafor_v();
                        
                        // Oddajemy żeton bramki i wracamy do krążenia
                        m.mtype = nr_wejscia;
                        msgsnd(msg_id, &m, sizeof(int), 0);
                        
                        printf("[DRON %d] !!! BAZA ZAPEŁNIONA W OSTATNIEJ CHWILI - KRĄŻĘ DALEJ\n", getpid());
                    }
                }
                else 
                {
                    // Brak miejsc w pamięci dzielonej - dron krąży i traci baterię
                    printf("[DRON %d] !!! BRAK MIEJSC W BAZIE - OCZEKIWANIE (Bateria: %d%%)\n", getpid(), bateria);
                    
                    sleep(1);
                    bateria -= drain; 
                    
                    if (bateria <= 0) {
                        semafor_p();
                        s->aktywne_drony--;
                        semafor_v();
                        printf("[DRON %d] !!! ROZBITTY POD BAZĄ (0%% Baterii)\n", getpid());
                        exit(0);
                    }
                }
            }
        }
        /* LOGIKA STANU: ŁADOWANIE ENERGII */
        else if (stan == LADOWANIE) 
        {
            printf("[DRON %d] <<< LADOWANIE (%ds)\n", getpid(), T1);
            sprintf(buf, "[DRON %d] <<< LADOWANIE (%ds)\n", getpid(), T1);
            log_msg(buf);

            // sleep() zwraca czas, który pozostał, jeśli został przerwany sygnałem
            unsigned int left = sleep(T1);
            
            if (left > 0 && atak) // Jeśli atak nastąpił w trakcie ładowania
            {
                continue; // Skok do początku pętli, gdzie obsłużymy flagę 'atak'
            }

            bateria = 100; // Akumulator pełny
            ladowania++; // Zwiększenie licznika zużycia drona

            semafor_p(); // Opuszczenie bazy
            s->drony_w_bazie--;
            semafor_v();

            printf("[DRON %d] <<< WYLOT Z BAZY | bateria=100%% | ladowania=%d\n", getpid(), ladowania);
            sprintf(buf, "[DRON %d] <<< WYLOT Z BAZY | bateria=100%% | ladowania=%d\n", getpid(), ladowania);
            log_msg(buf);

            if (ladowania >= XI) // Sprawdzenie limitu żywotności drona
            {
                semafor_p();
                s->aktywne_drony--;
                semafor_v();

                printf("[DRON %d] XXX UTYLIZACJA (Xi=%d)\n", getpid(), XI);
                sprintf(buf, "[DRON %d] XXX UTYLIZACJA (Xi=%d)\n", getpid(), XI);
                log_msg(buf);
                exit(0); // Dron wycofany z eksploatacji
            }

            stan = LOT; // Powrót do patrolowania
            stan_global = LOT;
        }
    }
}