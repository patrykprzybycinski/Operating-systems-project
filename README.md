### Rój dronów

## 1. Ogólny opis projektu

Projekt polega na symulacji cyklu życia **roju autonomicznych dronów**, działających w oparciu o mechanizmy systemów operacyjnych w środowisku Linux/UNIX. System składa się z trzech głównych typów procesów: **dowódcy**, **operatora** oraz **dronów**, które współpracują ze sobą i komunikują się przy użyciu procesów, wątków, sygnałów oraz mechanizmów IPC.

Każdy dron funkcjonuje jako niezależna jednostka wykonawcza, realizując cykl:
**lot → powrót do bazy → ładowanie → ponowny start**.  
Liczba dronów jest dynamiczna i zależna od decyzji dowódcy oraz dostępnej pojemności bazy. Ograniczenia systemowe obejmują m.in. maksymalną liczbę dronów w bazie, jednokierunkowy ruch w wąskich wejściach do bazy, czas lotu zależny od poziomu naładowania baterii oraz ryzyko zniszczenia drona przy jej całkowitym rozładowaniu.

Dowódca systemu zarządza globalnymi parametrami roju poprzez wysyłanie sygnałów do operatora i pojedynczych dronów (np. rozbudowa platform startowych, redukcja floty, atak samobójczy). Operator odpowiada za uzupełnianie braków w liczbie dronów, o ile pozwala na to aktualna pojemność bazy.

Cała symulacja generuje **raport tekstowy**, zapisywany do plików, dokumentujący przebieg działania systemu, zdarzenia oraz aktualne stany dronów.

Projekt ma na celu praktyczne wykorzystanie i zaprezentowanie **kluczowych funkcji systemowych** związanych z zarządzaniem procesami, wątkami, synchronizacją, komunikacją międzyprocesową, obsługą sygnałów oraz operacjami na plikach.

## 2. Ogólny opis kodu

Projekt został podzielony na **kilka logicznie rozdzielonych plików źródłowych**, z których każdy odpowiada za odrębny element symulowanego systemu. Taki podział ułatwia rozwój projektu, zwiększa czytelność kodu oraz pozwala na łatwiejsze testowanie i debugowanie poszczególnych komponentów.

## 2.1. Struktura projektu

- **`dowodca.c`**  
  Główny proces sterujący symulacją. Odpowiada za inicjalizację parametrów systemu (liczba dronów, pojemność bazy, czasy), tworzenie procesu operatora oraz obsługę interfejsu decyzyjnego dowódcy. Realizuje wysyłanie sygnałów sterujących do operatora oraz nadzoruje poprawne zakończenie systemu.

- **`operator.c`**  
  Proces pośredniczący pomiędzy dowódcą a dronami. Odpowiada za tworzenie i usuwanie dronów, reagowanie na sygnały dowódcy (rozbudowa, redukcja, atak), okresowe uzupełnianie liczby dronów oraz sprzątanie zakończonych procesów potomnych. Zarządza również zasobami współdzielonymi systemu.

- **`dron.c`**  
  Implementacja pojedynczego drona jako autonomicznego procesu. Dron realizuje własny cykl życia (lot, powrót, ładowanie), zarządza poziomem baterii oraz reaguje na sygnały systemowe (atak samobójczy). Logika drona uwzględnia ograniczenia bazy, dostępność wejść oraz warunki awaryjne.

- **`semafory.c`**  
  Moduł odpowiedzialny za synchronizację dostępu do pamięci współdzielonej przy użyciu semaforów System V. Zapewnia ochronę sekcji krytycznych i spójność danych globalnych.

- **`pamiec.c`**  
  Obsługa segmentów pamięci dzielonej. Plik odpowiada za tworzenie, podłączanie, odłączanie oraz usuwanie pamięci współdzielonej używanej do przechowywania aktualnego stanu systemu.

- **`kolejka.c`**  
  Implementacja kolejek komunikatów System V wykorzystywanych do synchronizacji dostępu do wejść bazy. Kolejka pełni rolę mechanizmu kontroli jednokierunkowego ruchu dronów przez dwa wąskie wejścia.

- **`log.c`**  
  Moduł odpowiedzialny za rejestrowanie przebiegu symulacji. Zapewnia spójny, czasowo oznaczony zapis zdarzeń do pliku logu.

- **`bledy.c`**  
  Prosty moduł pomocniczy do obsługi błędów krytycznych i kończenia programu w przypadku niepowodzeń operacji systemowych.

- **`shared.h`**  
  Wspólny plik nagłówkowy zawierający deklaracje struktur, zmiennych globalnych oraz prototypy funkcji używanych w całym projekcie.

## 2.2. Zastosowane rozwiązania zwiększające wydajność i stabilność

- **Asynchroniczna komunikacja za pomocą sygnałów**  
  Sygnały systemowe pozwalają na natychmiastową reakcję procesów bez konieczności ciągłego odpytywania stanu.

- **Minimalizacja sekcji krytycznych**  
  Dostęp do pamięci współdzielonej jest chroniony semaforami tylko w niezbędnych fragmentach kodu, co ogranicza czas blokowania i poprawia równoległość działania procesów.

- **Dynamiczne zarządzanie zasobami**  
  Lista aktywnych dronów jest alokowana dynamicznie i rozszerzana w razie potrzeby, co pozwala na efektywne wykorzystanie pamięci.

- **Obsługa sygnałów SIGCHLD**  
  Zapewnia bieżące usuwanie zakończonych procesów dronów, zapobiegając powstawaniu procesów zombie.

---
## 3. Zrealizowane funkcjonalności

W ramach projektu udało się zaimplementować pełną symulację cyklu życia roju autonomicznych dronów,
zgodnie z założeniami projektowymi. Zrealizowane zostały następujące elementy:

- utworzenie trzech współpracujących typów procesów: dowódcy, operatora oraz dronów,
- dynamiczne tworzenie i usuwanie dronów z wykorzystaniem procesów potomnych,
- komunikacja międzyprocesowa przy użyciu sygnałów systemowych,
- synchronizacja dostępu do zasobów współdzielonych za pomocą semaforów System V,
- współdzielenie globalnego stanu systemu przy użyciu pamięci dzielonej,
- kontrola dostępu do dwóch wąskich wejść bazy przy użyciu kolejek komunikatów,
- realistyczna symulacja poziomu baterii, czasu lotu, powrotu oraz ładowania dronów,
- obsługa sytuacji awaryjnych (zniszczenie drona w locie, atak samobójczy, brak miejsc w bazie),
- poprawne i uporządkowane zakończenie systemu wraz ze zwolnieniem zasobów IPC,
- generowanie szczegółowego raportu z przebiegu symulacji w plikach tekstowych.

---

## 4. Napotkane problemy i trudności

Podczas realizacji projektu napotkano kilka istotnych problemów technicznych, głównie związanych
z równoległością oraz komunikacją międzyprocesową:

- **Synchronizacja dostępu do pamięci współdzielonej**  
  W początkowej wersji projektu występowały warunki wyścigu podczas jednoczesnej modyfikacji liczników
  dronów. Problem został rozwiązany poprzez zawężenie sekcji krytycznych i konsekwentne użycie semaforów.

- **Obsługa sygnałów i procesów zombie**  
  Występowały sytuacje, w których zakończone procesy dronów nie były poprawnie usuwane.
  Zastosowanie obsługi sygnału `SIGCHLD` oraz `waitpid()` rozwiązało ten problem.

- **Asynchroniczność sygnałów**  
  Sygnały mogły być dostarczane w dowolnym momencie wykonywania kodu, co wymagało zastosowania
  zmiennych typu `volatile sig_atomic_t` oraz dodatkowych sprawdzeń stanu drona.

- **Koordynacja dostępu do wejść bazy**  
  Trudność stanowiło zapewnienie jednokierunkowego ruchu w dwóch wąskich wejściach bazy.
  Problem rozwiązano poprzez wykorzystanie kolejek komunikatów jako mechanizmu przydziału wejść.

- **Testowanie scenariuszy brzegowych**  
  Równoczesne zakończenie wielu dronów, redukcja platform do zera lub atak na drona w trakcie ładowania
  ujawniały błędy logiczne, które wymagały dodatkowych zabezpieczeń i sprawdzeń warunków.  
  Problemy te zostały rozwiązane poprzez dodanie jawnych kontroli stanu systemu, zabezpieczenie
  operacji na licznikach semaforami oraz wprowadzenie dodatkowych warunków kończących symulację
  w sposób uporządkowany (zamykanie systemu przez operatora po wyczerpaniu dostępnych zasobów).

## 5. Wyróżniające się elementy specjalne

- **Obsługa wielu scenariuszy awaryjnych**  
  Projekt uwzględnia rzadkie i trudne przypadki, takie jak atak na drona w trakcie ładowania,
  rozładowanie baterii podczas oczekiwania na wejście do bazy czy redukcja liczby platform
  w trakcie aktywnej pracy systemu.

- **Rozbudowane logowanie zdarzeń**  
  Log zawiera szczegółowe informacje o stanie każdego drona (lot, powrót, ładowanie,
  zniszczenie, utylizacja), co umożliwia późniejszą analizę przebiegu symulacji oraz
  diagnozowanie problemów współbieżności.

- **Dynamiczny i niedeterministyczny charakter symulacji**  
  Losowe czasy lotu, ładowania oraz zużycia baterii powodują, że każda symulacja przebiega
  inaczej. Utrudnia to testowanie, ale lepiej odwzorowuje zachowanie rzeczywistych systemów
  autonomicznych.
  
## 6. Przeprowadzone testy

W celu weryfikacji poprawności działania systemu, spełnienia założeń projektowych
oraz sprawdzenia zachowania symulacji w różnych scenariuszach, przeprowadzono
następujące testy funkcjonalne i scenariusze użytkowe:

1. Skonfigurowanie roju dronów (parametry N, P, Tk, Xi) oraz poprawne uruchomienie programu  
2. Test wysłania sygnału 3 – atak samobójczy losowego drona  
3. Test wysłania sygnału 2 – redukcja platform startowych w trakcie działania systemu  
4. Test wysłania sygnału 1 – rozbudowa platform startowych w trakcie działania systemu  
5. Test zakończenia programu    
6. Test utylizacji dronów po osiągnięciu określonej liczby ładowań (Xi)  
7. Test powrotu dronów do bazy i przejścia przez jedno z dwóch istniejących wejść  

### Test 1 – Konfiguracja roju dronów i uruchomienie systemu

Test polegał na uruchomieniu programu dowódcy oraz skonfigurowaniu początkowych parametrów
symulacji: liczby dronów (N), pojemności bazy (P), czasu uzupełniania roju (Tk) oraz maksymalnej
liczby ładowań pojedynczego drona (Xi).

Po poprawnym wprowadzeniu danych system został uruchomiony, a proces operatora rozpoczął
tworzenie dronów zgodnie z zadanymi parametrami. Każdy dron wystartował jako osobny proces,
inicjalizując losowe czasy lotu i ładowania oraz rozpoczynając swój cykl życia.

Test potwierdza:
- poprawną walidację danych wejściowych,
- prawidłową inicjalizację pamięci dzielonej i semaforów,
- poprawne utworzenie procesów operatora i dronów,

![Test 1](test1.png)

### Test 2 – Wysłanie sygnału 3 (atak samobójczy losowego drona)

Test polegał na wysłaniu sygnału ataku samobójczego do systemu w trakcie trwania symulacji.
Dowódca wysyłał sygnał sterujący, który był przekazywany przez operatora do losowo wybranego
drona.

W trakcie testu zweryfikowano poprawność reakcji drona na sygnał w zależności od jego
aktualnego stanu oraz poziomu naładowania baterii. Dron z poziomem baterii powyżej 20%
realizował atak i kończył działanie, natomiast dron z poziomem baterii poniżej 20%
ignorował sygnał zgodnie z założeniami projektu.

Test potwierdza:
- poprawne przekazywanie sygnałów pomiędzy procesami,
- losowy wybór drona do wykonania ataku,
- zgodną z założeniami reakcję drona na sygnał ataku,
- poprawną aktualizację stanu systemu po zakończeniu procesu drona.

Dla drona z poziomem baterii > 20%:

![Test 2](test2.1.png)

Dla drona z poziomem baterii < 20%:

![Test 2](test2.2.png)

### Test 3 – Wysłanie sygnału 2 (redukcja platform startowych)

Test polegał na wysłaniu sygnału redukcji platform startowych w trakcie działania systemu.
Po odebraniu sygnału operator zmniejszał maksymalną liczbę dostępnych platform startowych,
co skutkowało ograniczeniem dopuszczalnej liczby aktywnych dronów w systemie.

W ramach testu zweryfikowano zachowanie systemu przy dynamicznym zmniejszaniu dostępnych
zasobów, w tym poprawne wysyłanie sygnałów zakończenia do nadmiarowych dronów oraz
aktualizację wartości przechowywanych w pamięci dzielonej.

Test potwierdza:
- poprawną obsługę sygnału redukcji platform przez operatora,
- dynamiczne dostosowanie liczby aktywnych dronów do nowego limitu,
- bezpieczne usuwanie procesów dronów w trakcie działania symulacji,

![Test 3](test3.png)

### Test 4 – Wysłanie sygnału 1 (rozbudowa platform startowych)

Test polegał na wysłaniu sygnału rozbudowy platform startowych w trakcie trwania symulacji.
Po odebraniu sygnału operator zwiększał maksymalną liczbę dostępnych platform, co umożliwiało
dalsze tworzenie i uruchamianie nowych dronów.

W trakcie testu zweryfikowano poprawność dynamicznej zmiany limitów systemowych bez
zatrzymywania symulacji oraz reakcję operatora na zwiększoną dostępność zasobów.

Test potwierdza:
- poprawną obsługę sygnału rozbudowy platform przez operatora,
- dynamiczne zwiększenie maksymalnej liczby dronów w systemie,
- możliwość dalszego uzupełniania roju w trakcie działania symulacji.

![Test 4](test4.png)

### Test 5 – Zakończenie programu i sprzątanie zasobów systemowych

Test polegał na zakończeniu działania symulacji poprzez wysłanie sygnału zakończenia z poziomu
procesu dowódcy. Po odebraniu sygnału operator inicjował procedurę kontrolowanego zamykania
systemu.

W ramach testu zweryfikowano poprawne zakończenie wszystkich aktywnych procesów dronów
oraz zwolnienie wykorzystywanych zasobów systemowych, takich jak pamięć dzielona,
semafory oraz kolejki komunikatów.

Test potwierdza:
- uporządkowane zakończenie działania wszystkich procesów,
- brak pozostawionych procesów zombie,
- poprawne usunięcie struktur IPC,
- bezpieczne zamknięcie symulacji bez wycieków zasobów.

![Test 5](tes5.png)

### Test 6 – Utylizacja dronów po osiągnięciu określonej liczby ładowań (Xi)

Test polegał na obserwacji cyklu życia pojedynczych dronów w trakcie działania symulacji,
ze szczególnym uwzględnieniem liczby pobytów drona w bazie. Każdy dron zliczał liczbę
zakończonych ładowań, a po osiągnięciu wartości granicznej Xi kończył swoje działanie.

W trakcie testu zweryfikowano poprawne przechodzenie dronów przez kolejne cykle:
lot → powrót → ładowanie → ponowny start, aż do momentu utylizacji. Po osiągnięciu limitu
Xi dron był wycofywany z eksploatacji, a jego proces kończył działanie w sposób kontrolowany.

Test potwierdza:
- poprawne zliczanie liczby ładowań przez drona,
- automatyczną utylizację drona po osiągnięciu limitu Xi,
- aktualizację liczników aktywnych dronów w pamięci dzielonej,
- stabilność systemu przy stopniowym wycofywaniu dronów z eksploatacji.

![Test 6](test6.png)

### Test 7 – Powrót dronów do bazy i przejście przez jedno z dwóch istniejących wejść

Test polegał na jednoczesnym powrocie wielu dronów do bazy w momencie obniżenia poziomu
naładowania baterii. W trakcie testu obserwowano sposób przydzielania dronom dostępu
do dwóch wąskich wejść prowadzących do bazy.

Zweryfikowano poprawne działanie mechanizmu synchronizacji, który zapewniał, że w danej
chwili jedno wejście może być wykorzystywane tylko przez jednego drona. Drony, które nie
uzyskały dostępu do wejścia, przechodziły w stan oczekiwania lub krążyły w pobliżu bazy
do momentu zwolnienia wejścia.

Test potwierdza:
- poprawną synchronizację dostępu do wejść bazy,
- brak kolizji podczas jednoczesnych powrotów wielu dronów,
- równomierne wykorzystanie obu dostępnych wejść.

![Test 7](test7.png)



