# OpenCVFrameGrabber

## Główne tryby pracy: 
C - kamera, przechwytywanie obrazu z kamery \
V - video, przechwytywanie obrazu z pliku video \
I - image, przechwytywanie obrazu z pliku graficznego 

## Inne tryby: 
PI - path input, wprowadzanie ścieżki \
L - logo, tryb nakładania logo

## Inne używane w README skróty: 
OT - obecny tryb

## Opcje menu:

| Typ opcji | Opcja | Dostępne w trybie | Opis | Skrót | Szczególny tryb skrótu |
| ---  | ---  | --- | --- | --- | --- |
| CHECKBOX | Put logo on | C, V, I | Nakładanie logo na klatkę OT |
| CHECKBOX | Record straight to video file | C, V | Zapis strumienia OT do pliku video | S - start, E - end | C |
| TRACKBAR | Set FPS | C, V | Ustawienie pożądanej ilości FPS dla pliku video | 
| Save video to frames: | - | - | - | - | - |
| CHECKBOX | On demand (must be running) | C, V | Zapis strumień do klatek (od razu) | S - start, E - end | V |
| CHECKBOX | Given frames | V | Pozwala wprowadzić dane dot. przechwytywania ilości klatek |
| FIELD | (Given frames) Start point | V | Pozwala na podanie pkt. startowego |
| FIELD | (Given frames) Quantity | V | Pozwala na podanie ilości klatek |
| BUTTON | GO! | V | Zapisz strumień do klatek (na podst. danych) |
| CHECKBOX | Given time | V |  Pozwala wprowadzić dane dot. przechwytywania wycinka filmu |
| FIELD | (Given time) Start point | V |  Pozwala na podanie pkt. startowego |
| FIELD | (Given time) Quantity | V | Pozwala na podanie czasu |
| BUTTON | GO! | V | Zapisz strumień do klatek (na podst. danych) |
| - | - | - | - | - | - |
| BUTTON | Save current to file | C, V, I | Zapisz obecną klatkę / obraz do pliku |
| CHECKBOX | Show image | C, V | Pokaż obraz wczytywany z podanej ścieżki |
| CHECKBOX | Join frames to video | jeśli istnieje ścieżka | Złącz klatki z podanego pliku do filmu |
| CHECKBOX | Enable PI | C, V, I | Włączenie możliwości wprowadzania ścieżek | CTRL + P | 
| CHECKBOX | Enable video PI | PI | | | CTRL + 1 |
| CHECKBOX | Enable frame PI | PI| | | CTRL + 2 |
| CHECKBOX | Enable image PI | PI | | | CTRL + 3 |
| CHECKBOX | Enable logo PI | PI | | | CTRL + 4 |

## Opcje trybu V:

| Skrót | Opis | Uwagi |
| --- | --- | --- |
| SPACE | Pauza/Odtwarzanie | |
| CTRL + LEFT |Początkowa klatka | |
| CTRL + RIGHT |Ostatnia klatka | |
| LEFT |Klatka w tył / Cofnij jedną | |
| RIGHT |Klatka do przodu / Następna klatka | |
| SHIFT + LEFT |15 klatek w tył | |
| SHIFT + RIGHT |15 klatek w przód | |
| ALT + LEFT | 50 klatek w tył | |
| ALT + RIGHT | 50 klatek w przód | |
| CTRL + SHIFT + LEFT | Odtwarzaj od bieżącej do pierwszej klatki | |
| CTRL + SHIFT + RIGHT | Odtwarzaj od bieżącej do końcowej klatki | |
| CTRL + Q  | Wyczyszczenie ciągu commands | w trybach C, V, I |  |

## Opcje trybu PI:
| Skrót | Opis | Uwagi |
| --- | --- | --- |
| CTRL + V |Wklejenie zawartości schowka do wybranych boxów |
| DELETE | Usuń całą zawartość boxa |
| BACKSPACE | Usuń ostatni znak w boxie |
