# 🎧 Internet Radio v3 – ESP32-S3 Internet Radio & Audio Player (IR Remote Controlled)

> 📻 **Pełna obsługa radia internetowego i plików audio z karty SD dla ESP32-S3 z wyświetlaczem TFT ILI9488 i DAC PCM5102A.**  
> 💡 **Sterowanie wyłącznie pilotem IR (NEC 38 kHz)** — bez enkoderów i przycisków fizycznych.

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
`
#include <FreeSans12pt7b.h>       // Sans, 12pt – nagłówki i opisy
#include <FreeMonoBold12pt7b.h>   // Mono Bold, 12pt – dane liczbowe
#include <FreeMonoBold18pt7b.h>   // Mono Bold, 18pt – liczniki
#include <FreeSansBold18pt7b.h>   // Sans Bold, 18pt – wyróżnienia
#include <FreeMono18pt7b.h>       // Mono, 18pt – siatki danych
#include <DS_DIGII35pt7b.h>       // Styl 7-segmentowy, 35pt – duży zegar
`

---

## 🌟 Opis projektu (PL)

**Internet Radio v3** to nowoczesny projekt odtwarzacza audio i radia internetowego opartego na **ESP32-S3**.  
Działa w dwóch trybach:
- 📡 **Internet Radio**
- 💾 **Odtwarzacz plików audio z karty SD**

Projekt nie wymaga żadnych pokręteł ani przycisków — **pełna obsługa odbywa się z pilota IR**.

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

## 🎮 Sterowanie pilotem IR

Projekt obsługuje **pilot NEC 38 kHz**.  
Każdy przycisk ma przypisaną funkcję w zależności od trybu pracy.

---

### ⚙️ Przyciskowe sterowanie – ogólnie

| 🔘 Przycisk | 🧭 Funkcja |
|--------------|------------|
| **MODE** | Zmiana trybu: Radio ↔ Odtwarzacz plików |
| **HOME** | Powrót do ekranu głównego |
| **OK** | Zatwierdzenie wyboru |
| **GOTO / FolderList** | Wyświetlenie listy folderów |
| **FAV+ / FAV-** | Zmiana banku stacji (radio) |
| **VOL+ / VOL-** | Regulacja głośności |
| **MUTE** | Wyciszenie / ponowne włączenie |
| **PLAY / PAUSE** | Wstrzymaj / Wznów odtwarzanie |
| **CYFRY 0–9** | Wpisywanie numeru stacji, folderu lub pliku |
| **↑ / ↓ / ← / →** | Przewijanie lub nawigacja po liście |

---

### 📡 Tryb: Radio internetowe

| Przycisk | Działanie |
|-----------|------------|
| **↑ / ↓** | Przewijanie listy stacji |
| **← / →** | Nawigacja między pozycjami na liście |
| **OK** | Odtwarzanie wybranej stacji |
| **CYFRY 0–9** | Bezpośrednie wpisanie numeru stacji |
| **FAV+ / FAV-** | Zmiana banku stacji (1–18) |
| **VOL+ / VOL-** | Zmiana głośności |
| **MUTE** | Wyciszenie / ponowne włączenie |
| **PLAY / PAUSE** | Pauza / Wznów |
| **MODE** | Przejście do odtwarzacza plików |
| **HOME** | Powrót do ekranu głównego |

---

### 💾 Tryb: Odtwarzacz plików

| Przycisk | Działanie |
|-----------|------------|
| **↑ / ↓** | Przewijanie listy plików lub folderów |
| **← / →** | Zmiana widoku (foldery ↔ pliki) |
| **OK** | Otwórz folder lub odtwórz plik |
| **CYFRY 0–9** | Wybór folderu lub pliku (np. „123”) |
| **GOTO / FolderList** | Powrót do listy folderów |
| **VOL+ / VOL-** | Regulacja głośności |
| **PLAY / PAUSE** | Pauza / Wznów |
| **MUTE** | Wyciszenie / włączenie dźwięku |
| **MODE** | Przejście do radia internetowego |

---

## 💾 Struktura karty SD

/station_nr.txt ← ostatnia stacja radiowa
/bank_nr.txt ← ostatni bank
/folderIndex.txt ← ostatni folder
/fileIndex.txt ← ostatni plik
/bank01.txt ... bank18.txt ← listy stacji radiowych
/MUSIC/ ← katalog z plikami audio



---

## 📈 Informacje wyświetlane na ekranie

- 📻 Numer i nazwa stacji radiowej  
- 🌐 Adres URL strumienia  
- 🎵 Tytuł i wykonawca (jeśli dostępne)  
- 💡 Parametry audio:
  - Bitrate (kbps)
  - Sample rate (Hz)
  - Bit depth  
- 🔊 Poziom głośności  
- 🕒 Tryb automatycznego powrotu (po 12 s)

---

## 💾 Zapisywanie ustawień

Projekt automatycznie zapisuje bieżące ustawienia:
- Ostatnio wybraną stację i bank → `/station_nr.txt`, `/bank_nr.txt`
- Ostatnio odtwarzany folder i plik → `/folderIndex.txt`, `/fileIndex.txt`

Po restarcie ESP32-S3 projekt **automatycznie przywraca ostatni stan odtwarzania**.

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

## 🇬🇧 English Summary

**Internet Radio v3** is a dual-mode project for **ESP32-S3**:  
- Internet Radio (up to 18 banks × 99 stations)  
- Local Audio Player from SD (MP3, FLAC, AAC, OGG)

All control is performed **via IR remote**, no rotary encoders required.  
The project automatically saves and restores playback state after reboot.

---

📦 **Version:** 3.4.x (development build)  
🧠 **Platform:** ESP32-S3  
🎨 **Display:** ILI9488 (480×320)  
🎧 **Audio:** PCM5102A DAC  
📡 **Control:** IR Remote (NEC 38 kHz)  
💾 **Storage:** microSD  

---

👤 **Author:** _[Sławomir Malinowski / MAJSTER XXL / sarunia]_  
🌍 **Repository:** [GitHub – Internet-Radio-v3](https://github.com/TwojProfil/Internet-Radio-v3)