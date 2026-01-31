## Rój dronów

### 1. Ogólny opis projektu.

Projekt polega na symulacji cyklu życia **roju autonomicznych dronów**, działających w oparciu o mechanizmy systemów operacyjnych w środowisku Linux/UNIX. System składa się z trzech głównych typów procesów: **dowódcy**, **operatora** oraz **dronów**, które współpracują ze sobą i komunikują się przy użyciu procesów, wątków, sygnałów oraz mechanizmów IPC.

Każdy dron funkcjonuje jako niezależna jednostka wykonawcza, realizując cykl:
**lot → powrót do bazy → ładowanie → ponowny start**.  
Liczba dronów jest dynamiczna i zależna od decyzji dowódcy oraz dostępnej pojemności bazy. Ograniczenia systemowe obejmują m.in. maksymalną liczbę dronów w bazie, jednokierunkowy ruch w wąskich wejściach do bazy, czas lotu zależny od poziomu naładowania baterii oraz ryzyko zniszczenia drona przy jej całkowitym rozładowaniu.

Dowódca systemu zarządza globalnymi parametrami roju poprzez wysyłanie sygnałów do operatora i pojedynczych dronów (np. rozbudowa platform startowych, redukcja floty, atak samobójczy). Operator odpowiada za uzupełnianie braków w liczbie dronów, o ile pozwala na to aktualna pojemność bazy.

Cała symulacja generuje **raport tekstowy**, zapisywany do plików, dokumentujący przebieg działania systemu, zdarzenia oraz aktualne stany dronów.

Projekt ma na celu praktyczne wykorzystanie i zaprezentowanie **kluczowych funkcji systemowych** związanych z zarządzaniem procesami, wątkami, synchronizacją, komunikacją międzyprocesową, obsługą sygnałów oraz operacjami na plikach.

