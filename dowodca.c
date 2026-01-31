#include "shared.h"
#include <sys/wait.h>

/* PID procesu operatora – zapamiętany, aby móc wysyłać do niego sygnały */
pid_t operator_pid;

/*
 * Funkcja obsługi sygnału SIGCHLD.
 * Wywoływana automatycznie, gdy proces potomny (operator)
 * zakończy swoje działanie.
 */
void obsluga_zakonczenia_operatora(int sig)
{
    int status;

    // Sprawdzenie, czy któryś z procesów potomnych zakończył się (bez blokowania)
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0) 
    {
        // Informacja dla dowódcy, że operator zakończył działanie
        printf("\n[DOWODCA] Odebrano sygnał zakończenia od Operatora (PID: %d).\n", pid);
        printf("[DOWODCA] System został zamknięty poprawnie.\n");

        // Zakończenie procesu dowódcy po zamknięciu operatora
        exit(0);
    }
}

/*
 * Funkcja pomocnicza do bezpiecznego wczytywania liczby całkowitej.
 * Zapewnia:
 *  - poprawny format danych
 *  - kontrolę zakresu wartości
 */
int wczytaj_int(const char *opis, int min, int max)
{
    int x;
    while (1)
    {
        printf("%s (%d-%d): ", opis, min, max);
        fflush(stdout);

        // Sprawdzenie, czy użytkownik podał liczbę
        if (scanf("%d", &x) != 1)
        {
            printf("Niepoprawna wartość\n");
            while (getchar() != '\n'); // Czyszczenie bufora wejścia
            continue;
        }

        // Sprawdzenie, czy liczba mieści się w dozwolonym zakresie
        if (x < min || x > max)
        {
            printf("Wartość poza zakresem\n");
            continue;
        }

        return x; // Poprawna wartość
    }
}

int main()
{
    // Usunięcie poprzedniego pliku logu – każda symulacja startuje od zera
    remove("system.log");

    printf("[DOWODCA] KONFIGURACJA SYSTEMU\n");

    // Pobranie parametrów startowych systemu
    int N  = wczytaj_int("Podaj N (liczba dronów)", 1, 10000);   // Liczba dronów
    int P  = wczytaj_int("Podaj P (pojemność bazy)", 0, (N/2)); // Maks. dronów w bazie
    int Tk = wczytaj_int("Podaj Tk (czas uzupełniania)", 1, 60);// Czas tworzenia nowego drona
    int XI = wczytaj_int("Podaj Xi (liczba ładowań)", 1, 10);   // Limit ładowań drona

    // Inicjalizacja generatora losowego (np. do losowych zdarzeń)
    srand(time(NULL));

    // Inicjalizacja systemu logowania
    log_init("system.log");

    printf("[DOWODCA] START\n");
    log_msg("[DOWODCA] START\n");

    // Utworzenie i podłączenie pamięci dzielonej
    upd();
    upa();

    // Utworzenie i inicjalizacja semafora binarnego
    utworz_nowy_semafor();
    ustaw_semafor();

    // Wskaźnik na strukturę stanu systemu w pamięci dzielonej
    struct stan *s = (struct stan *)adres;

    // Sekcja krytyczna – zapis parametrów początkowych do pamięci dzielonej
    semafor_p();
    s->N = N;                 // Początkowa liczba dronów
    s->P = P;                 // Pojemność bazy
    s->Tk = Tk;               // Czas uzupełniania
    s->XI = XI;               // Limit ładowań

    s->max_drony = N;         // Maksymalna liczba dronów na starcie
    s->aktywne_drony = 0;     // Na początku brak aktywnych dronów
    s->drony_w_bazie = 0;     // Na początku baza pusta
    semafor_v();

    // Utworzenie procesu operatora
    operator_pid = fork();
    if (operator_pid == -1)
    {
        perror("fork");
        exit(1);
    }

    // Kod wykonywany tylko w procesie potomnym (operator)
    if (operator_pid == 0)
    {
        execl("./operator", "operator", NULL); // Uruchomienie programu operator
        perror("execl operator");
        exit(1);
    }

    // Rejestracja obsługi SIGCHLD – wykrycie zakończenia operatora
    struct sigaction sa_exit;
    sa_exit.sa_handler = obsluga_zakonczenia_operatora;
    sigemptyset(&sa_exit.sa_mask);
    sa_exit.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_exit, NULL) == -1) 
    {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    // Menu sterowania systemem przez dowódcę
    printf("\n=== SYSTEM STEROWANIA DRONAMI ===\n");
    printf("1 - [SIGUSR1] Rozbudowa platform (max drony x2)\n");
    printf("2 - [SIGUSR2] Redukcja platform (liczba dronow -50%%)\n");
    printf("3 - [SIGWINCH] Atak samobójczy (losowy dron)\n");
    printf("q - Zakoncz symulacje\n");
    printf("=================================\n");

    int wybor;
    while (1)
    {
        printf("\nDecyzja dowodcy > ");
        fflush(stdout);

        // Odczyt polecenia z klawiatury
        wybor = getchar();
        if (wybor == '\n') continue;
        while (getchar() != '\n'); // Czyszczenie bufora

        if (wybor == '1')
        {
            printf("[DOWODCA] Rozkaz: ROZBUDOWA\n");
            log_msg("[DOWODCA] Sygnal ROZBUDOWA (manual)\n");
            kill(operator_pid, SIGUSR1); // Sygnał do operatora: zwiększenie max_drony
        }
        else if (wybor == '2')
        {
            printf("[DOWODCA] Rozkaz: REDUKCJA\n");
            log_msg("[DOWODCA] Sygnal REDUKCJA (manual)\n");
            kill(operator_pid, SIGUSR2); // Sygnał do operatora: redukcja platform
        }
        else if (wybor == '3')
        {
            printf("[DOWODCA] Rozkaz: ATAK SAMOBOJCZY\n");
            log_msg("[DOWODCA] Sygnal ATAK (manual)\n");
            kill(operator_pid, SIGWINCH); // Sygnał do operatora: losowy atak drona
        }
        else if (wybor == 'q')
        {
            printf("[DOWODCA] Konczenie symulacji...\n");
            kill(operator_pid, SIGINT); // Sygnał do operatora: zakończenie programu
            break;
        }
    }
}
