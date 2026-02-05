#include "shared.h"
#include <sys/wait.h>

struct stan *s; // Wskaźnik na strukturę stanu w pamięci dzielonej

pid_t *drony = NULL;     // Dynamiczna tablica przechowująca PID-y aktywnych dronów
int liczba_dronow = 0;   // Aktualna liczba dronów w systemie
int pojemnosc_dronow = 0; // Rozmiar zaalokowanej tablicy drony

/* Funkcja obsługująca sygnał SIGWINCH - przekazanie rozkazu ataku do konkretnego drona */
void przekaz_atak(int sig)
{
    char buf[128]; // Lokalny bufor na tekst do pliku logów

    semafor_p(); 
    pid_t wybrany_dron = s->cel_ataku; // Pobranie PID celu zapisanego przez Dowódcę
    semafor_v(); 

    // Sprawdzenie, czy odczytany PID jest prawidłowy
    if (wybrany_dron > 0)
    {
        printf("[OPERATOR] Przekazuje rozkaz ataku do wskazanego drona PID=%d\n", wybrany_dron);
        sprintf(buf, "[OPERATOR] Przekazuje rozkaz ataku do wskazanego drona PID=%d\n", wybrany_dron);
        log_msg(buf); 
        buf[0] = '\0';
        // Przeszukiwanie listy aktywnych dronów w celu weryfikacji PID
        int znaleziono = 0; 
        for(int i=0; i<liczba_dronow; i++) 
        {
            if(drony[i] == wybrany_dron) 
            {
                znaleziono = 1; // Potwierdzenie obecności drona na liście operatora
                break;
            }
        }

        // Procedura ataku, jeśli dron o podanym PID istnieje
        if (znaleziono) 
        {
            // Formatowanie i zapisanie informacji o ataku do logów systemowych
            sprintf(buf, "[OPERATOR] ROZKAZ ATAKU DLA PID=%d\n", wybrany_dron);
            log_msg(buf); 
            buf[0] = '\0'; // Czyszczenie bufora pomocniczego

            // Wysłanie sygnału SIGUSR1 do drona (inicjacja ataku)
            kill(wybrany_dron, SIGUSR1);

            semafor_p(); 
            s->cel_ataku = 0; // Wyzerowanie celu w pamięci po skutecznym przekazaniu
            semafor_v(); 
        } 
        else 
        {
            // Obsługa błędu w przypadku podania nieistniejącego PID
            printf("[OPERATOR] BŁĄD: Dron %d nie istnieje lub nie jest aktywny!\n", wybrany_dron);
            
            // Rejestracja błędnej próby ataku w pliku system.log
            sprintf(buf, "[OPERATOR] BŁĄD: Proba ataku na nieaktywny PID=%d\n", wybrany_dron);
            log_msg(buf); 
            buf[0] = '\0'; // Czyszczenie bufora
        }
    }
}

/* Bezpieczne powiększanie tablicy PID-ów dronów */
void alokacja_dronow()
{
    if (liczba_dronow < pojemnosc_dronow) // Jeśli jest miejsce, nie rób nic
    {
        return;
    }

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
        exit(1);
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

    semafor_p(); // Blokada semafora na czas modyfikacji struktury
    s->max_drony = 2 * s->N; // Nowy limit: dwukrotność parametru startowego N
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
    int przed_redukcja;
    int nowy_limit;

    semafor_p(); // Sekcja krytyczna - musimy bezpiecznie zmodyfikować pamięć
    
    przed_redukcja = s->max_drony;
    
    // Obliczamy o ile redukujemy (połowa, ale minimum 1, jeśli cokolwiek zostało)
    int do_redukcji = przed_redukcja / 2;
    if (do_redukcji == 0 && przed_redukcja > 0) 
    {
        do_redukcji = 1;
    }

    s->max_drony -= do_redukcji;
    if (s->max_drony < 0) s->max_drony = 0;
    
    nowy_limit = s->max_drony; 
    semafor_v();

    // Logowanie i komunikacja o zmianie statusu platform
    printf("[OPERATOR] !!! REDUKCJA PLATFORM O %d (Nowy limit: %d)\n", do_redukcji, nowy_limit);
    sprintf(buf, "[OPERATOR] !!! REDUKCJA PLATFORM O %d (Nowy limit: %d)\n", do_redukcji, nowy_limit);
    log_msg(buf);
    buf[0] = '\0';

    /* PROCES USYPIANIA NADMIAROWYCH DRONÓW */
    // Blokujemy SIGCHLD, aby tablica drony[] nie zmieniała się w trakcie wysyłania sygnałów
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    // Zabijamy drony tak długo, aż ich liczba zrówna się z nowym limitem
    // Idziemy od końca tablicy (najmłodsze drony)
    while (liczba_dronow > nowy_limit)
    {
        pid_t pid_do_ubicia = drony[liczba_dronow - 1];
        
        // Wysyłamy SIGTERM - bezwarunkowy nakaz zakończenia pracy drona
        if (kill(pid_do_ubicia, SIGTERM) == 0) 
        {
            // Zmniejszamy lokalny licznik, aby kontynuować pętlę redukcji
            liczba_dronow--; 
        } 
        else 
        {
            // Jeśli dron już nie istnieje, pomijamy go i idziemy dalej
            liczba_dronow--;
        }
    }
    
    sigprocmask(SIG_SETMASK, &oldmask, NULL); // Przywracamy standardową obsługę SIGCHLD
}

/* Sprzątanie zasobów IPC i kończenie pracy Operatora */
void cleanup(int sig)
{
    char buf[128];

    sprintf(buf, "[OPERATOR] ZAKONCZENIE PROGRAMU (sygnal %d)\n", sig);
    log_msg(buf);
    buf[0] = '\0';
    // Pętla kończąca wszystkie aktywne procesy dronów przed zamknięciem systemu
    for (int i = 0; i < liczba_dronow; i++)
    {
        kill(drony[i], SIGTERM); 
    }

    free(drony); // Zwolnienie pamięci dynamicznej tablicy PID-ów
    drony = NULL;
    pojemnosc_dronow = 0;

    usun_semafor(); // Usunięcie zestawu semaforów z pamięci systemowej
    odlacz_pamiec(); // Odłączenie segmentu pamięci współdzielonej (shmdt)

    log_msg("[OPERATOR] USUNIETO SEMAFOR I PAMIEC DZIELONA\n");

    // Usunięcie kolejki komunikatów (bramki bazy) z systemu
    if (msg_id != -1 && msgctl(msg_id, IPC_RMID, NULL) == -1) 
    {
        perror("Błąd usuwania kolejki"); 
    }
    
    log_msg("[OPERATOR] USUNIETO KOLEJKE KOMUNIKATOW\n");

    log_close(); // Zamknięcie deskryptora pliku logowania
    exit(0); // Zakończenie procesu operatora
}

int main()
{
    char buf[128];
    buf[0] = '\0';

    srand(time(NULL)); // Inicjalizacja ziarna dla liczb losowych (oparta na czasie)
    log_init("system.log"); // Inicjalizacja zapisu do pliku system.log

    // Konfiguracja kolejki komunikatów (mechanizm bramek wejściowych bazy)
    key_t key_q = ftok("/home/inf1s-24z/przybycinski.patryk.155298/PROJEKT2/ipc.key", 'Q');
    msg_id = msgget(key_q, 0600 | IPC_CREAT); 

    // Inicjalizacja dwóch dostępnych bramek (żetonów) w kolejce
    struct msg_wejscie m1 = {1, 0}; 
    struct msg_wejscie m2 = {2, 0}; 

    msgsnd(msg_id, &m1, sizeof(int), 0); // Dodanie bramki nr 1 do kolejki
    msgsnd(msg_id, &m2, sizeof(int), 0); // Dodanie bramki nr 2 do kolejki

    // Konfiguracja automatycznego sprzątania procesów zombie (SIGCHLD)
    struct sigaction sa;
    sa.sa_handler = sprzatnij_drony;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    signal(SIGINT, cleanup); // Przechwycenie Ctrl+C w celu bezpiecznego zamknięcia IPC

    printf("[OPERATOR] START\n");
    log_msg("[OPERATOR] START\n");

    // Rejestracja sygnałów SIGUSR1/SIGUSR2/SIGWINCH wysyłanych przez Dowódcę
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

    // Inicjalizacja połączeń z segmentem IPC (Shared Memory & Semaphores)
    upd(); // Podłączenie segmentu pamięci
    upa(); // Uzyskanie adresu struktury
    utworz_nowy_semafor(); // Alokacja zestawu semaforów
    ustaw_semafor(); // Inicjalizacja wartości semaforów

    s = (struct stan *)adres; // Powiązanie wskaźnika s ze strukturą w pamięci dzielonej

    // Pobranie parametrów konfiguracyjnych zapisanych przez Dowódcę
    semafor_p(); 
    int N = s->N;
    int P = s->P;
    int TK = s->Tk;
    semafor_v();

    // Zapamiętanie PID procesu operatora (rodzica)
    pid_t parent_pid = getpid();

    // Startowa generacja floty dronów – tworzymy N procesów potomnych
    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork(); // Utworzenie nowego procesu

        if (pid == -1)
        {
            // Błąd tworzenia procesu
            perror("fork dron");
            exit(1);
        }

        if (pid == 0)
        {
            // KOD DZIECKA 
            // Zastąpienie procesu potomnego programem "dron"
            // Po udanym execl() kod poniżej się już nie wykona
            execl("./dron", "dron", NULL);

            // Jeśli execl wróci → wystąpił błąd
            perror("execl dron");
            exit(1);
        }

        // Zapewnienie miejsca w dynamicznej tablicy PID-ów dronów. 
        alokacja_dronow();

        // Zapis PID nowo utworzonego drona w tablicy
        drony[liczba_dronow++] = pid;

    }

    // Aktualizacja liczby aktywnych dronów w pamięci współdzielonej
    // Sekcja krytyczna zabezpieczona semaforem
    semafor_p();                  // LOCK
    s->aktywne_drony += N;        // Dodanie N nowych dronów do systemu
    semafor_v();                  // UNLOCK

    time_t nastepne_uzupelnienie = time(NULL) + TK; // Ustawienie harmonogramu uzupełniania floty
    time_t ostatnia_sekunda_status = 0; // Pomocniczy zegar do logowania statusu

    /* GŁÓWNA PĘTLA OPERATORA - TRYB AKTYWNEGO OCZEKIWANIA */
    while (1)
    {
        time_t teraz = time(NULL);

        // 1. Warunek zakończenia: sprawdzenie czy zostały jeszcze platformy lub drony
        if (teraz > ostatnia_sekunda_status) 
        {
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
                cleanup(0); // Rozpoczęcie procedury zwalniania zasobów IPC
            }
            ostatnia_sekunda_status = teraz;
        }

        // 2. Harmonogram uzupełniania floty co TK sekund
        if (teraz - nastepne_uzupelnienie >= TK)
        {
            semafor_p();
            int aktywne = s->aktywne_drony;
            int max = s->max_drony;
            int w_bazie = s->drony_w_bazie;
            int limit_P = s->P;

            // Wyświetlenie aktualnego stanu zasobów w konsoli i logach
            printf("[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", aktywne, w_bazie, max);
            sprintf(buf, "[OPERATOR] STATUS: aktywne=%d baza=%d max=%d\n", aktywne, w_bazie, max);
            log_msg(buf);
            buf[0] = '\0';
            
            // Sprawdzenie limitów: czy mamy wolne platformy i czy baza nie jest przepełniona
            if (aktywne < max && w_bazie < limit_P)
            {
                printf("[OPERATOR] >>> UZUPELNIANIE: %d -> %d\n", aktywne, aktywne + 1);
                sprintf(buf, "[OPERATOR] >>> UZUPELNIANIE: %d -> %d\n", aktywne, aktywne + 1);
                log_msg(buf);
                buf[0] = '\0';

                semafor_v(); // Zwolnienie semafora przed forkiem
                stworz_drona();
            }
            else
            {
                semafor_v(); // Zwolnienie jeśli nie tworzymy drona
            }

            nastepne_uzupelnienie = teraz; // Aktualizacja czasu następnej próby
        }

    }
}