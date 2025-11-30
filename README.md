# 🎧 Internet Radio v3 – ESP32-S3 Internet Radio & Audio Player (IR Remote Controlled)

>  **Pełna obsługa radia internetowego i plików audio z karty SD dla ESP32-S3 z wyświetlaczem TFT ILI9488 i DAC PCM5102A.**  
>  **Sterowanie wyłącznie pilotem IR (NEC 38 kHz)** — bez enkoderów i przycisków fizycznych.
>  **Kartka z kalendarza, która w trybie radia wyświetla cyklicznie w linii informacje o dacie, wschodzie i zachodzie słonca, długości dnia, imieninach.**
>  **Pogodynka, która w trybie radia wyświetla cyklicznie dane ze swojej lokalizacji - wymagany własny URL z openweathermap.org, pogodynka informuje o aktualnej temperaturze, wilgotności, ciśnieniu, wietrze.**
>  **W trybie radia dodano wyświetlanie skróconych informacji z kanału RSS Polsat News Polska, informacje są podane na pełnym ekranie co 2 minuty przez czas 20 sekund** 

---

## 🧰 Instalacja i konfiguracja środowiska

Aby poprawnie uruchomić projekt, wykonaj poniższe kroki:

### 1️⃣ Zainstaluj Arduino IDE
- Pobierz i zainstaluj **Arduino IDE w wersji 2.3.6**  
  🔗 [Pobierz Arduino IDE 2.3.6](https://www.arduino.cc/en/software)

---

### 2️⃣ Dodaj obsługę ESP32
- Otwórz:  
  `Plik → Preferencje → Dodatkowe adresy URL do menedżera płytek`
- Wklej poniższy adres:
https://espressif.github.io/arduino-esp32/package_esp32_index.json

- Następnie przejdź do:  
`Narzędzia → Płytka → Menedżer płytek`
- Wyszukaj **esp32** i zainstaluj wersję **3.3.2**
- Ustaw zakres dla pamięci flash:  
`Narzędzia → Flash Size → 8MB(64Mb)`
- Ustaw rozmiar partycji:  
`Narzędzia → Partition Scheme → 8M with spiffs`
- Włącz pamięć PSRAM:  
`Narzędzia → PSRAM → OPI PSRAM`

---

### 📦 3️⃣ Zainstaluj wymagane biblioteki

Wszystkie biblioteki można pobrać na dwa sposoby:
- ✅ **Z poziomu Arduino IDE:**  
`Szkic → Dołącz bibliotekę → Zarządzaj bibliotekami...`
- 💾 **Lub ręcznie z GitHub:**  
Pobierz plik `.zip`, a następnie:  
`Szkic → Dołącz bibliotekę → Dodaj bibliotekę .ZIP`

---

#### 🔹 Główne biblioteki projektu

| Biblioteka | Opis | Instalacja |
|-------------|------|------------|
| **ESP32-audioI2S** (v3.4.2) | Odtwarzanie strumieni i plików audio (MP3, FLAC, AAC, OGG) | [GitHub – ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S) |
| **Adafruit GFX** | Uniwersalna biblioteka graficzna (rysowanie, czcionki) | Menedżer bibliotek |
| **WiFiManager** | Automatyczne łączenie z Wi-Fi, konfiguracja przez portal | [GitHub – WiFiManager](https://github.com/tzapu/WiFiManager) |
| **ArduinoJson** | Obsługa plików konfiguracyjnych i danych JSON | Menedżer bibliotek |
| **Ticker** | Cykliczne wywołania (np. co sekundę) | Menedżer bibliotek |
| **Time** | Obsługa czasu systemowego | Menedżer bibliotek |
| **SD**, **SPI**, **FS** | Obsługa kart SD i magistrali SPI | Wbudowane |
| **EEPROM**, **HTTPClient**, **Arduino.h** | Standardowe biblioteki ESP32 | Wbudowane w pakiet esp32 |

---

### 🔤 4️⃣ Czcionki

Projekt wykorzystuje czcionki **FreeFonts** oraz jedną niestandardową czcionkę cyfrową.  
📁 Wszystkie czcionki są **dołączone do projektu** – nie wymagają osobnej instalacji.

```cpp
#include <FreeSans12pt7b.h>
#include <FreeMonoBold12pt7b.h>
#include <FreeMonoBold18pt7b.h>
#include <FreeSansBold18pt7b.h>
#include <FreeMono18pt7b.h>
#include <DS_DIGII35pt7b.h>
```

---

## 🌟 Opis projektu (PL)

**Internet Radio v3** to nowoczesny projekt odtwarzacza audio i radia internetowego opartego na **ESP32-S3**.  
Działa w dwóch trybach:
- 📡 **Radio internetowe**
- 💾 **Odtwarzacz plików audio z karty SD**

Projekt nie wymaga żadnych pokręteł ani przycisków — **pełna obsługa odbywa się z pilota IR**. Należy we własnym zakresie przypisać przyciski swojego pilota, korzystajac z Serial terminala:
`Narzędzia → Monitor portu szeregowego` odczytać adres i komendę dla każdego przycisku, który chcemy zaplanowac do przypisania odpowiedniej funkcji, w kodzie programu następnie należy ustawić te kody pilota dla definicji przycisków:
```cpp
#define rcCmdVolumeUp     0x0013   // Przycisk VOL+
#define rcCmdVolumeDown   0x0008   // Przycisk VOL-
#define rcCmdArrowRight   0x0014   // Przycisk w prawo - następna stacja, następny plik, od razu uruchamiane przejście
#define rcCmdArrowLeft    0x0016   // Przycisk w lewo - poprzednia stacja, poprzedni plik, od razu uruchamiane przejście
#define rcCmdArrowUp      0x0019   // Przycisk w górę - lista stacji, lista plików - krok do góry na przewijanej liście
#define rcCmdArrowDown    0x0011   // Przycisk w dół - lista stacji, lista plikow - krok w dół na przewijanej liście
#define rcCmdOk           0x0015   // Przycisk OK - zatwierdzenie wybranej stacji, banku, folderu, pliku
#define rcCmdMode         0x000E   // Przycisk SOURCE - przełączanie radio internetowe, odtwarzacz plików
#define rcCmdMute         0x000A   // Przycisk MUTE - wyciszenie
#define rcCmdKey0         0x004C   // Przycisk "0"
#define rcCmdKey1         0x0040   // Przycisk "1"
#define rcCmdKey2         0x0041   // Przycisk "2"
#define rcCmdKey3         0x0042   // Przycisk "3"
#define rcCmdKey4         0x0044   // Przycisk "4"
#define rcCmdKey5         0x0045   // Przycisk "5"
#define rcCmdKey6         0x0046   // Przycisk "6"
#define rcCmdKey7         0x0048   // Przycisk "7"
#define rcCmdKey8         0x0049   // Przycisk "8"
#define rcCmdKey9         0x004A   // Przycisk "9"
#define rcCmdBankUp       0x001B   // Przycisk FF+ - numer banku, krok w dół
#define rcCmdBankDown     0x0017   // Przycisk FF- - numer banku, krok w górę
#define rcCmdPauseResume  0x0012   // Przycisk Play / Pause
#define rcCmdFolderList   0x004E   // Przycisk GOTO - wyświetlenie listy folderów z zaznaczeniem aktualnego folderu
#define rcCmdStop         0x0010   // Przycisk STOP
#define rcCmdProg         0x000F   // Przycisk PROG - włącza i zamyka Menu wyboru ustawień do wyświetlania danych dodatkowych w trybie radia
#define rcCmdMemory       0x001F   // Przycisk MEMORY - włącza i zatrzymuje nagrywanie stacji radiowej do pliku mp3 na karcie SD
#define rcCmdRandom       0x001A   // Przycisk RANDOM - włącza i wyłącza tryb losowy podczas odtwarzania plikow z folderu
```
Przypisanie adresu i komendy dla każdego przycisku polega na sklejeniu 2 bajtów np. odczytany przycisk z serial terminala"
`ADR:0 CMD:4B`
trzeba skleić do postaci:
`0x004B`
Projekt obsługuje **pilot NEC 38 kHz**.  
Każdy przycisk ma przypisaną funkcję w zależności od trybu pracy.
Poniżej znajduje się przykładowy pilot używany podczas tworzenia projektu:

<p align="center"> <img src="https://raw.githubusercontent.com/sarunia/ESP32_internet_radio_v3/main/Zdj%C4%99cia/20251026_164114.jpg" width="300"> </p>

---

## 🔍 Funkcjonalności

- 🎨 **Wyświetlacz TFT 480×320 (ILI9488)** z obsługą kolorów RGB
- 🖋️ Czytelne czcionki **FreeSans / FreeMono** w wielu rozmiarach
- 📶 **Radio internetowe (MP3 / AAC / FLAC / OGG)**
- Do **18 banków** stacji (1–18)
- Każdy bank do **99 stacji**
- Pobieranie list stacji z plików `bankXX.txt`
- 💾 **Odtwarzacz plików audio z karty SD**
- Nawigacja po folderach i plikach
- Zapamiętywanie ostatniego folderu i pliku
- 🔊 **Pełna obsługa pilota IR**
- 💡 **Zapamiętywanie ostatnich ustawień**:
- `/station_nr.txt`
- `/bank_nr.txt`
- `/folderIndex.txt`
- `/fileIndex.txt`
- ⏱️ Automatyczny powrót do głównego widoku po 12 s bezczynności

---

### ⚙️ Przyciskowe sterowanie – ogólnie

| 🔘 Przycisk | 🧭 Funkcja |
|--------------|------------|
| **SOURCE** | Zmiana trybu: Radio ↔ Odtwarzacz plików |
| **OK** | Zatwierdzenie wyboru |
| **GOTO** | Wyświetlenie listy folderów |
| **FAV+ / FAV-** | Zmiana banku stacji (radio) |
| **VOL+ / VOL-** | Regulacja głośności |
| **CYFRY 0–9** | Wpisywanie numeru stacji, folderu lub pliku |
| **↑ / ↓ / ← / →** | Przewijanie lub nawigacja po liście |

---

### 📡 Tryb: Radio internetowe

| Przycisk | Działanie |
|-----------|------------|
| **↑ / ↓** | Przewijanie listy stacji |
| **← / →** | Przełączanie stacji o jedną do przodu lub do tyłu |
| **OK** | Odtwarzanie wybranej stacji |
| **CYFRY 0–9** | Bezpośrednie wpisanie numeru stacji |
| **FAV+ / FAV-** | Zmiana banku stacji (1–18) |
| **VOL+ / VOL-** | Zmiana głośności |
| **SOURCE** | Przejście do wyboru źródła audio |
| **STOP** | Zatrzymanie odtwarzania i przejście do listy stacji |
| **PROG** | Włączenie i zamknięcie Menu dla aktywacji pogodynki, kartki z kalendarza, wiadomości z kanału RSS |
| **MEMORY** | Włączenie i zatrzymanie nagrywania stacji radiowych do pliku mp3 na kartę SD |

---
Poniżej przedstawiono widok urządzenia w trybie radia internetowego:

<p align="center"> <img src="https://raw.githubusercontent.com/sarunia/ESP32_internet_radio_v3/main/Zdj%C4%99cia/20250913_225635.jpg" width="380"> </p>

### 💾 Tryb: Odtwarzacz plików

| Przycisk | Działanie |
|-----------|------------|
| **↑ / ↓** | Przewijanie listy plików podczas odtwarzania |
| **← / →** | Przełączanie pliku o jeden do przodu lub do tyłu |
| **OK** | Otwórz folder lub odtwórz plik |
| **CYFRY 0–9** | Wybór folderu lub pliku (np. „123”) |
| **GOTO** | Wyświetlenie listy folderów |
| **VOL+ / VOL-** | Regulacja głośności |
| **SOURCE** | Przejście do wyboru źródła audio |
| **STOP** | Zatrzymanie odtwarzania i przejście do listy folderów |

---
Poniżej widok urządzenia podczas pracy w trybie odtwarzacza plików:

<p align="center"> <img src="https://raw.githubusercontent.com/sarunia/ESP32_internet_radio_v3/main/Zdj%C4%99cia/20251004_190120.jpg" width="380"> </p>

## 💾 Struktura karty SD
```cpp
/station_nr.txt ← ostatnia stacja radiowa
/bank_nr.txt ← ostatni bank
/folderIndex.txt ← ostatni folder
/fileIndex.txt ← ostatni plik
/bank01.txt ... bank18.txt ← listy stacji radiowych
/ ← foldery z plikami audio muszą być w głównym katalogu karty SD
```

---

## ⚙️ Wymagane biblioteki

| Biblioteka | Opis |
|-------------|------|
| **Adafruit GFX** | Obsługa grafiki TFT |
| **FreeFonts (FreeSans / FreeMono)** | Czcionki tekstowe |
| **ArduinoJson** | Przetwarzanie plików konfiguracyjnych |
| **WiFiManager** | Automatyczne łączenie z Wi-Fi |
| **ESP32-audioI2S** | Odtwarzanie strumieni i plików audio |
| **SD, SPI, FS, Ticker, Time** | Obsługa karty SD i systemu czasu |

---

## ⚡ Sprzęt

| Element | Model |
|----------|--------|
| 🧠 Mikrokontroler | **ESP32-S3** |
| 🖥️ Wyświetlacz | **TFT ILI9488 (480x320)** |
| 🎧 DAC audio | **PCM5102A** |
| 🔌 Sterowanie | **Pilot IR NEC 38 kHz** |
| 💾 Pamięć | **microSD** |

---

## ⚖️ Licencja

Ten projekt jest udostępniony na zasadach **wolnej licencji MIT (MIT License)**.  
Oznacza to, że możesz:

- 🆓 **Używać** kodu w dowolnym celu (komercyjnym lub prywatnym)  
- ✏️ **Modyfikować** kod według własnych potrzeb  
- 📤 **Rozpowszechniać** projekt lub jego zmodyfikowane wersje  
- 💡 **Uczyć się** z tego kodu i wykorzystywać go w swoich projektach

Pod warunkiem, że w każdej kopii projektu lub jego części zachowasz informację o autorze i treść niniejszej licencji.

---

### 🧾 Pełna treść licencji MIT

MIT License

Copyright (c) 2025 [Sławomir Malinowski / MAJSTER XXL / sarunia]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

👤 **Author:** _[Sławomir Malinowski / MAJSTER XXL / sarunia]_  
🌍 **Repository:** [GitHub – Internet-Radio-v3](https://github.com/sarunia/ESP32_internet_radio_v3)