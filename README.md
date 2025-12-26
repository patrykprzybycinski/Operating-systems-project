Rój autonomicznych dronów liczy początkowo N egzemplarzy. Drony startują (i lądują) z ukrytej platformy (bazy), na której w danym momencie może znajdować się maksymalnie P dronów (P < N/2).

Dron, który chce wrócić do bazy, musi wlecieć przez jedno z dwóch istniejących wejść. Wejścia te są bardzo wąskie, więc możliwy jest w nich ruch tylko w jedną stronę w danej chwili.

Zbyt długie przebywanie w bazie (ładowanie baterii) grozi jej przegrzaniem, dlatego każdy dron opuszcza bazę po pewnym skończonym czasie T1i.

Jedno pełne ładowanie wystarcza na lot, który maksymalnie może trwać T2i (T2i = 2.5 * T1i). Przy poziomie naładowania baterii 20% dron automatycznie rozpoczyna powrót do bazy. Jeżeli w trakcie lotu poziom naładowania baterii osiągnie 0%, dron ulega zniszczeniu.

Znajdujący się w bazie operator co pewien czas Tk stara się uzupełnić braki w liczbie dronów, pod warunkiem że w bazie jest wystarczająca ilość miejsca.

Dowódca systemu może:

Wysłać sygnał1 do operatora w celu dodania dodatkowych platform startowych, co pozwala zwiększyć maksymalną liczbę dronów do 2 * N egzemplarzy.
Wysłać sygnał2 do operatora w celu zdemontowania platform startowych, ograniczając bieżącą maksymalną liczbę egzemplarzy o 50%.
Wysłać sygnał3 do konkretnego drona (nawet tego w bazie podczas ładowania baterii) w celu wykonania ataku samobójczego. Jeżeli poziom naładowania baterii jest niższy niż 20%, dron ignoruje sygnał3.
Każdy dron jest utylizowany (wycofany z eksploatacji) po określonym czasie Xi, liczonym ilością ładowań (pobytów w bazie).

Raport z przebiegu symulacji należy zapisać w plikach tekstowych.

Testy
W projekcie planuję przeprowadzić następujące testy symulacji roju dronów:

Test startu i powrotu dronów do bazy

Sprawdzenie, czy drony startują z bazy i wracają po czasie T1i.
Weryfikacja, że drony automatycznie wracają przy poziomie baterii ≤ 20%.

Test ograniczeń bazy

Symulacja sytuacji, gdy baza jest pełna (maksymalnie P dronów).
Sprawdzenie, że drony czekają na wolne miejsce, zanim wleca do bazy.

Test utylizacji dronów

Sprawdzenie, czy drony są wycofywane z eksploatacji po osiągnięciu liczby ładowań Xi.
