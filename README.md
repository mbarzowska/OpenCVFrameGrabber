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

| Typ opcji | Opcja | Dostępne w trybie | Opis | Skrót | Tryb skrótu |
| ---  | ---  | --- | --- | --- | --- |
| CHECKBOX | Put logo on | C, V, I | Nakładanie logo na klatkę OT |
| CHECKBOX | Record straight to video file | C, V | Zapis strumienia OT do pliku video | S - start, E - end | C |
| TRACKBAR | Set FPS | C, V | Ustawienie pożądanej ilości FPS dla pliku video | 
| Save video to frames: | - | - | - | - | - |
| CHECKBOX | On demand (must be running) | C, V | | S - start, E - end | V |
| CHECKBOX | Given frames | V |
| CHECKBOX | (Given frames) Start point | V | 
| CHECKBOX | (Given frames) Quantity | V |
| BUTTON | GO! | V |
| CHECKBOX | Given frames | V |
| CHECKBOX | (Given frames) Given time | V |
| CHECKBOX | (Given frames) Start point | V | 
| CHECKBOX | (Given frames) Quantity | V | 
| BUTTON | GO! | V |
| - | - | - | - | - | - |
| BUTTON | Save current to file | C, V, I |
| CHECKBOX | Show image | C, V |
| CHECKBOX | Join frames to video | jeśli istnieje ścieżka |
| CHECKBOX | Enable PI | CTRL + P | C, V, I |
| CHECKBOX | Enable video PI | 1 | jeśli PI włączony |
| CHECKBOX | Enable frame PI | 2 | jeśli PI włączony |
| CHECKBOX | Enable image PI | 3 | jeśli PI włączony |
| CHECKBOX | Enable logo PI | 4 | jeśli PI włączony |
