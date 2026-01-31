#include "shared.h"
#include <sys/wait.h>

struct stan *s; // Wskaźnik na strukturę stanu w pamięci dzielonej

pid_t *drony = NULL;     // Dynamiczna tablica przechowująca PID-y aktywnych dronów
int liczba_dronow = 0;   // Aktualna liczba dronów w systemie
int pojemnosc_dronow = 0; // Rozmiar zaalokowanej tablicy drony

/* Funkcja przekazująca rozkaz ataku wybranemu losowo dronowi (obsługa SIGWINCH) */
void przekaz_atak(int sig)
{
    if (liczba_dronow > 0)
    {
        int index = rand() % liczba_dronow; // Losowanie drona z dostępnej listy
        pid_t wybrany_dron = drony[index];  // Pobranie PID-u wylosowanego drona
        
        printf("[OPERATOR] Przekazuje rozkaz ataku do drona PID=%d \n", wybrany_dron);
        log_msg("[OPERATOR] Przekazanie SIGTERM i redukcja max_drony\n");
        
        kill(wybrany_dron, SIGTERM); // Wysłanie sygnału do procesu drona
    }
}

/* Bezpieczne powiększanie tablicy PID-ów dronów */
void alokacja_dronow()
{
    if (liczba_dronow < pojemnosc_dronow) // Jeśli jest miejsce, nie rób nic
        return;

    int nowa = (pojemnosc_dronow == 0) ? 16 : pojemnosc_dronow * 2; // Strategia podwajania rozmiaru

    pid_t *tmp = realloc(drony, nowa * sizeof(pid_t)); // Reallokacja pamięci
    if (!tmp)
    {
        perror("realloc drony"); // Błąd krytyczny braku pamięci
        exit(1);
    }

    drony = tmp; // Przypisanie nowego adresu tablicy
    pojemnosc_dronow = nowa; // Aktualizacja pojemności
}

/* Obsługa sygnału SIGCHLD - sprzątanie po zakończonych procesach dronów */
void sprzatnij_drony(int sig)
{
    int status;
    pid_t pid;
    char buf[128];

    // waitpid z WNOHANG sprawdza status bez blokowania pracy Operatora
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < liczba_dronow; i++)
        {
            if (drony[i] == pid) // Znaleziono drona, który zakończył działanie
            {
                drony[i] = drony[liczba_dronow - 1]; // Przesunięcie ostatniego PID-u w lukę
                liczba_dronow--; // Zmniejszenie liczby śledzonych procesów

                printf("[OPERATOR] DRON PID=%d USUNIETY Z LISTY\n", pid);

                sprintf(buf, "[OPERATOR] DRON PID=%d USUNIETY Z LISTY\n", pid);
                log_msg(buf); // Zapisanie zdarzenia do pliku logu
                buf[0] = '\0';

                break;
            }
        }
    }
}

/* Funkcja tworząca nowy proces drona przy pomocy fork + execl */
void stworz_drona()
{
    char buf[128];

    pid_t pid = fork(); // Rozwidlenie procesu
    if (pid == -1)
    {
        perror("fork dron");
        return;
    }

    if (pid == 0) // Kod procesu potomnego
    {
        execl("./dron", "dron", NULL); // Uruchomienie programu dron
        perror("execl dron"); // Wykona się tylko w razie błędu execl
        exit(1);
    }

    alokacja_dronow(); // Przygotowanie miejsca w tablicy Operatora
    drony[liczba_dronow++] = pid; // Rejestracja PID-u nowego drona

    semafor_p(); // Sekcja krytyczna: dostęp do pamięci współdzielonej
    s->aktywne_drony++; // Zwiększenie licznika aktywnych dronów
    semafor_v(); // Koniec sekcji krytycznej

    printf("[OPERATOR] +++ NOWY DRON PID=%d\n", pid);

    sprintf(buf, "[OPERATOR] +++ NOWY DRON PID=%d\n", pid);
    log_msg(buf);
    buf[0] = '\0';
}

/* Obsługa sygnału SIGUSR1 - rozbudowa limitów systemu */
void sig_plus(int sig)
{
    char buf[128];

    semafor_p(); // Blokada semafora
    s->max_drony = 2 * s->N; // Nowy limit: dwukrotność N
    semafor_v(); // Zwolnienie semafora

    printf("[OPERATOR] !!! DODANO PLATFORMY (max=%d)\n", s->max_drony);

    sprintf(buf, "[OPERATOR] !!! DODANO PLATFORMY (max=%d)\n", s->max_drony);
    log_msg(buf);
    buf[0] = '\0';
}

/* Obsługa sygnału SIGUSR2 - redukcja platform i usuwanie dronów */
void sig_minus(int sig)
{
    char buf[128];
    int stare_max;

    semafor_p(); // Pobranie aktualnego limitu z pamięci
    stare_max = s->max_drony;
    semafor_v();

    if (liczba_dronow <= 0)
    {
        printf("[OPERATOR] !!! BRAK DRONOW DO USUNIECIA\n");
    }

    int do_usuniecia = stare_max / 2; // Obliczenie połowy limitu
    if (do_usuniecia == 0 && stare_max > 0)
    {
        do_usuniecia = 1; // Zawsze usuwamy minimum jednego, jeśli limit > 0
    }

    printf("[OPERATOR] !!! USUWANIE PLATFORM (-%d DRONOW)\n", do_usuniecia);

    sprintf(buf, "[OPERATOR] !!! USUWANIE PLATFORM (-%d DRONOW)\n", do_usuniecia);
    log_msg(buf);
    buf[0] = '\0';

    int wyslane = 0;
    // Pętla wysyłająca SIGTERM do wymaganej liczby dronów
    for (int i = 0; i < liczba_dronow && wyslane < do_usuniecia; i++)
    {
        pid_t pid = drony[i];

        if (kill(pid, SIGTERM) == -1) // Próba zakończenia procesu drona
        {
            if (errno != ESRCH)
                perror("kill SIGTERM");
            continue;
        }

        printf("[OPERATOR] --- DRON PID=%d USUNIETY (SIGTERM)\n", pid);
        sprintf(buf, "[OPERATOR] --- DRON PID=%d USUNIETY (SIGTERM)\n", pid);
        log_msg(buf);
        buf[0] = '\0';

        wyslane++;
    }

    semafor_p(); // Aktualizacja limitu max_drony w pamięci współdzielonej
    s->max_drony -= do_usuniecia;
    if (s->max_drony < 0)
        s->max_drony = 0;
    semafor_v();

    if (s->max_drony == 0)
    {
        printf("[OPERATOR] !!! LICZBA PLATFORM OSIAGNELA ZERO\n");
    }
}

/* Sprzątanie zasobów IPC i kończenie pracy Operatora */
void cleanup(int sig)
{
    char buf[128];

    sprintf(buf, "[OPERATOR] ZAKONCZENIE PROGRAMU (sygnal %d)\n", sig);
    log_msg(buf);

    for (int i = 0; i < liczba_dronow; i++)
    {
        kill(drony[i], SIGTERM); // Wysłanie sygnału do wszystkich dzieci
    }

    free(drony); // Zwolnienie tablicy PID-ów
    drony = NULL;
    pojemnosc_dronow = 0;

    usun_semafor(); // Usunięcie semafora z systemu (IPC)
    odlacz_pamiec(); // Odłączenie pamięci współdzielonej (shmdt)

    log_msg("[OPERATOR] USUNIETO SEMAFOR I PAMIEC DZIELONA\n");

    if (msgctl(msg_id, IPC_RMID, NULL) == -1) // Całkowite usunięcie kolejki komunikatów
    {
        perror("Błąd usuwania kolejki"); 
    }
    
    log_msg("[OPERATOR] USUNIETO KOLEJKE KOMUNIKATOW\n");

    log_close(); // Zamknięcie pliku logowania
    exit(0); // Wyjście z procesu
}

int main()
{
    char buf[128];
    buf[0] = '\0';

    srand(time(NULL)); // Inicjalizacja generatora liczb losowych
    log_init("system.log"); // Start logowania

    // Inicjalizacja kolejki komunikatów dla bramek bazy
    key_t key_q = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'Q');
    msg_id = msgget(key_q, 0600 | IPC_CREAT); 

    // Tworzenie "żetonów" wejścia (typ 1 i typ 2) do kolejki
    struct msg_wejscie m1 = {1, 0}; 
    struct msg_wejscie m2 = {2, 0}; 

    msgsnd(msg_id, &m1, sizeof(int), 0); // Wstawienie żetonu nr 1
    msgsnd(msg_id, &m2, sizeof(int), 0); // Wstawienie żetonu nr 2

    // Ustawienie obsługi SIGCHLD dla automatycznego sprzątania dronów
    struct sigaction sa;
    sa.sa_handler = sprzatnij_drony;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    signal(SIGINT, cleanup); // Rejestracja sprzątania na sygnał przerwania

    printf("[OPERATOR] START\n");
    log_msg("[OPERATOR] START\n");

    // Rejestracja sygnałów sterujących wysyłanych przez Dowódcę
    struct sigaction sa_p;
    sa_p.sa_handler = sig_plus;
    sigemptyset(&sa_p.sa_mask);
    sa_p.sa_flags = 0; 
    sigaction(SIGUSR1, &sa_p, NULL);

    struct sigaction sa_m;
    sa_m.sa_handler = sig_minus;
    sigemptyset(&sa_m.sa_mask);
    sa_m.sa_flags = 0; 
    sigaction(SIGUSR2, &sa_m, NULL);

    struct sigaction sa_a;
    sa_a.sa_handler = przekaz_atak;
    sigemptyset(&sa_a.sa_mask);
    sa_a.sa_flags = 0;
    sigaction(SIGWINCH, &sa_a, NULL);

    // Inicjalizacja mechanizmów pamięci dzielonej i semaforów
    upd(); // shmat
    upa(); // adres
    utworz_nowy_semafor(); // semget
    ustaw_semafor(); // semctl

    s = (struct stan *)adres; // Rzutowanie adresu pamięci na strukturę stan

    semafor_p(); // Pobranie parametrów z pamięci współdzielonej
    int N = s->N;
    int P = s->P;
    int TK = s->Tk;
    semafor_v();

    for (int i = 0; i < N; i++) // Pętla tworząca początkową flotę N dronów
    {
        stworz_drona();
    }

    time_t nastepne_uzupelnienie = time(NULL) + TK; // Obliczenie czasu następnego spawnu

    while (1)
    {
        // 1. Sprawdzanie warunku zakończenia systemu
        semafor_p();
        int aktualne_max = s->max_drony;
        int aktualne_aktywne = s->aktywne_drony;
        semafor_v();

        if (aktualne_max <= 0 && aktualne_aktywne <= 0)
        {
            printf("\n[OPERATOR] ########################################\n");
            printf("[OPERATOR] !!! BRAK DOSTEPNYCH PLATFORM I DRONOW !!!\n");
            printf("[OPERATOR] !!! ZAMYKANIE SYSTEMU PRZEZ OPERATORA !!!\n");
            printf("[OPERATOR] ########################################\n\n");
            log_msg("[OPERATOR] Koniec symulacji - max_drony = 0\n");
            cleanup(0); // Zamknięcie całego systemu
        }

        // 2. Blok uzupełniania floty co TK sekund
        time_t teraz = time(NULL);
        if (teraz - nastepne_uzupelnienie >= TK)
        {
            semafor_p(); // Sprawdzenie czy można dodać drona (limity platform i bazy)
            int aktywne = s->aktywne_drony;
            int max = s->max_drony;
            int w_bazie = s->drony_w_bazie;
            int limit_P = s->P;
            semafor_v();

            if (aktywne < max && w_bazie < limit_P)
            {
                printf("[OPERATOR] >>> UZUPELNIANIE: %d -> %d\n", aktywne, aktywne + 1);
                sprintf(buf, "[OPERATOR] >>> UZUPELNIANIE: %d -> %d\n", aktywne, aktywne + 1);
                log_msg(buf);
                stworz_drona(); // Dodanie nowej jednostki
            }

            nastepne_uzupelnienie = teraz; // Reset licznika czasu

            semafor_p(); // Wyświetlenie i zalogowanie statusu systemu
            printf("[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", s->aktywne_drony, s->drony_w_bazie, s->max_drony);
            sprintf(buf, "[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", s->aktywne_drony, s->drony_w_bazie, s->max_drony);
            log_msg(buf);
            semafor_v();
        }

        sleep(1); // Oczekiwanie 1 sekundy (zmniejszenie obciążenia procesora)
    }
}