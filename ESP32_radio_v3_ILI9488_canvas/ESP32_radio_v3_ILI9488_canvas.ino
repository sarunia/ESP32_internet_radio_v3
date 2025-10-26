#include <FreeSans12pt7b.h>       // Czcionka Sans, 12pt, normalna – dobra do nagłówków i mniejszych opisów
#include <FreeMonoBold12pt7b.h>   // Czcionka Mono (stała szerokość), 12pt, pogrubiona – przydatna np. do tabel lub danych liczbowych
#include <FreeMonoBold18pt7b.h>   // Czcionka Mono (stała szerokość), 18pt, pogrubiona – wyraźna, nadaje się np. do liczników
#include <FreeSansBold18pt7b.h>   // Czcionka Sans, 18pt, pogrubiona – czytelna, do większych nagłówków i wyróżnień
#include <FreeMono18pt7b.h>       // Czcionka Mono (stała szerokość), 18pt, normalna – równe odstępy, np. do siatek danych
#include <DS_DIGII35pt7b.h>       // Czcionka cyfrowa (styl wyświetlacza 7-segmentowego), 35pt – idealna do dużego zegara

#include "Arduino.h"              // Standardowy nagłówek Arduino, który dostarcza podstawowe funkcje i definicje
#include <HTTPClient.h>           // Biblioteka do wykonywania żądań HTTP, umożliwia komunikację z serwerami przez protokół HTTP
#include <EEPROM.h>               // Biblioteka do obsługi pamięci EEPROM, przechowywanie danych w pamięci nieulotnej
#include <Ticker.h>               // Mechanizm tickera do odświeżania timera 1s, pomocny do cyklicznych akcji w pętli głównej
#include <WiFiManager.h>          // Biblioteka do zarządzania konfiguracją sieci WiFi, opis jak ustawić połączenie WiFi przy pierwszym uruchomieniu jest opisany tu: https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // Biblioteka do parsowania i tworzenia danych w formacie JSON, użyteczna do pracy z API
#include <Time.h>                 // Biblioteka do obsługi funkcji związanych z czasem, np. odczytu daty i godziny
#include "Audio.h"                // Biblioteka do obsługi funkcji związanych z dźwiękiem
#include "SD.h"                   // Biblioteka do obsługi kart SD
#include "FS.h"                   // Biblioteka do obsługi systemu plików
#include "SPI.h"                  // Biblioteka do obsługi komunikacji SPI (ekran TFT, karta SD itp.)
#include <Adafruit_GFX.h>         // Uniwersalna biblioteka graficzna Adafruit GFX (podstawy rysowania, obsługa czcionek)
#include "FilePlayer.h"           // Obsługa odtwarzacza plików przeniesiona tutaj

// Definicje pinów dla SPI wyświetlacza TFT typu ILI9488
#define TFT_CS    5    // Pin CS (Chip Select) – wybór układu TFT
#define TFT_DC    4    // Pin DC (Data/Command) – rozróżnia dane od komend
#define TFT_RST  -1    // Pin RESET – tutaj -1 oznacza brak użycia sprzętowego resetu
#define TFT_MOSI 12    // Pin MOSI (Master Out Slave In) – dane z ESP32 do TFT
#define TFT_MISO 13    // Pin MISO (Master In Slave Out) – odczyt danych z TFT (rzadko używany)
#define TFT_SCLK 11    // Pin SCLK (Serial Clock) – zegar SPI

// Rozdzielczość wyświetlacza TFT (dla ILI9488 to 480x320)
#define TFT_WIDTH   480   // Szerokość ekranu w pikselach
#define TFT_HEIGHT  320   // Wysokość ekranu w pikselach
GFXcanvas16 canvas(TFT_WIDTH, TFT_HEIGHT);  // Bufor do rysowania całego ekranu w pamięci RAM przed wysłaniem na TFT

// Definicje pinów dla SPI czytnika kart SD
#define SD_SCK     45             // Pin SCK dla karty SD
#define SD_MISO    21             // Pin MISO dla karty SD
#define SD_MOSI    48             // Pin MOSI dla karty SD
#define SD_CS      47             // Pin CS dla karty SD

// Definicje pinów dla I2S modułu DAC z PCM5102A
#define I2S_BCLK      16          // Podłączenie po pinu BCK na module DAC z PCM5102A
#define I2S_DOUT      17          // Podłączenie do pinu DIN na module DAC z PCM5102A
#define I2S_LRC       18          // Podłączenie do pinu LCK na module DAC z PCM5102A

// Makra upraszczające sterowanie liniami TFT
#define CS_ACTIVE   digitalWrite(TFT_CS, LOW)   // Aktywacja wyświetlacza (CS = LOW)
#define CS_IDLE     digitalWrite(TFT_CS, HIGH)  // Dezaktywacja wyświetlacza (CS = HIGH)
#define DC_COMMAND  digitalWrite(TFT_DC, LOW)   // Tryb komend (DC = LOW) – dane traktowane jako instrukcje
#define DC_DATA     digitalWrite(TFT_DC, HIGH)  // Tryb danych (DC = HIGH) – dane traktowane jako piksele

#define MAX_STATIONS 99           // Maksymalna liczba stacji radiowych, które mogą być przechowywane w jednym banku
#define STATION_NAME_LENGTH 42    // Nazwa stacji wraz z bankiem i numerem stacji do wyświetlenia w pierwszej linii na ekranie
#define MAX_DIRECTORIES 192       // Maksymalna liczba katalogów
#define MAX_FILES 192             // Maksymalna liczba plików w katalogu

#define STATIONS_URL1   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_01"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL2   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_02"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL3   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_03"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL4   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_04"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL5   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_05"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL6   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_06"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL7   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_07"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL8   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_08"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL9   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_09"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL10  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_10"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL11  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_11"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL12  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_12"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL13  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_13"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL14  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_14"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL15  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_15"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL16  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_16"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL17  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_17"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL18  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_18"      // Adres URL do pliku z listą stacji radiowych

// Konwersja z 8-bitowego RGB na 16-bit RGB565
#define RGB565(r,g,b)  (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// Definicje kolorów
#define COLOR_RED         RGB565(255, 0, 0)      // Czerwony
#define COLOR_GREEN       RGB565(0, 255, 0)      // Zielony
#define COLOR_BLUE        RGB565(0, 0, 255)      // Niebieski
#define COLOR_YELLOW      RGB565(255, 255, 0)    // Żółty
#define COLOR_CYAN        RGB565(0, 255, 255)    // Turkus / błękitny
#define COLOR_MAGENTA     RGB565(255, 0, 255)    // Magenta (fuksja)
#define COLOR_ORANGE      RGB565(255, 128, 0)    // Pomarańczowy
#define COLOR_PURPLE      RGB565(128, 0, 255)    // Fiolet
#define COLOR_PINK        RGB565(255, 0, 128)    // Różowy
#define COLOR_LIME        RGB565(128, 255, 0)    // Jasna limonka
#define COLOR_TURQUOISE   RGB565(0, 128, 255)    // Turkus
#define COLOR_WHITE       RGB565(255, 255, 255)  // Biały
#define COLOR_GOLD        RGB565(231, 211, 90)   // Złoty
#define COLOR_BLACK       RGB565(0, 0, 0)        // Czarny
#define COLOR_SKYBLUE     RGB565(135, 206, 235)  // Jasny niebieski (sky blue)
#define COLOR_SPRINGGREEN RGB565(0, 255, 127)    // Zielony wiosenny (spring green)
#define COLOR_DEEPPINK    RGB565(255, 20, 147)   // Intensywny róż (deep pink)
#define COLOR_CORAL       RGB565(255, 127, 80)   // Koralowy (coral)
#define COLOR_VIOLET      RGB565(238, 130, 238)  // Jasny fiolet (violet)
#define COLOR_BROWN       RGB565(139, 69, 19)    // Brązowy (brown)
#define COLOR_NAVY        RGB565(0, 0, 128)      // Granatowy (navy)
#define COLOR_GRAY        RGB565(128, 128, 128)  // Szary (gray)
#define COLOR_OLIVE       RGB565(128, 128, 0)    // Oliwkowy (olive)
#define COLOR_MAROON      RGB565(128, 0, 0)      // Ciemnoczerwony (maroon)

const unsigned long DISPLAY_TIMEOUT = 12000;

int currentSelection = 0;         // Numer aktualnego wyboru na ekranie OLED
int firstVisibleLine = 0;         // Numer pierwszej widocznej linii na ekranie OLED
int station_nr;                   // Numer aktualnie wybranej stacji radiowej z listy
int previous_station_nr = 0;      // Numer stacji radiowej przechowywanej w buforze do przywrócenia na ekran po bezczynności
int bank_nr;                      // Numer aktualnie wybranego banku stacji z listy
int previous_bank_nr = 0;         // Numer aktualnie wybranego banku stacji z listy do przywrócenia na ekran po bezczynności       
int stationsCount = 0;            // Aktualna liczba przechowywanych stacji w tablicy
int folderCount = 0;              // Licznik folderów na karcie SD
int filesCount = 0;               // Licznik plików w danym folderze na karcie SD
int fileIndex = 0;                // Numer aktualnie wybranego pliku audio ze wskazanego folderu
int previous_fileIndex = 0;       // Numer aktualnie wybranego pliku do przywrócenia na ekran po bezczynności
int folderIndex = 0;              // Numer aktualnie wybranego folderu podczas przełączenia do odtwarzania z karty SD
int previous_folderIndex = 0;     // Numer aktualnie wybranego folderu do przywrócenia na ekran po bezczynności oraz
int volumeValue = 12;             // Wartość głośności, domyślnie ustawiona na 12
int volumeArray[100];             // Wartości głośności dla 100 stacji w każdym banku
int cycle = 0;                    // Numer cyklu do danych pogodowych wyświetlanych w trzech rzutach co 10 sekund
int maxVisibleLines = 6;          // Maksymalna liczba widocznych linii na ekranie OLED
int maxVisibleFolders = 6;        // Maksymalna liczba folderów widocznych na ekranie w danym momencie

unsigned long lastSwitchWeather = 0;   // Czas ostatniego przełączenia widoku pogody (do rotacji danych)
unsigned long lastSwitchCalendar = 0;  // Czas ostatniego przełączenia widoku kalendarza

int messageIndex = 0;               // Indeks aktualnie wyświetlanej wiadomości
int namedayLineIndex = 0;           // Indeks aktualnie wyświetlanej linii imienin
std::vector<String> namedayLines;   // Wektor z gotowymi liniami tekstu imienin

bool fileEnd = false;             // Flaga sygnalizująca koniec odtwarzania pliku audio
bool displayActive = false;       // Flaga określająca, czy wyświetlacz jest aktywny
bool isPlaying = false;           // Flaga określająca, czy obecnie trwa odtwarzanie
bool mp3 = false;                 // Flaga określająca, czy aktualny plik audio jest w formacie MP3
bool flac = false;                // Flaga określająca, czy aktualny plik audio jest w formacie FLAC
bool aac = false;                 // Flaga określająca, czy aktualny plik audio jest w formacie AAC
bool vorbis = false;              // Flaga określająca, czy aktualny plik audio jest w formacie VORBIS
bool id3tag = false;              // Flaga określająca, czy plik audio posiada dane ID3
bool menuEnable = false;          // Flaga określająca, czy na ekranie można wyświetlić menu
bool bitratePresent = false;      // Flaga określająca, czy na serial terminalu pojawiła się informacja o bitrate - jako ostatnia dana spływajaca z info
bool playNextFile = false;        // Flaga określająca przejście do kolejnego odtwarzanego pliku audio
bool playPreviousFile = false;    // Flaga określająca przejście do poprzednio odtwarzanego pliku audio
bool weatherServerConnection = false;  // Flaga określająca połączenie z serwerem pogody
bool folderSelection = false;     // Flaga określająca wyświetlanie listy folderów z karty SD
bool fileSelection = false;       // Flaga określająca wyświetlanie listy plików z aktualnego folderu
bool bankSwitch = false;          // Flaga określająca aktywny tryb wybierania numeru banku
bool isMuted = false;             // Flaga pomocnicza czy aktualnie jest wyciszenie
bool isPaused = false;            // Flaga pomocnicza czy aktualnie jest pauza
bool stationsList = false;        // Flaga określająca aktywny tryb wyświetlania listy stacji radiowych podczas przewijania wyboru
bool folderList = false;          // Flaga określająca aktywny tryb wyświetlania listy folderów z karty SD
bool randomMode = false;

unsigned long displayStartTime = 0;       // Czas rozpoczęcia wyświetlania komunikatu
unsigned long seconds = 0;                // Licznik sekund timera

// Definicje flag do obsługi z pilota zdalnego sterowania z protokołu NEC 38kHz
bool IRrightArrow = false;        // Flaga określająca użycie zdalnego sterowania z pilota IR - kierunek w prawo
bool IRleftArrow = false;         // Flaga określająca użycie zdalnego sterowania z pilota IR - kierunek w lewo
bool IRupArrow = false;           // Flaga określająca użycie zdalnego sterowania z pilota IR - kierunek w górę
bool IRdownArrow = false;         // Flaga określająca użycie zdalnego sterowania z pilota IR - kierunek w dół
bool IRsourceButton = false;      // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk "SOURCE"
bool IRokButton = false;          // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk środkowy "OK"
bool IRvolumeUp = false;          // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk VOL+
bool IRvolumeDown = false;        // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk VOL-
bool IRbankUp = false;            // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk FAV+
bool IRbankDown = false;          // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk FAV-
bool IRpauseResume = false;       // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk Play / Pause
bool IRmuteTrigger = false;       // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk Mute
bool IRstopButton = false;        // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk STOP
bool IRprogButton = false;        // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk PROG
bool IRmemoryButton = false;      // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk MEMORY
bool IRrandomButton = false;      // Flaga określająca użycie zdalnego sterowania z pilota IR - przycisk RANDOM

String directories[MAX_DIRECTORIES];   // Tablica do przechowywania nazw folderów na karcie SD
String files[MAX_FILES];               // Tablica do przechowywania nazw plików na karcie SD

String currentDirectory = "/";         // Ścieżka bieżącego katalogu w systemie plików

String stationName;                    // Nazwa aktualnie wybranej stacji radiowej
String stationInfo;                    // Dodatkowe informacje o stacji radiowej (np. opis, meta)

String bitrateString;                  // Informacja o bitrate aktualnie odtwarzanego strumienia
String sampleRateString;               // Informacja o częstotliwości próbkowania (Hz)
String bitsPerSampleString;            // Informacja o liczbie bitów na próbkę audio

String artistString;                   // Nazwa wykonawcy (odczytana z metadanych ID3)
String titleString;                    // Tytuł utworu (odczytany z metadanych ID3)
String albumString;                    // Nazwa albumu (odczytana z metadanych ID3)
String yearString;                     // Rok wydania utworu (odczytany z metadanych ID3)

unsigned long trackStartMillis = 0;    // Czas w milisekundach, kiedy rozpoczęto odtwarzanie bieżącego utworu
String fileTime = "00h:00m:00s";       // Aktualny czas trwania odtwarzanego utworu w formacie hh:mm:ss

String fileNameString;                 // Nazwa aktualnie odtwarzanego pliku
String folderNameString;               // Nazwa folderu, w którym znajduje się plik

String calendar;                       // Dane kalendarza
String sunrise;                        // Godzina wschodu słońca
String sunset;                         // Godzina zachodu słońca
String dayLength;                      // Długość dnia
String namedays;                       // Imieniny przypadające na dany dzień

String fileType;                       // Typ pliku (np. MP3, WAV, FLAC)
String volumeDisplay;                  // Tekst z poziomem głośności (np. "VOL 12")

// Przygotowanie danych pogody do wyświetlenia
String tempStr;           // Zmienna do przechowywania temperatury
String feels_likeStr;     // Zmienna do przechowywania temperatury odczuwalnej
String humidityStr;       // Zmienna do przechowywania wilgotności
String pressureStr;       // Zmienna do przechowywania ciśnienia atmosferycznego
String windStr;           // Zmienna do przechowywania prędkości wiatru
String windGustStr;       // Zmienna do przechowywania prędkości porywów wiatru

File myFile; // Uchwyt pliku

Audio audio;                              // Obiekt do obsługi funkcji związanych z dźwiękiem i audio
AudioBuffer audioBuffer;                  // Bufor audio do płynniejszego odtwarzania strumieni i plików
Ticker timer1;                            // Timer do updateTimer co 1s
Ticker timer2;                            // Timer do getWeatherData co 60s
Ticker timer3;                            // Timer do przełączania wyświetlania danych pogodoych w ostatniej linii co 10s

volatile bool updateClockFlag = false;  // flaga do odświeżania zegara

void IRAM_ATTR timerCallback()
{
  updateClockFlag = true;  // Tylko ustaw flagę w przerwaniu
  //drawClock();
}

WiFiClient client;                        // Obiekt do obsługi połączenia WiFi dla klienta HTTP

SPIClass spi = SPIClass(HSPI); // Użycie HSPI dla ekranu

// Konfiguracja dodatkowego SPI z wybranymi pinami dla czytnika kart SD
SPIClass customSPI = SPIClass(FSPI);  // Użycie FSPI dla karty SD

char stations[MAX_STATIONS][STATION_NAME_LENGTH];   // Tablica przechowująca nazwy stacji wraz z bankiem i numerem stacji

const char* ntpServer = "pool.ntp.org";      // Adres serwera NTP używany do synchronizacji czasu

// Deklaracja obiektu JSON
StaticJsonDocument<1024> doc;     // Przyjęto rozmiar JSON na 1024 bajty

MenuOption currentOption = INTERNET_RADIO;  // Aktualnie wybrana opcja menu (domyślnie radio internetowe)

/*===============    Definicja portu i deklaracje zmiennych do obsługi odbiornika IR    =============*/
int recv_pin = 15;                          // Pin odbiornika IR
int bit_count = 0;                          // Licznik bitów w odebranym kodzie

volatile bool pulse_ready = false;          // Flaga sygnału gotowości
volatile bool pulse_ready9ms = false;       // Flaga sygnału gotowości
volatile bool pulse_ready_low = false;      // Flaga sygnału gotowości
bool data_start_detected = false;           // Flaga dla sygnału wstępnego

unsigned long pulse_start = 0;              // Czas początkowy impulsu
unsigned long pulse_end = 0;                // Czas końcowy impulsu
unsigned long pulse_duration = 0;           // Czas trwania impulsu
unsigned long pulse_duration_9ms = 0;       // Tylko do analizy - Czas trwania impulsu startowego 9ms 
unsigned long pulse_duration_4_5ms = 0;     // Tylko do analizy - Czas trwania impulsu startowego 4,5ms
unsigned long pulse_duration_560us = 0;     // Tylko do analizy - Czas trwania impulsu logicznego 0 (1,12ms - 0,56ms)
unsigned long pulse_duration_1690us = 0;    // Tylko do analizy - Czas trwania impulsu logicznej 1 (2,25ms - 0,56ms)
unsigned long pulse_start_low = 0;          // Czas początkowy impulsu
unsigned long pulse_end_low = 0;            // Czas końcowy impulsu
unsigned long pulse_duration_low = 0;       // Czas trwania impulsu
unsigned long runTime1 = 0;                 // Czas T1 służący do obliczenia czasu trwania impulsu sygnału IR
unsigned long runTime2 = 0;                 // Czas T2 służący do obliczenia czasu trwania impulsu sygnału IR
unsigned long ir_code = 0;                  // Zmienna do przechowywania kodu IR

const int LEAD_HIGH = 9000;         // 9 ms sygnał wysoki (początkowy)
const int LEAD_LOW = 4600;          // 4,5 ms sygnał niski (początkowy)
const int TOLERANCE = 160;          // Tolerancja (w mikrosekundach)
const int HIGH_THRESHOLD = 1690;    // Sygnał "1"
const int LOW_THRESHOLD = 600;      // Sygnał "0"

#define rcCmdVolumeUp     0x0013   // Przycisk VOL+
#define rcCmdVolumeDown   0x0008   // Przycisk VOL-
#define rcCmdArrowRight   0x0014   // Przycisk w prawo - następna stacja / następny plik, od razu uruchamiane przejście
#define rcCmdArrowLeft    0x0016   // Przycisk w lewo - poprzednia stacja / poprzedni plik, od razu uruchamiane przejście
#define rcCmdArrowUp      0x0019   // Przycisk w górę - lista stacji / lista plików - krok do góry na przewijanej liście
#define rcCmdArrowDown    0x0011   // Przycisk w dół - lista stacji / lista plikow - krok w dół na przewijanej liście
#define rcCmdOk           0x0015   // Przycisk OK - zatwierdzenie wybranej stacji / banku / folderu / pliku
#define rcCmdMode         0x000E   // Przycisk SOURCE - przełączanie radio internetowe / odtwarzacz plików
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
#define rcCmdBankUp       0x001B   // Przycisk FF+ - lista banków / lista folderów - krok w dół na przewijanej liście
#define rcCmdBankDown     0x0017   // Przycisk FF- lista banków / lista folderów - krok do góry na przewijanej liście
#define rcCmdPauseResume  0x0012   // Przycisk Play / Pause
#define rcCmdFolderList   0x004E   // Przycisk GOTO - wyświetlenie listy folderów z zaznaczeniem aktualnego folderu
#define rcCmdStop         0x0010   // Przycisk STOP
#define rcCmdProg         0x000F   // Przycisk PROG
#define rcCmdMemory       0x001F   // Przycisk MEMORY
#define rcCmdRandom       0x001A   // Przycisk RANDOM

String inputBuffer = "";       // Bufor na wciśnięte cyfry
unsigned long inputStartTime;  // Czas rozpoczęcia wpisywania numeru
bool inputActive = false;      // Flaga, czy jesteśmy w trybie wpisywania numeru stacji


// Funkcja obsługująca przerwanie (reakcja na zmianę stanu pinu)
void IRAM_ATTR pulseISR()
{
  runTime1 = esp_timer_get_time();
  if (digitalRead(recv_pin) == HIGH)
  {
    pulse_start = micros();  // Zapis początku impulsu
  }
  else
  {
    pulse_end = micros();    // Zapis końca impulsu
    pulse_ready = true;
  }

  if (digitalRead(recv_pin) == LOW)
  {
    pulse_start_low = micros();  // Zapis początku impulsu
  }
  else
  {
    pulse_end_low = micros();    // Zapis końca impulsu
    pulse_ready_low = true;
  }

  
  // ------------------------------ ANALIZA PULSÓW ----------------------------- //
  if (pulse_ready_low) // sprawdzamy czy jest stan niski przez 9ms - start ramki
  {
    pulse_duration_low = pulse_end_low - pulse_start_low;
  
    if (pulse_duration_low > (LEAD_HIGH - TOLERANCE) && pulse_duration_low < (LEAD_HIGH + TOLERANCE))
    {
      pulse_duration_9ms = pulse_duration_low; // przypisz czas trwania pulsu Low do zmiennej puls 9ms
      pulse_ready9ms = true; // flaga poprawnego wykrycia pulsu 9ms w granicach tolerancji
    }

  }
  // Sprawdzenie, czy impuls jest gotowy do analizy
  if ((pulse_ready== true) && (pulse_ready9ms = true))
  {
    pulse_ready = false;
    pulse_ready9ms = false; // kasujemy flagę wykrycia pulsu 9ms

    // Obliczenie czasu trwania impulsu
    pulse_duration = pulse_end - pulse_start;
    //Serial.println(pulse_duration); odczyt dlugosci pulsow z pilota - debug
    if (!data_start_detected)
    {
    
      // Oczekiwanie na sygnał 4,5 ms wysoki
      if (pulse_duration > (LEAD_HIGH - TOLERANCE) && pulse_duration < (LEAD_HIGH + TOLERANCE))
      {
       
        // Początek sygnału: 9 ms niski
        //Serial.println("Otrzymano początek sygnału (9 ms niski).");
        //pulse_duration_9ms = pulse_duration; 
      }
      else if (pulse_duration > (LEAD_LOW - TOLERANCE) && pulse_duration < (LEAD_LOW + TOLERANCE))
      {
        
        pulse_duration_4_5ms = pulse_duration;
        // Początek sygnału: 4,5 ms wysoki
        // Serial.println("Otrzymano początek sygnału (4,5 ms wysoki).");
        data_start_detected = true;  // Ustawienie flagi po wykryciu sygnału wstępnego
        bit_count = 0;               // Reset bit_count przed odebraniem danych
        ir_code = 0;                 // Reset kodu IR przed odebraniem danych
      }
    }
    else
    {
      // Sygnały dla bajtów (adresu ADDR, IADDR, komendy CMD, ICMD) zaczynają się po wstępnym sygnale
      if (pulse_duration > (HIGH_THRESHOLD - TOLERANCE) && pulse_duration < (HIGH_THRESHOLD + TOLERANCE))
      {
        ir_code = (ir_code << 1) | 1;  // Dodanie "1" do kodu IR
        bit_count++;
        pulse_duration_1690us = pulse_duration;
      }
      else if (pulse_duration > (LOW_THRESHOLD - TOLERANCE) && pulse_duration < (LOW_THRESHOLD + TOLERANCE))
      {
        ir_code = (ir_code << 1) | 0;  // Dodanie "0" do kodu IR
        bit_count++;
        pulse_duration_560us = pulse_duration;
      }

      // Sprawdzenie, czy otrzymano pełny 32-bitowy kod IR
      if (bit_count == 32)
      {
        // Rozbicie kodu na 4 bajty
        uint8_t ADDR = (ir_code >> 24) & 0xFF;  // Pierwszy bajt
        uint8_t IADDR = (ir_code >> 16) & 0xFF; // Drugi bajt (inwersja adresu)
        uint8_t CMD = (ir_code >> 8) & 0xFF;    // Trzeci bajt (komenda)
        uint8_t ICMD = ir_code & 0xFF;          // Czwarty bajt (inwersja komendy)

        // Sprawdzenie poprawności (inwersja) bajtów adresu i komendy
        if ((ADDR ^ IADDR) == 0xFF && (CMD ^ ICMD) == 0xFF)
        {
          data_start_detected = false;
        }
        else
        {
          ir_code = 0; 
          data_start_detected = false;       
        }

      }
    }
  }
  runTime2 = esp_timer_get_time();
}


// Odwrócenie kolejności bitów z otrzymanego ciągu z nadajnika IR
uint32_t reverse_bits(uint32_t inval, int bits)
{
  if ( bits > 0 )
  {
    bits--;
    return reverse_bits(inval >> 1, bits) | ((inval & 1) << bits);
  }
  return 0;
}
  

// ------------------------------------------------------------
// Funkcja przypisująca odpowiednie flagi do przycisków pilota
// ------------------------------------------------------------
void processIRCode()
{
  if (bit_count == 32 && ir_code != 0)
  {
    detachInterrupt(recv_pin);
    Serial.print("Kod NEC OK: ");
    Serial.print(ir_code, HEX);

    ir_code = reverse_bits(ir_code, 32); // zmiana z LSB-MSB na MSB-LSB
    Serial.print("  MSB-LSB: ");
    Serial.println(ir_code, HEX);

    uint8_t CMD = (ir_code >> 16) & 0xFF;
    uint8_t ADDR = ir_code & 0xFF;
    Serial.print("  ADR:");
    Serial.print(ADDR, HEX);
    Serial.print(" CMD:");
    Serial.println(CMD, HEX);
    ir_code = (ADDR << 8) | CMD; // końcowy kod

    attachInterrupt(digitalPinToInterrupt(recv_pin), pulseISR, CHANGE);

    // --- Mapowanie przycisków ---
    if      (ir_code == rcCmdArrowRight) IRrightArrow = true;
    else if (ir_code == rcCmdArrowLeft)  IRleftArrow  = true;
    else if (ir_code == rcCmdArrowUp)    IRupArrow    = true;
    else if (ir_code == rcCmdArrowDown)  IRdownArrow  = true;
    else if (ir_code == rcCmdMode)       IRsourceButton = true;
    else if (ir_code == rcCmdVolumeUp)   IRvolumeUp   = true;
    else if (ir_code == rcCmdVolumeDown) IRvolumeDown = true;
    else if (ir_code == rcCmdBankUp)     IRbankUp     = true;
    else if (ir_code == rcCmdBankDown)   IRbankDown   = true;
    else if (ir_code == rcCmdPauseResume) IRpauseResume = true;
    else if (ir_code == rcCmdMute)       IRmuteTrigger = true;
    else if (ir_code == rcCmdFolderList) folderList   = true;
    else if (ir_code == rcCmdStop)       IRstopButton   = true;
    else if (ir_code == rcCmdProg)       IRprogButton   = true;
    else if (ir_code == rcCmdMemory)     IRmemoryButton   = true;
    else if (ir_code == rcCmdRandom)     IRrandomButton   = true;

    // --- Przyciski numeryczne ---
    else if (ir_code == rcCmdKey0) handleDigitInput(0);
    else if (ir_code == rcCmdKey1) handleDigitInput(1);
    else if (ir_code == rcCmdKey2) handleDigitInput(2);
    else if (ir_code == rcCmdKey3) handleDigitInput(3);
    else if (ir_code == rcCmdKey4) handleDigitInput(4);
    else if (ir_code == rcCmdKey5) handleDigitInput(5);
    else if (ir_code == rcCmdKey6) handleDigitInput(6);
    else if (ir_code == rcCmdKey7) handleDigitInput(7);
    else if (ir_code == rcCmdKey8) handleDigitInput(8);
    else if (ir_code == rcCmdKey9) handleDigitInput(9);

    // --- Przycisk OK ---
    else if (ir_code == rcCmdOk)
    {
      if (inputActive && inputBuffer.length() > 0)
      {
        int chosenNumber = inputBuffer.toInt();

        if (currentOption == INTERNET_RADIO)
        {
          if (chosenNumber >= 1 && chosenNumber <= stationsCount)
          {
            station_nr = chosenNumber;
            currentSelection = station_nr - 1;
            firstVisibleLine = max(0, currentSelection - maxVisibleLines / 2);
            displayStations();
          }
        }

        if (currentOption == PLAY_FILES)
        {
          if (folderSelection)
          {
            int chosenFolder = chosenNumber - 1;
            if (chosenFolder >= 0 && chosenFolder < folderCount)
            {
              folderIndex = chosenFolder;
              currentSelection = folderIndex;
              firstVisibleLine = max(0, currentSelection - maxVisibleLines / 2);
              displayFolders();
            }
          }
          else
          {
            int chosenFile = chosenNumber - 1;
            if (chosenFile >= 0 && chosenFile < filesCount)
            {
              fileIndex = chosenFile;
              currentSelection = fileIndex;
              firstVisibleLine = max(0, currentSelection - maxVisibleLines / 2);
              displayFiles();
            }
          }
        }

        inputBuffer = "";
        inputActive = false;
      }
      else
      {
        IRokButton = true; // normalne działanie OK
      }
    }

    else
    {
      Serial.println("Nieznany przycisk.");
    }

    ir_code = 0;
    bit_count = 0;
  }
}


// -------------------------------------------------------------------
// Obsługa cyfr wpisywanych z pilota (dla wyboru stacji/pliku/folderu)
// -------------------------------------------------------------------
void handleDigitInput(int digit)
{
  if (!inputActive)
  {
    inputBuffer = "";
    inputActive = true;
    displayActive = true;
    displayStartTime = millis();
  }

  int maxDigits = 2;
  if ((currentOption == PLAY_FILES && filesCount > 99) || (currentOption == PLAY_FILES && folderCount > 99))
  {
    maxDigits = 3;
  }

  if (inputBuffer.length() < maxDigits)
  {
    inputBuffer += String(digit);
  }

  Serial.printf("Wprowadzono: %s\n", inputBuffer.c_str());

  // --- Wyświetlanie numeru na ekranie ---
  canvas.fillRect(0, 0, 480, 35, COLOR_BLACK);
  canvas.setFont(&FreeMonoBold12pt7b);

  if (currentOption == INTERNET_RADIO)
  {
    canvas.fillRect(0, 0, 480, 40, COLOR_BLACK);
    String header = "WYBIERZ STACJE 1-" + String(stationsCount);
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(0, 25);
    canvas.print(header);
  }
  else if (currentOption == PLAY_FILES && !folderSelection)
  {
    String header = "WYBIERZ PLIK 1-" + String(filesCount);
    canvas.setTextColor(COLOR_CYAN);
    canvas.setCursor(0, 25);
    canvas.print(header);
  }
  else if (currentOption == PLAY_FILES && folderSelection)
  {
    String header = "WYBIERZ FOLDER 1-" + String(folderCount);
    canvas.setTextColor(COLOR_CYAN);
    canvas.setCursor(0, 25);
    canvas.print(header);
  }

  // Pokaż wpisany numer w kolorze zielonym
  int16_t x1, y1;
  uint16_t w, h;
  canvas.getTextBounds("WYBIERZ", 0, 25, &x1, &y1, &w, &h);
  canvas.setTextColor(COLOR_GREEN);
  canvas.setCursor(w + 200, 25);
  canvas.print(inputBuffer);
  tft_pushCanvas(canvas);

  // Aktualizacja wskaźników
  if (currentOption == PLAY_FILES)
  {
    if (folderSelection)
      folderIndex = constrain(inputBuffer.toInt() - 1, 0, folderCount - 1);
    else
      fileIndex = constrain(inputBuffer.toInt() - 1, 0, filesCount - 1);
  }
}


// Funkcja sprawdza, czy plik jest plikiem audio na podstawie jego rozszerzenia
bool isAudioFile(const char *fileNameString)
{
  // Znajdź ostatnie wystąpienie kropki w nazwie pliku
  const char *ext = strrchr(fileNameString, '.');
  
  // Jeśli nie znaleziono kropki lub nie ma rozszerzenia, zwróć false
  if (!ext)
  {
    return false;
  }

  // Sprawdź rozszerzenie, ignorując wielkość liter
  return (strcasecmp(ext, ".mp3") == 0 ||  // MP3: popularny format stratny, szeroko stosowany
          strcasecmp(ext, ".wav") == 0 ||  // WAV: nieskompresowany format bezstratny, często używany w nagraniach
          strcasecmp(ext, ".flac") == 0 || // FLAC: bezstratna kompresja, wysoka jakość dźwięku
          strcasecmp(ext, ".aac") == 0 ||  // AAC: lepsza kompresja niż MP3, używany w iTunes
          strcasecmp(ext, ".wma") == 0 ||  // WMA: format stratny stworzony przez Microsoft
          strcasecmp(ext, ".ogg") == 0 ||  // OGG: otwarty format stratny, często używany z kodekiem Vorbis
          strcasecmp(ext, ".m4a") == 0 ||  // M4A: używany przez Apple, podobny do AAC
          strcasecmp(ext, ".aiff") == 0 || // AIFF: nieskompresowany format bezstratny, używany głównie na komputerach Apple
          strcasecmp(ext, ".alac") == 0);  // ALAC: bezstratny format od Apple, podobny do FLAC
}


// Wysłanie komendy do wyświetlacza TFT
void tft_command(uint8_t cmd)
{ 
  DC_COMMAND;         // Ustawienie linii DC na LOW (tryb komendy)
  CS_ACTIVE;          // Aktywacja wyświetlacza (CS LOW)
  spi.transfer(cmd);  // Wysłanie bajtu komendy przez SPI
  CS_IDLE;            // Dezaktywacja wyświetlacza (CS HIGH)
}


// Wysłanie danych do wyświetlacza TFT
void tft_data(uint8_t d)
{ 
  DC_DATA;            // Ustawienie linii DC na HIGH (tryb danych)
  CS_ACTIVE;          // Aktywacja wyświetlacza (CS LOW)
  spi.transfer(d);    // Wysłanie bajtu danych przez SPI
  CS_IDLE;            // Dezaktywacja wyświetlacza (CS HIGH)
}


// Ustawienie obszaru roboczego (okna) na wyświetlaczu TFT
void tft_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  tft_command(0x2A);                // Komenda: Column Address Set (ustawienie kolumn)
  DC_DATA;                           // Przełączenie w tryb danych
  CS_ACTIVE;                         // Aktywacja wyświetlacza (CS LOW)
  spi.transfer(x0 >> 8);             // Wysłanie starszego bajtu x0
  spi.transfer(x0 & 0xFF);           // Wysłanie młodszego bajtu x0
  spi.transfer(x1 >> 8);             // Wysłanie starszego bajtu x1
  spi.transfer(x1 & 0xFF);           // Wysłanie młodszego bajtu x1
  CS_IDLE;                            // Dezaktywacja wyświetlacza (CS HIGH)

  tft_command(0x2B);                // Komenda: Row Address Set (ustawienie wierszy)
  DC_DATA;                           // Przełączenie w tryb danych
  CS_ACTIVE;                         // Aktywacja wyświetlacza
  spi.transfer(y0 >> 8);             // Wysłanie starszego bajtu y0
  spi.transfer(y0 & 0xFF);           // Wysłanie młodszego bajtu y0
  spi.transfer(y1 >> 8);             // Wysłanie starszego bajtu y1
  spi.transfer(y1 & 0xFF);           // Wysłanie młodszego bajtu y1
  CS_IDLE;                            // Dezaktywacja wyświetlacza

  tft_command(0x2C);                // Komenda: Memory Write (rozpoczęcie wysyłania danych do wybranego okna)
}


// Inicjalizacja wyświetlacza
void tft_init()
{
  if (TFT_RST >= 0)
  {
    // Ustaw pin RESET jako wyjście
    pinMode(TFT_RST, OUTPUT);

    // Wykonaj sprzętowy reset ekranu
    digitalWrite(TFT_RST, LOW);
    delay(50);
    digitalWrite(TFT_RST, HIGH);
    delay(150);
  }

  // Programowy reset kontrolera
  tft_command(0x01);
  delay(120);

  // Sterowanie zasilaniem – Power Control 1
  tft_command(0xC0);
  tft_data(0x17); tft_data(0x15);

  // Sterowanie zasilaniem – Power Control 2
  tft_command(0xC1);
  tft_data(0x41);

  // Sterowanie VCOM (napięcia kontrastu)
  tft_command(0xC5);
  tft_data(0x00); tft_data(0x12); tft_data(0x80);

  // Kierunek i orientacja obrazu (MADCTL)
  tft_command(0x36);
  tft_data(0xE8);   // orientacja ekranu (rotacja, odbicie)

  // Format piksela – ustaw RGB666 (18 bitów na piksel)
  tft_command(0x3A);
  tft_data(0x66);

  // Tryb interfejsu
  tft_command(0xB0);
  tft_data(0x00);

  // Kontrola odświeżania (Frame Rate Control)
  tft_command(0xB1);
  tft_data(0x50);   // ok. 30Hz              tft_data(0xA0);   // ok. 60Hz

  // Kontrola inwersji wyświetlania
  tft_command(0xB4);
  tft_data(0x02);   // inwersja 2-punktowa

  // Kontrola funkcji wyświetlania (timingi, linie skanowania)
  tft_command(0xB6);
  tft_data(0x02); tft_data(0x02); tft_data(0x3B);

  // Tryb wejściowy
  tft_command(0xB7);
  tft_data(0xC6);

  // Regulacja (Adjust Control 3) – parametry wewnętrzne
  tft_command(0xF7);
  tft_data(0xA9); tft_data(0x51); tft_data(0x2C); tft_data(0x82);

  // Wyjście ze stanu uśpienia
  tft_command(0x11);
  delay(120);

  // Normalny tryb wyświetlania
  tft_command(0x13);

  // Włączenie wyświetlacza
  tft_command(0x29);
  delay(100);
}


// Funkcja wysyła zawartość całego canvasu do pamięci ekranu TFT i rysuje ją na wyświetlaczu
void tft_pushCanvas(GFXcanvas16 &c)
{
  tft_setAddrWindow(0,0,TFT_WIDTH-1,TFT_HEIGHT-1);  // Ustawienie obszaru ekranu do rysowania (cały ekran)
  DC_DATA; CS_ACTIVE;                               // Przełączenie linii DC na dane i aktywacja chip select
  static uint8_t line[TFT_WIDTH*3];                // Bufor dla jednej linii pikseli w formacie 18-bit RGB (3 bajty na piksel)

  for(int y=0; y<TFT_HEIGHT; y++)                 // Iteracja po każdej linii pionowej ekranu
  {
    for(int x=0; x<TFT_WIDTH; x++)                // Iteracja po każdym pikselu w linii
    {
      uint16_t color = c.getPixel(x,y);           // Pobranie koloru piksela z canvas (16-bit RGB565)

      // Konwersja z RGB565 (5-6-5) na RGB888 (8-8-8) dla transmisji do wyświetlacza ILI9488
      line[x*3]   = (color >> 8) & 0xF8;          // Składnik czerwony, 5 bitów -> 8 bitów
      line[x*3+1] = (color >> 3) & 0xFC;          // Składnik zielony, 6 bitów -> 8 bitów
      line[x*3+2] = (color << 3) & 0xF8;          // Składnik niebieski, 5 bitów -> 8 bitów
    }

    spi.writeBytes(line, sizeof(line));           // Wysyłanie całej linii pikseli do wyświetlacza
  }

  CS_IDLE;                                        // Dezaktywacja chip select po zakończeniu rysowania
}


/**
 * Wypełnia prostokąt na wyświetlaczu TFT kolorem RGB.
 *
 * @param x  współrzędna X lewego górnego rogu prostokąta
 * @param y  współrzędna Y lewego górnego rogu prostokąta
 * @param w  szerokość prostokąta w pikselach
 * @param h  wysokość prostokąta w pikselach
 * @param r  składowa koloru (czerwony, 0–255)
 * @param g  składowa koloru (zielony, 0–255)
 * @param b  składowa koloru (niebieski, 0–255)
 */
void tft_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b)
{
  // Ustaw obszar do rysowania (tzw. address window) – wszystkie dane SPI
  // wysłane w tym trybie zostaną wpisane wprost do prostokąta [x..x+w-1, y..y+h-1].
  tft_setAddrWindow(x, y, x + w - 1, y + h - 1);

  DC_DATA;   // Przełącz linię DC w tryb danych (a nie komend)
  CS_ACTIVE; // Aktywuj chip select (rozpoczęcie transmisji do kontrolera TFT)

  // Bufor dla jednej linii obrazu (każdy piksel = 3 bajty RGB)
  static uint8_t line[TFT_WIDTH * 3];

  // Wypełnij bufor kolorem: każdy piksel to (r,g,b)
  for (uint16_t i = 0; i < w; i++)
  {
    line[i * 3 + 0] = r; // czerwony
    line[i * 3 + 1] = g; // zielony
    line[i * 3 + 2] = b; // niebieski
  }

  // Wyślij kolejne linie do wyświetlacza
  for (uint16_t j = 0; j < h; j++)
  {
    // Przesyłamy dane jednej linii przez SPI (szerokość * 3 bajty RGB)
    spi.writeBytes(line, w * 3);

    // yield() oddaje sterowanie schedulerowi (ważne na ESP8266/ESP32,
    // żeby nie blokować całkowicie innych zadań)
    yield();
  }

  CS_IDLE; // Zakończ transmisję (odznacz chip select)
}


/**
 * Rysuje pojedynczy znak na ekranie TFT z wykorzystaniem czcionki GFX.
 *
 * @param font  wskaźnik do struktury czcionki (GFXfont)
 * @param x     pozycja X (początek znaku, lewy górny róg względem offsetu)
 * @param y     pozycja Y (pozycja bazowa znaku – linia odniesienia baseline)
 * @param c     znak do narysowania
 * @param r,g,b składowe koloru (RGB) w zakresie 0–255
 */
void drawCharFont(const GFXfont *font, int16_t x, int16_t y, char c, uint8_t r, uint8_t g, uint8_t b)
{
  // Sprawdź, czy znak mieści się w zakresie obsługiwanym przez czcionkę
  if (c < font->first || c > font->last)
    return;

  // Pobierz informacje o znaku (szerokość, wysokość, offsety, bitmapOffset)
  GFXglyph *glyph = &font->glyph[c - font->first];

  // Wskaźnik do danych bitmapy znaku
  const uint8_t *bitmap = font->bitmap + glyph->bitmapOffset;

  // Wymiary znaku w pikselach
  int w = glyph->width;
  int h = glyph->height;

  // Pozycja znaku na ekranie (z uwzględnieniem przesunięcia offset)
  int16_t xPos = x + glyph->xOffset;
  int16_t yPos = y + glyph->yOffset;

  // Ustawienie obszaru rysowania (window) raz dla całego znaku
  // zamiast ustawiać dla każdego piksela – to znacznie przyspiesza rysowanie
  tft_setAddrWindow(xPos, yPos, xPos + w - 1, yPos + h - 1);

  int bit = 0;       // licznik bitów
  uint8_t bits = 0;  // aktualny bajt z bitmapy (8 pikseli)

  // Iteracja po wysokości znaku
  for (int yy = 0; yy < h; yy++)
  {
    // Iteracja po szerokości znaku
    for (int xx = 0; xx < w; xx++)
    {
      // Co 8 pikseli wczytujemy nowy bajt z bitmapy
      if (!(bit & 7))
      {
        bits = *bitmap++;
      }

      // Najbardziej znaczący bit określa, czy piksel ma być rysowany (1) czy pusty (0)
      if (bits & 0x80)
      {
        // Piksel w kolorze podanym w RGB
        tft_data(r);
        tft_data(g);
        tft_data(b);
      }
      else
      {
        // Piksel "tła" (czarny)
        tft_data(0);
        tft_data(0);
        tft_data(0);
      }

      // Przesunięcie bitów w lewo, aby sprawdzić następny piksel
      bits <<= 1;
      bit++;
    }
  }
}


// Rysowanie napisu dla dowolnej czcionki
void drawStringFont(const GFXfont* font, int16_t x, int16_t y, const char* str, uint8_t r, uint8_t g, uint8_t b)
{
  int16_t cursorX = x;  // Początkowa pozycja kursora w osi X

  while (*str)  // Pętla przechodzi po wszystkich znakach w napisie
  {
    uint8_t c = *str++;  // Pobierz aktualny znak i przesuwaj wskaźnik na następny

    if (c < font->first || c > font->last)
      continue;  // Pomijaj znaki, których czcionka nie obsługuje (spoza zakresu)

    // Pobierz dane glifu (kształtu znaku) z czcionki
    GFXglyph *glyph = &font->glyph[c - font->first];

    // Narysuj pojedynczy znak w zadanej pozycji i kolorze
    // drawCharFont rysuje znak bez dodatkowego przesunięcia yOffset
    drawCharFont(font, cursorX, y, c, r, g, b);

    // Przesuń kursor w prawo o szerokość znaku (xAdvance),
    // aby kolejny znak pojawił się obok
    cursorX += glyph->xAdvance;
  }
}


// Funkcja do pobierania danych z API z serwera pogody openweathermap.org
void getWeatherData()
{
  weatherServerConnection = false;
  HTTPClient http;  // Utworzenie obiektu HTTPClient
  
  // Poniżej zdefiniuj swój unikalny URL zawierający dane lokalizacji wraz z kluczem API otrzymany po resetracji w serwisie openweathermap.org
  // String url = "http://api.openweathermap.org/data/2.5/weather?q=Piła,pl&appid=your_own_API_key";
  String url = "http://api.openweathermap.org/data/2.5/weather?q=Piła,pl&appid=cbc705bd4e66cb3422111f1533a78355";

  http.begin(url);  // Inicjalizacja połączenia HTTP z podanym URL-em, otwieramy połączenie z serwerem.

  int httpCode = http.GET();  // Wysłanie żądanie GET do serwera, aby pobrać dane pogodowe

  if (httpCode == HTTP_CODE_OK)  // Sprawdzenie, czy odpowiedź z serwera była prawidłowa (kod 200 OK)
  {
    weatherServerConnection = true;
    String payload = http.getString();  // Pobranie odpowiedzi z serwera w postaci ciągu znaków (JSON)
    Serial.println("Odpowiedź JSON z API:");
    Serial.println(payload); 

    DeserializationError error = deserializeJson(doc, payload);  // Deserializujemy dane JSON do obiektu dokumentu
    if (error)  // Sprawdzamy, czy deserializacja JSON zakończyła się niepowodzeniem
    {
      Serial.print(F("deserializeJson() failed: "));  // Jeśli jest błąd, drukujemy komunikat o błędzie
      Serial.println(error.f_str());  // Wydruk szczegółów błędu deserializacji
      return;  // Zakończenie funkcji w przypadku błędu
    }

    updateWeather();  // Jeśli deserializacja zakończyła się sukcesem, wywołujemy funkcję `updateWeather`, aby zaktualizować wyświetlacz i serial terminal
  }
  else  // Jeśli połączenie z serwerem nie powiodło się
  {
    weatherServerConnection = false;
    Serial.println("Błąd połączenia z serwerem.");  
  }
  
  http.end();  // Zakończenie połączenia HTTP, zamykamy zasoby
}


// Funkcja do aktualizacji danych pogodowych
void updateWeather()
{
  JsonObject root = doc.as<JsonObject>();  // Konwertuje dokument JSON do obiektu typu JsonObject

  JsonObject main = root["main"];  // Pobiera obiekt "main" zawierający dane główne, takie jak temperatura, wilgotność, ciśnienie
  JsonObject weather = root["weather"][0];  // Pobiera pierwszy element z tablicy "weather", który zawiera dane o pogodzie
  JsonObject wind = root["wind"];  // Pobiera obiekt "wind" zawierający dane o wietrze

  unsigned long timestamp = root["dt"];  // Pobiera timestamp (czas w sekundach) z JSON
  String formattedDate = convertTimestampToDate(timestamp);  // Konwertuje timestamp na sformatowaną datę i godzinę

  float temp = main["temp"].as<float>() - 273.15;  // Pobiera temperaturę w Kelvinach i konwertuje ją na °C
  float feels_like = main["feels_like"].as<float>() - 273.15;  // Pobiera odczuwalną temperaturę i konwertuje ją na °C

  int humidity = main["humidity"];  // Pobiera wilgotność powietrza
  String weatherDescription = weather["description"].as<String>();  // Pobiera opis pogody (np. "light rain")
  String icon = weather["icon"].as<String>();  // Pobiera kod ikony pogody (np. "10d" dla deszczu)
  float windSpeed = wind["speed"];  // Pobiera prędkość wiatru w m/s
  float windGust = wind["gust"];  // Pobiera prędkość podmuchów wiatru w m/s
  float pressure = main["pressure"].as<float>();  // Pobiera ciśnienie powietrza w hPa

  Serial.println("Dane z JSON:");
  Serial.print("Data ");
  Serial.println(formattedDate);
  Serial.print("Temperatura ");
  Serial.print(temp, 2);
  Serial.println(" °C");
  tempStr = "Temperatura " + String(temp, 2) + " C";
  
  Serial.print("Odczuwalna temperatura ");
  Serial.print(feels_like, 2);
  Serial.println(" °C");
  feels_likeStr = "Odczuwalna " + String(feels_like, 2) + " C";
  
  Serial.print("Wilgotność ");
  Serial.print(humidity);
  Serial.println(" %");
  humidityStr = "Wilgotnosc " + String(humidity) + " %";
  
  Serial.print("Ciśnienie ");
  Serial.print(pressure);
  Serial.println(" hPa");
  pressureStr = "Cisnienie " + String(pressure, 2) + " hPa";
  
  Serial.print("Opis pogody ");
  Serial.println(weatherDescription);
  Serial.print("Ikona ");
  Serial.println(icon);
  
  Serial.print("Predkosc wiatru ");
  Serial.print(windSpeed, 2);
  Serial.println(" m/s");
  windStr = "Wiatr " + String(windSpeed) + " m/s";
  
  Serial.print("Porywy wiatru ");
  Serial.print(windGust, 2);
  Serial.println(" m/s");
  windGustStr = "W porywach " + String(windGust) + " m/s";
}


// Funkcja do ustawienia głośności na żądaną wartość
void volumeSet()
{
  displayActive = true;  // Ustawienie flagi aktywności wyświetlacza
  displayStartTime = millis();  // Zapisanie czasu rozpoczęcia wyświetlania
  
  // Sprawdzenie, czy volumeValue mieści się w zakresie 0-21
  if (volumeValue < 0) volumeValue = 0;
  if (volumeValue > 21) volumeValue = 21;

  Serial.print("Wartość głośności: ");
  Serial.println(volumeValue);
  
  // Ustawienie głośności
  audio.setVolume(volumeValue); // dopuszczalny zakres 0...21

  // --- Wyczyść tylko obszar głośności na canvas ---
  canvas.fillRect(40, 265, 50, 25, COLOR_BLACK);

  // Przygotowanie napisu głośności
  volumeDisplay = "VOL " + String(volumeValue);

  // Czcionka, kolor i pozycja
  canvas.setFont(&FreeMonoBold12pt7b);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setCursor(5, 280);

  // Rysowanie napisu na canvas
  canvas.print(volumeDisplay);

  // Wyślij canvas na ekran TFT
  tft_pushCanvas(canvas);


  if (currentOption == INTERNET_RADIO)
  {
    saveVolumeSettings(station_nr, volumeValue, bank_nr);
  }
}


// Obsługa regulacji głośności z pilota zdalnego sterowania
void volumeSetFromRemote()
{
  if (IRvolumeUp == true)  // Przycisk VOL+ w pilocie
  {
    IRvolumeUp = false;
    volumeValue++;
    if (volumeValue > 18)
    {
      volumeValue = 18;
    }
    volumeSet();
  }

  if (IRvolumeDown == true)  // Przycisk VOL- w pilocie
  {
    IRvolumeDown = false;
    volumeValue--;
    if (volumeValue < 3)
    {
      volumeValue = 3;
    }
    volumeSet();
  }
}


// Zapisywanie na karcie SD wartości głośności dla wybranej stacji z aktualnego banku
void saveVolumeSettings(int station, int volume, int bank)
{
  // Sprawdzenie czy stacja mieści się w zakresie
  if (station < 0 || station >= 100)
  {
    Serial.println("Błąd: Indeks stacji poza zakresem.");
    return;
  }

  // Generowanie nazwy pliku na podstawie numeru banku (np. /volume_bank01.txt)
  String fileNameString = "/volume_bank";
  if (bank < 10)
  {
    fileNameString += "0";  // Dodajemy '0', jeśli numer banku jest mniejszy niż 10
  }
  fileNameString += String(bank) + ".txt";

  // Otwieramy plik do zapisu
  File file = SD.open(fileNameString, FILE_WRITE);
  if (file)
  {
    // Aktualizujemy tylko wartość dla danej stacji (station_nr - 1, aby użyć właściwego indeksu tablicy)
    volumeArray[station - 1] = volume;  // Zapisywanie głośności dla wybranej stacji

    // Zapisujemy całą tablicę głośności do pliku
    for (int i = 0; i < 100; i++)
    {
      file.print(volumeArray[i]);
      if (i < 99)
      {
        file.print(", ");
      }
      else
      {
        file.println(volumeArray[i]);
      }
    }

    file.close();
    Serial.println("Ustawienia głośności zostały zapisane.");

    // Drukowanie tablicy po zapisie
    Serial.println("Tablica głośności po zapisie:");
    for (int i = 0; i < 100; i++)
    {
      Serial.print(volumeArray[i]);
      Serial.print(", ");
    }
    Serial.println();
  }
  else
  {
    Serial.println("Błąd otwierania pliku do zapisu.");
  }
}


// Ładowanie zapisanych na karcie SD wartości głośności dla wybranej stacji z aktualnego banku
void loadVolumeSettings(int station, int bank)
{
  // Sprawdzamy, czy indeks stacji jest w zakresie
  if (station < 1 || station > 100)
  { 
    Serial.println("Błąd: Indeks stacji poza zakresem.");
    return;
  }

  // Generowanie nazwy pliku na podstawie numeru banku (np. /volume_bank01.txt)
  String fileNameString = "/volume_bank";
  if (bank < 10)
  {
    fileNameString += "0";  // Dodajemy '0', jeśli numer banku jest mniejszy niż 10
  }
  fileNameString += String(bank) + ".txt";

  // Sprawdzamy, czy plik z ustawieniami głośności istnieje
  if (SD.exists(fileNameString))
  {
    Serial.println("Plik głośności istnieje, próbuję otworzyć...");

    // Otwieramy plik do odczytu
    File file = SD.open(fileNameString, FILE_READ);
    if (file)
    {
      Serial.println("Ustawienia głośności wczytane.");

      int i = 0;
      while (file.available())
      {
        String line = file.readStringUntil(',');  // Czytamy do przecinka
        int volume = line.toInt();  // Przekształcamy string na int

        if (i < 100)
        {
          // Sprawdzenie aby zmieścić się w min / max głośności
          if (volume < 0) volume = 0;
          if (volume > 21) volume = 21;
          volumeArray[i] = volume;  // Zapisujemy odczytane wartości do tablicy
        }
        i++;
      }
      file.close();

      // Ustawiamy głośność dla wybranej stacji
      volumeValue = volumeArray[station - 1];
      Serial.print("Głośność dla stacji ");
      Serial.print(station);
      Serial.print(" została ustawiona na: ");
      Serial.println(volumeValue);
      // Ustawienie głośności
      audio.setVolume(volumeValue); // dopuszczalny zakres 0...21
    }
    else
    {
      Serial.println("Błąd otwierania pliku do odczytu.");
    }
  }
  else
  {
    Serial.println("Plik głośności nie istnieje.");
    
    // Tworzymy nowy plik z domyślnymi wartościami 12 dla wszystkich stacji
    Serial.println("Tworzenie nowego pliku z wartościami 12 dla wszystkich stacji...");

    File file = SD.open(fileNameString, FILE_WRITE);
    if (file)
    {
      // Ustawiamy wszystkie elementy tablicy na 12
      for (int i = 0; i < 100; i++)
      {
        volumeArray[i] = 12;  // Domyślna wartość głośności
        file.print(volumeArray[i]);
        if (i < 99)
        {
          file.print(", ");
        }
      }
      file.close();
      Serial.println("Plik z wartościami głośności 12 został zapisany.");

      // Ustawiamy głośność dla wybranej stacji
      volumeValue = volumeArray[station - 1];  // Pobieramy głośność dla wybranej stacji
      Serial.print("Głośność dla stacji ");
      Serial.print(station);
      Serial.print(" została ustawiona na: ");
      Serial.println(volumeValue);
      // Ustawienie głośności
      audio.setVolume(volumeValue); // dopuszczalny zakres 0...21
    }
    else
    {
      Serial.println("Błąd podczas tworzenia pliku.");
    }
  }

  // Drukowanie tablicy po odczycie
  Serial.println("Tablica głośności po odczycie:");
  for (int i = 0; i < 100; i++)
  {
    Serial.print(volumeArray[i]);
    Serial.print(", ");
  }
  Serial.println();
}


// Funkcja konwertująca timestamp na datę i godzinę w formacie "YYYY-MM-DD HH:MM:SS"
String convertTimestampToDate(unsigned long timestamp)  
{
  int year, month, day, hour, minute, second;  // Deklaracja zmiennych dla roku, miesiąca, dnia, godziny, minuty i sekundy z pogodynki
  time_t rawTime = timestamp;                  // Konwersja timestamp na typ time_t, który jest wymagany przez funkcję localtime()
  struct tm* timeInfo;                         // Wskaźnik na strukturę tm, która zawiera informacje o czasie
  timeInfo = localtime(&rawTime);              // Konwertowanie rawTime na strukturę tm zawierającą szczegóły daty i godziny

  year = timeInfo->tm_year + 1900;             // Rok jest liczony od 1900 roku, więc musimy dodać 1900
  month = timeInfo->tm_mon + 1;                // Miesiąc jest indeksowany od 0, więc dodajemy 1
  day = timeInfo->tm_mday;                     // Dzień miesiąca
  hour = timeInfo->tm_hour;                    // Godzina (0-23)
  minute = timeInfo->tm_min;                   // Minuta (0-59)
  second = timeInfo->tm_sec;                   // Sekunda (0-59)

  // Formatowanie na dwie cyfry (dodawanie zer na początku, jeśli liczba jest mniejsza niż 10)
  String strMonth = (month < 10) ? "0" + String(month) : String(month);            // Dodaje zero przed miesiącem, jeśli miesiąc jest mniejszy niż 10
  String strDay = (day < 10) ? "0" + String(day) : String(day);                    // Dodaje zero przed dniem, jeśli dzień jest mniejszy niż 10
  String strHour = (hour < 10) ? "0" + String(hour) : String(hour);                // Dodaje zero przed godziną, jeśli godzina jest mniejsza niż 10
  String strMinute = (minute < 10) ? "0" + String(minute) : String(minute);        // Dodaje zero przed minutą, jeśli minuta jest mniejsza niż 10
  String strSecond = (second < 10) ? "0" + String(second) : String(second);        // Dodaje zero przed sekundą, jeśli sekunda jest mniejsza niż 10

  // Tworzenie sformatowanej daty w formacie "YYYY-MM-DD HH:MM:SS"
  String date = String(year) + "-" + strMonth + "-" + strDay + " " + strHour + ":" + strMinute + ":" + strSecond;
                
  return date;  // Zwraca sformatowaną datę jako String
}


// Funkcja do pobierania i wyciągania danych kalendarzowych z HTML poniższego adresu URL
void fetchAndDisplayCalendar()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    
    // Adres URL do pobrania danych z kalendarza
    const char* serverName = "https://www.kalendarzswiat.pl/dzisiaj";

    // Wysyłamy żądanie GET do serwera
    http.begin(serverName);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)  // 200 = OK
    {
      // Otrzymujemy odpowiedź w formie string
      String payload = http.getString();
      
      // Wyciągamy nazwę dnia tygodnia
      int startWeekday = payload.indexOf("<span class=\"weekday\">") + 21;
      int endWeekday = payload.indexOf("</span>", startWeekday);
      String weekday = payload.substring(startWeekday, endWeekday);
      weekday.trim();  // Usuwamy zbędne znaki

      // Wyciągamy numer dnia
      int startDay = payload.indexOf("<span class=\"day\">") + 18;
      int endDay = payload.indexOf("</span>", startDay);
      String day = payload.substring(startDay, endDay);
      day.trim();  // Usuwamy zbędne znaki

      // Wyciągamy nazwę miesiąca
      int startMonth = payload.indexOf("<span class=\"month-name\">") + 24;
      int endMonth = payload.indexOf("</span>", startMonth);
      String month = payload.substring(startMonth, endMonth);
      month.trim();  // Usuwamy zbędne znaki

      // Wyciągamy rok
      int startYear = payload.indexOf("<span class=\"year\">") + 19;
      int endYear = payload.indexOf("</span>", startYear);
      String year = payload.substring(startYear, endYear);
      year.trim();  // Usuwamy zbędne znaki

      // Wyciąganie imienin
      int startImieniny = payload.indexOf("<p class=\"namedays\">") + 20;
      int endImieniny = payload.indexOf("</p>", startImieniny);
      namedays = payload.substring(startImieniny, endImieniny);
      namedays.trim();  // Usuwamy zbędne znaki

      // Wyciąganie ważnych wydarzeń
      int startSwieta = payload.indexOf("<h3>Święta i ważne wydarzenia</h3>") + 34;
      int endSwieta = payload.indexOf("</ul>", startSwieta);
      String swieta = payload.substring(startSwieta, endSwieta);

      // Szukamy wszystkich Świąt i ważnych wydarzeń
      String allHolidays = "";
      int startStrongIndex = swieta.indexOf("<strong>");
      while (startStrongIndex != -1)
      {
        int endStrongIndex = swieta.indexOf("</strong>", startStrongIndex);  // Końcowy znacznik </strong>
        String event = swieta.substring(startStrongIndex + 8, endStrongIndex);  // Wyciągamy tekst wewnątrz <strong>
        allHolidays += event + "\n";  // Dodajemy wydarzenie do listy
        startStrongIndex = swieta.indexOf("<strong>", endStrongIndex);  // Szukamy kolejnego <strong>
      }

      // Wyciąganie wschodu słońca
      int startWschod = payload.indexOf("id=\"sunrise_time\">") + 18;
      int endWschod = payload.indexOf("</b>", startWschod);
      sunrise = payload.substring(startWschod, endWschod);
      sunrise.trim();  // Usuwamy zbędne znaki

      // Wyciąganie zachodu słońca
      int startZachod = payload.indexOf("id=\"sunset_time\">") + 17;
      int endZachod = payload.indexOf("</b>", startZachod);
      sunset = payload.substring(startZachod, endZachod);
      sunset.trim();  // Usuwamy zbędne znaki

      // Wyciąganie długości dnia
      int startDlugosc = payload.indexOf("id=\"day_duration\">") + 18;
      int endDlugosc = payload.indexOf("</strong>", startDlugosc);
      dayLength = payload.substring(startDlugosc, endDlugosc);
      dayLength.trim();  // Usuwamy zbędne znaki

      // Szukamy przysłów, uwzględniając oba możliwe nagłówki: "Przysłowia na dziś" i "Przysłowie na dziś"
      String allProverbs = "";
      int startProverbs = payload.indexOf("<h3>Przysłowia na dziś</h3>");
      if (startProverbs == -1)
      {
        startProverbs = payload.indexOf("<h3>Przysłowie na dziś</h3>");  // Sprawdzamy alternatywny nagłówek
      }
      if (startProverbs != -1)
      {
        startProverbs += 26;  // +26, aby przejść za nagłówek
        int endProverbs = payload.indexOf("</h3>", startProverbs);
        String proverbs = payload.substring(startProverbs, endProverbs);
        proverbs.trim();  // Usuwamy zbędne znaki

        // Szukamy wszystkich przysłów
        int startProverbIndex = payload.indexOf("<p class=\"section\">", startProverbs);
        while (startProverbIndex != -1)
        {
          int endProverbIndex = payload.indexOf("</p>", startProverbIndex);
          String proverb = payload.substring(startProverbIndex + 19, endProverbIndex); // 19 to długość "<p class=\"section\">"
          allProverbs += proverb + "\n";  // Dodajemy przysłowie do listy
          startProverbIndex = payload.indexOf("<p class=\"section\">", endProverbIndex);  // Szukamy kolejnego przysłowia
        }
      }

      // Wyświetlamy dane na Serial Monitorze
      calendar = weekday + ", " + day + " " + month + " " + year;
      calendar.replace(">", "");  // Usuwamy wszystkie znaki '>'
      
      Serial.println("Dzis jest: " + calendar);
      Serial.println("Wschód słońca: " + sunrise);
      Serial.println("Zachód słońca: " + sunset);
      Serial.println("Długość dnia: " + dayLength);
      Serial.println("Imieniny: " + namedays);

      Serial.println("Święta i ważne wydarzenia:");
      Serial.println(allHolidays);

      Serial.println("Przysłowia na dziś:");
      Serial.println(allProverbs);

    }

    else
    {
      Serial.print("Błąd w zapytaniu HTTP. Kod błędu: ");
      Serial.println(httpResponseCode);
    }

    // Zamykanie połączenia HTTP
    http.end();
  }
  else
  {
    Serial.println("Brak połączenia z WiFi.");
  }
}

// Zamiana polskich znaków na "bezogonkowe"
String normalizePolish(String s)
{
  s.replace("ą","a"); s.replace("ć","c"); s.replace("ę","e");
  s.replace("ł","l"); s.replace("ń","n"); s.replace("ó","o");
  s.replace("ś","s"); s.replace("ż","z"); s.replace("ź","z");
  s.replace("Ą","A"); s.replace("Ć","C"); s.replace("Ę","E");
  s.replace("Ł","L"); s.replace("Ń","N"); s.replace("Ó","O");
  s.replace("Ś","S"); s.replace("Ż","Z"); s.replace("Ź","Z");
  return s;
}


// Funkcja wyświetla tekst w canvas linia po linii, automatycznie zawijając słowa, aby nie przekroczyły maksymalnej szerokości
// Parametry:
//   str       - wskaźnik na tekst do wyświetlenia
//   x, y      - początkowe współrzędne kursora w canvas
//   maxWidth  - maksymalna szerokość linii w pikselach
//   lineHeight- wysokość pojedynczej linii tekstu w pikselach (odstęp między liniami)
void drawWrappedCanvasText(const char* str, int16_t x, int16_t y, int16_t maxWidth, int16_t lineHeight)
{
  String tekst = normalizePolish(String(str));  // Zamiana polskich znaków i konwersja na String
  tekst.trim();                                 // Usunięcie białych znaków na początku i końcu

  int16_t cursorY = y;                          // Pozycja Y kursora w canvas

  while (tekst.length() > 0)
  {
    String line = tekst;
    int cut = line.length();

    // Dopasowanie długości linii do maksymalnej szerokości
    while (line.length() > 0)
    {
      int16_t w = 0;
      for (int i = 0; i < line.length(); i++)
      {
        char c = line[i];
        if (c < FreeSans12pt7b.first || c > FreeSans12pt7b.last)
          continue;

        GFXglyph *glyph = &FreeSans12pt7b.glyph[c - FreeSans12pt7b.first];
        w += glyph->xAdvance;  // Dodaj szerokość znaku
      }

      if (w <= maxWidth)
        break;  // Linia mieści się w szerokości

      int lastSpace = line.lastIndexOf(' ');  // Znajdź ostatnią spację, aby nie ciąć słów
      if (lastSpace < 0) break;
      line = line.substring(0, lastSpace);
    }

    // Rysowanie linii w canvas
    canvas.print(line);  
    cursorY += lineHeight + 5;  // Przesunięcie kursora w dół o wysokość linii + odstęp
    canvas.setCursor(x, cursorY);

    tekst = tekst.substring(line.length());  // Usunięcie już wyświetlonej części tekstu
    tekst.trim();                            // Usunięcie białych znaków na początku nowej linii
  }
}


// Inicjalizacja karty SD wraz z pierwszyn utworzeniem wymaganych plików w głównym katalogu karty, jesli pliki już istnieją funkcja sprawdza ich obecność
void SDinit()
{
  if (!SD.begin(SD_CS, customSPI))
  {
    Serial.println("Błąd inicjalizacji karty SD!");
    return;
  }
  Serial.println("Karta SD zainicjalizowana pomyślnie.");

  // Sprawdzanie pojemności i zajęctości karty SD
  unsigned long totalSpace = SD.cardSize() / (1024 * 1024);  // Całkowita pojemność karty w MB
  unsigned long usedSpace = SD.usedBytes() / (1024 * 1024);   // Użyta przestrzeń w MB
  unsigned long freeSpace = totalSpace - usedSpace;  // Wolna przestrzeń w MB

  Serial.print("Całkowita pojemność karty SD: ");
  Serial.print(totalSpace);
  Serial.println(" MB");

  Serial.print("Użyte miejsce: ");
  Serial.print(usedSpace);
  Serial.println(" MB");

  Serial.print("Wolne miejsce: ");
  Serial.print(freeSpace);
  Serial.println(" MB");

  // Sprawdzenie i ewentualne utworzenie plików
  if (!SD.exists("/station_nr.txt"))
  {
    // Plik station_nr.txt nie istnieje, tworzymy go i zapisujemy wartość 9
    File stationFile = SD.open("/station_nr.txt", FILE_WRITE);
    if (stationFile)
    {
      stationFile.println("9");
      stationFile.close();
      Serial.println("Plik station_nr.txt został utworzony.");
    }
    else
    {
      Serial.println("Błąd podczas tworzenia pliku station_nr.txt!");
    }
  }
  else
  {
    Serial.println("Plik station_nr.txt już istnieje.");
  }

  if (!SD.exists("/bank_nr.txt"))
  {
    // Plik bank_nr.txt nie istnieje, tworzymy go i zapisujemy wartość 1
    File bankFile = SD.open("/bank_nr.txt", FILE_WRITE);
    if (bankFile)
    {
      bankFile.println("1");
      bankFile.close();
      Serial.println("Plik bank_nr.txt został utworzony.");
    }
    else
    {
      Serial.println("Błąd podczas tworzenia pliku bank_nr.txt!");
    }
  }
  else
  {
    Serial.println("Plik bank_nr.txt już istnieje.");
  }
}


// Funkcja do pobierania listy stacji radiowych z serwera i zapisania ich w wybranym banku na karcie SD
void fetchStationsFromServer()
{
  previous_bank_nr = bank_nr;
  
  // Utwórz obiekt klienta HTTP
  HTTPClient http;

  // URL stacji dla danego banku
  String url;

  // Wszystkie URL-e zapisane w tablicy
  const char* STATIONS_URLS[] = {
    STATIONS_URL1, STATIONS_URL2, STATIONS_URL3, STATIONS_URL4,
    STATIONS_URL5, STATIONS_URL6, STATIONS_URL7, STATIONS_URL8,
    STATIONS_URL9, STATIONS_URL10, STATIONS_URL11, STATIONS_URL12,
    STATIONS_URL13, STATIONS_URL14, STATIONS_URL15, STATIONS_URL16,
    STATIONS_URL17, STATIONS_URL18
  };

  // Wybór adresu z tablicy
  if (bank_nr >= 1 && bank_nr <= 18)
  {
    url = STATIONS_URLS[bank_nr - 1];   // Indeksy zaczynają się od 0
  }
  else
  {
    Serial.println("Nieprawidłowy numer banku");
    return;
  }

  // Tworzenie nazwy pliku dla danego banku
  String fileNameWithBank = String("/bank") + (bank_nr < 10 ? "0" : "") + String(bank_nr) + ".txt";
  
  // Sprawdzenie, czy plik istnieje
  if (SD.exists(fileNameWithBank))
  {
    Serial.println("Plik banku " + fileNameWithBank + " już istnieje.");
  }
  else
  {
    // Próba utworzenia pliku, jeśli nie istnieje
    File bankFile = SD.open(fileNameWithBank, FILE_WRITE);
    
    if (bankFile)
    {
      Serial.println("Utworzono plik banku: " + fileNameWithBank);
      bankFile.close();  // Zamykanie pliku po utworzeniu
    }
    else
    {
      Serial.println("Błąd: Nie można utworzyć pliku banku: " + fileNameWithBank);
      return;  // Przerwij dalsze działanie, jeśli nie udało się utworzyć pliku
    }
  }

  // Inicjalizuj żądanie HTTP do podanego adresu URL
  http.begin(url);

  // Wykonaj żądanie GET i zapisz kod odpowiedzi HTTP
  int httpCode = http.GET();

  // Wydrukuj dodatkowe informacje diagnostyczne
  Serial.print("Kod odpowiedzi HTTP: ");
  Serial.println(httpCode);

  // Sprawdź, czy żądanie było udane (HTTP_CODE_OK)
  if (httpCode == HTTP_CODE_OK)
  {
    // Pobierz zawartość odpowiedzi HTTP w postaci tekstu
    String payload = http.getString();
    //Serial.println("Stacje pobrane z serwera:");
    //Serial.println(payload);  // Wyświetlenie pobranych danych (payload)

    // Otwórz plik w trybie zapisu, aby zapisać payload
    File bankFile = SD.open(fileNameWithBank, FILE_WRITE);
    if (bankFile)
    {
      bankFile.println(payload);  // Zapisz dane do pliku
      bankFile.close();  // Zamknij plik po zapisaniu
      Serial.println("Dane zapisane do pliku: " + fileNameWithBank);
    }
    else
    {
      Serial.println("Błąd: Nie można otworzyć pliku do zapisu: " + fileNameWithBank);
    }

    // Zapisz każdą niepustą stację do pamięci EEPROM z indeksem
    int startIndex = 0;
    int endIndex;
    stationsCount = 0;

    // Przeszukuj otrzymaną zawartość w poszukiwaniu nowych linii
    while ((endIndex = payload.indexOf('\n', startIndex)) != -1 && stationsCount < MAX_STATIONS)
    {
      // Wyodrębnij pojedynczą stację z otrzymanego tekstu
      String station = payload.substring(startIndex, endIndex);
      
      // Sprawdź, czy stacja nie jest pusta, a następnie przetwórz i zapisz
      if (!station.isEmpty())
      {
        // Zapisz stację do pliku na karcie SD
        sanitizeAndSaveStation(station.c_str());
      }
      
      // Przesuń indeks początkowy do kolejnej linii
      startIndex = endIndex + 1;
    }
  }
  else
  {
    // W przypadku nieudanego żądania wydrukuj informację o błędzie z kodem HTTP
    Serial.printf("Błąd podczas pobierania stacji. Kod HTTP: %d\n", httpCode);
  }

  // Zakończ połączenie HTTP
  http.end();
}


// Funkcja przetwarza i zapisuje stację do pamięci EEPROM
void sanitizeAndSaveStation(const char* station)
{
  // Bufor na przetworzoną stację - o jeden znak dłuższy niż maksymalna długość linku
  char sanitizedStation[STATION_NAME_LENGTH + 1];
  
  // Indeks pomocniczy dla przetwarzania
  int j = 0;

  // Przeglądaj każdy znak stacji i sprawdź czy jest to drukowalny znak ASCII
  for (int i = 0; i < STATION_NAME_LENGTH && station[i] != '\0'; i++)
  {
    // Sprawdź, czy znak jest drukowalnym znakiem ASCII
    if (isprint(station[i]))
    {
      // Jeśli tak, dodaj do przetworzonej stacji
      sanitizedStation[j++] = station[i];
    }
  }

  // Dodaj znak końca ciągu do przetworzonej stacji
  sanitizedStation[j] = '\0';

  // Zapisz przetworzoną stację do pamięci EEPROM
  saveStationToEEPROM(sanitizedStation);
}


//Funkcja odpowiedzialna za zapisywanie informacji o stacji do pamięci EEPROM.
void saveStationToEEPROM(const char* station)
{   
  // Sprawdź, czy istnieje jeszcze miejsce na kolejną stację w pamięci EEPROM.
  if (stationsCount < MAX_STATIONS)
  {
    int length = strlen(station);

    // Sprawdź, czy długość linku nie przekracza ustalonego maksimum.
    if (length <= STATION_NAME_LENGTH)
    {
      // Zapisz długość linku jako pierwszy bajt.
      EEPROM.write(stationsCount * (STATION_NAME_LENGTH + 1), length);

      // Zapisz link jako kolejne bajty w pamięci EEPROM.
      for (int i = 0; i < length; i++)
      {
        EEPROM.write(stationsCount * (STATION_NAME_LENGTH + 1) + 1 + i, station[i]);
      }

      // Potwierdź zapis do pamięci EEPROM - czasowe wyłączenie na testy
      //EEPROM.commit();

      // Wydrukuj informację o zapisanej stacji na Serialu.
      Serial.println(String(stationsCount + 1) + "   " + String(station)); // Drukowanie na serialu od nr 1 jak w banku na serwerze

      // Zwiększ licznik zapisanych stacji.
      stationsCount++;
    } 
    else
    {
      // Informacja o błędzie w przypadku zbyt długiego linku do stacji.
      Serial.println("Błąd: Link do stacji jest zbyt długi");
    }
  }
  else
  {
    // Informacja o błędzie w przypadku osiągnięcia maksymalnej liczby stacji.
    Serial.println("Błąd: Osiągnięto maksymalną liczbę zapisanych stacji");
  }
}


// Funkcja do odczytywania numeru stacji i numeru banku z karty SD
void readStationFromSD()
{
  // Sprawdź, czy karta SD jest dostępna
  if (!SD.begin(47))
  {
    Serial.println("Nie można znaleźć karty SD. Ustawiam domyślne wartości.");
    station_nr = 9; // Domyślny numer stacji gdy brak karty SD
    bank_nr = 1;  // Domyślny numer banku gdy brak karty SD
    return;
  }

  // Sprawdź, czy plik station_nr.txt istnieje
  if (SD.exists("/station_nr.txt"))
  {
    myFile = SD.open("/station_nr.txt");
    if (myFile)
    {
      station_nr = myFile.parseInt();
      myFile.close();
      Serial.print("Wczytano station_nr z karty SD: ");
      Serial.println(station_nr);
    }
    else
    {
      Serial.println("Błąd podczas otwierania pliku station_nr.txt.");
    }
  }
  else
  {
    Serial.println("Plik station_nr.txt nie istnieje.");
  }

  // Sprawdź, czy plik bank_nr.txt istnieje
  if (SD.exists("/bank_nr.txt"))
  {
    myFile = SD.open("/bank_nr.txt");
    if (myFile)
    {
      bank_nr = myFile.parseInt();
      myFile.close();
      Serial.print("Wczytano bank_nr z karty SD: ");
      Serial.println(bank_nr);
    }
    else
    {
      Serial.println("Błąd podczas otwierania pliku bank_nr.txt.");
    }
  }
  else
  {
    Serial.println("Plik bank_nr.txt nie istnieje.");
  }
}


// Funkcja odpowiedzialna za zmianę aktualnie wybranej stacji radiowej.
void changeStation()
{
  mp3 = flac = aac = vorbis = false;
  bitratePresent = false;

  stationInfo.remove(0);  // Usunięcie wszystkich znaków z obiektu stationInfo

  // Tworzymy nazwę pliku banku
  String fileNameWithBank = String("/bank") + (bank_nr < 10 ? "0" : "") + String(bank_nr) + ".txt";

  // Sprawdzamy, czy plik istnieje
  if (!SD.exists(fileNameWithBank))
  {
    Serial.println("Błąd: Plik banku nie istnieje.");
    return;
  }

  // Otwieramy plik w trybie do odczytu
  File bankFile = SD.open(fileNameWithBank, FILE_READ);
  if (!bankFile)
  {
    Serial.println("Błąd: Nie można otworzyć pliku banku.");
    return;
  }

  // Przechodzimy do odpowiedniego wiersza pliku
  int currentLine = 0;
  String stationUrl = "";
  
  while (bankFile.available())
  {
    String line = bankFile.readStringUntil('\n');
    currentLine++;

    if (currentLine == station_nr)
    {
      // Wyciągnij pierwsze 42 znaki i przypisz do stationName
      stationName = line.substring(0, 42);  // Skopiuj pierwsze 42 znaki z linii
      Serial.print("Nazwa stacji: ");
      Serial.println(stationName);

      // Znajdź część URL w linii
      int urlStart = line.indexOf("http");  // Szukamy miejsca, gdzie zaczyna się URL
      if (urlStart != -1)
      {
        stationUrl = line.substring(urlStart);  // Wyciągamy URL od "http"
        stationUrl.trim();  // Usuwamy białe znaki na początku i końcu
      }
      break;
    }
  }

  bankFile.close();  // Zamykamy plik po odczycie

  // Sprawdzamy, czy znaleziono stację
  if (stationUrl.isEmpty())
  {
    Serial.println("Błąd: Nie znaleziono stacji dla podanego numeru.");
    Serial.print("Numer stacji:");
    Serial.println(station_nr);
    Serial.print("Numer banku:");
    Serial.println(bank_nr);
    return;
  }

  // Weryfikacja, czy w linku znajduje się "http" lub "https"
  if (stationUrl.startsWith("http://") || stationUrl.startsWith("https://")) 
  {
    // Nazwa stacji
    String mainName = stationName;
    if (mainName.length() > 25)
    {
      mainName = mainName.substring(0, 25); // Przytnij do max 25 znaków
    }
    mainName.trim(); // Usuń ewentualne spacje z początku i końca

    canvas.fillRect(0, 0, 480, 40, COLOR_BLACK);
    canvas.setFont(&FreeSansBold18pt7b);   // Ustawienie dużej czcionki dla nazwy stacji
    canvas.setTextColor(COLOR_CYAN);        // Kolor nazwy stacji
    canvas.setCursor(0, 30);               // Ustawienie pozycji początkowej dla tekstu
    canvas.println(mainName);              // Wyświetlenie nazwy stacji
    tft_pushCanvas(canvas);

    // Wydrukuj nazwę stacji i link na serialu
    Serial.print("Aktualnie wybrana stacja: ");
    Serial.println(station_nr);
    Serial.print("Link do stacji: ");
    Serial.println(stationUrl);

    // Połącz z daną stacją
    audio.connecttohost(stationUrl.c_str());
    // Wczytujemy ustawienia głośności dla banku
    loadVolumeSettings(station_nr, bank_nr);
    previous_station_nr = station_nr;
    previous_bank_nr = bank_nr;
    saveStationOnSD();
  } 
  else 
  {
    Serial.println("Błąd: link stacji nie zawiera 'http' lub 'https'");
    Serial.println("Odczytany URL: " + stationUrl);
  }
}


void saveStationOnSD()
{
  // Sprawdź, czy plik station_nr.txt istnieje
  if (SD.exists("/station_nr.txt"))
  {
    Serial.println("Plik station_nr.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość station_nr
    myFile = SD.open("/station_nr.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(station_nr);
      myFile.close();  // Zamknięcie pliku po zapisie
      Serial.println("Aktualizacja station_nr.txt na karcie SD.");
    }
    else
    {
      Serial.println("Błąd podczas otwierania pliku station_nr.txt.");
    }
  }
  else
  {
    Serial.println("Plik station_nr.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość station_nr
    myFile = SD.open("/station_nr.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(station_nr);
      myFile.close();  // Zamknięcie pliku po zapisie
      Serial.println("Utworzono i zapisano station_nr.txt na karcie SD.");
    }
    else
    {
      Serial.println("Błąd podczas tworzenia pliku station_nr.txt.");
    }
  }

  // Sprawdź, czy plik bank_nr.txt istnieje
  if (SD.exists("/bank_nr.txt"))
  {
    Serial.println("Plik bank_nr.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość bank_nr
    myFile = SD.open("/bank_nr.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(bank_nr);
      myFile.close();  // Zamknięcie pliku po zapisie
      Serial.println("Aktualizacja bank_nr.txt na karcie SD.");
    }
    else
    {
      Serial.println("Błąd podczas otwierania pliku bank_nr.txt.");
    }
  }
  else
  {
    Serial.println("Plik bank_nr.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość bank_nr
    myFile = SD.open("/bank_nr.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(bank_nr);
      myFile.close();  // Zamknięcie pliku po zapisie
      Serial.println("Utworzono i zapisano bank_nr.txt na karcie SD.");
    }
    else
    {
      Serial.println("Błąd podczas tworzenia pliku bank_nr.txt.");
    }
  }
}


// Funkcja do wyświetlania listy stacji radiowych
void displayStations()
{
  // Wyczyść tylko obszar listy
  canvas.fillRect(0, 0, TFT_WIDTH, 230, COLOR_BLACK);

  // --- Nagłówek ---
  String header = "STACJE RADIOWE   " + String(station_nr) + " / " + String(stationsCount);
  canvas.setFont(&FreeSans12pt7b);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setCursor(80, 20);  // nagłówek w górnej części
  canvas.print(header);

  int displayRow = 1;

  // Iteruj po stacjach (maks 6 linii na ekranie)
  for (int i = firstVisibleLine; i < min(firstVisibleLine + maxVisibleLines, stationsCount); i++)
  {
    char station[STATION_NAME_LENGTH + 1];
    memset(station, 0, sizeof(station));

    // Odczytaj długość nazwy stacji
    int length = EEPROM.read(i * (STATION_NAME_LENGTH + 1));

    // Odczytaj znaki nazwy stacji
    for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++)
    {
      station[j] = EEPROM.read(i * (STATION_NAME_LENGTH + 1) + 1 + j);
    }

    // Utwórz string z nazwy stacji
    String stationName = String(station);
    stationName.trim();  // usuń spacje i śmieci

    // Przytnij do 25 znaków
    if (stationName.length() > 25)
    {
      stationName = stationName.substring(0, 25);
    }

    // Kolor w zależności od zaznaczenia
    if (i == currentSelection)
      canvas.setTextColor(COLOR_CYAN);
    else
      canvas.setTextColor(COLOR_LIME);

    // Wyświetl stację – obniżenie o 5 px
    canvas.setFont(&FreeSans12pt7b);
    canvas.setCursor(0, displayRow * 30 + 25);  // +5 pikseli w pionie
    canvas.print(stationName);

    // --- Diagnostyka ---
    //Serial.print("Wyświetlana stacja: ");
    //Serial.print(stationName);
    //Serial.print(" | Bank: ");
    //Serial.println(bank_nr);

    displayRow++;
  }

  // Wyślij canvas na ekran
  tft_pushCanvas(canvas);
}


// Funkcja do wyświetlania aktualnej stacji radiowej
void displayRadio()
{
  if (displayActive == false)
  {
    // Czyszczenie całego ekranu
    canvas.fillScreen(COLOR_BLACK);

    // Nazwa stacji
    String mainName = stationName;
    if (mainName.length() > 25)
    {
      mainName = mainName.substring(0, 25); // Przytnij do max 25 znaków
    }
    mainName.trim(); // Usuń ewentualne spacje z początku i końca

    canvas.setFont(&FreeSansBold18pt7b);   // Ustawienie dużej czcionki dla nazwy stacji
    canvas.setTextColor(COLOR_CYAN);        // Kolor nazwy stacji
    canvas.setCursor(0, 30);               // Ustawienie pozycji początkowej dla tekstu
    canvas.println(mainName);              // Wyświetlenie nazwy stacji

    // Informacje o aktualnym strumieniu (stream title)
    canvas.setFont(&FreeSans12pt7b);      // Ustawienie mniejszej czcionki dla stream title
    canvas.setTextColor(COLOR_LIME);      // Kolor tekstu informacji o stacji
    drawWrappedCanvasText(stationInfo.c_str(), 0, 70, 480, 30); // Wyświetlenie zawijanego tekstu

    // Numer banku i numer stacji
    String bankInfo = "Bank " + String(bank_nr);
    if (bank_nr < 10)
      bankInfo += " "; // Dodanie spacji dla wyrównania
    bankInfo += " Stacja " + String(previous_station_nr);

    canvas.setFont(&FreeMonoBold12pt7b);  // Czcionka dla informacji o banku i stacji
    canvas.setTextColor(COLOR_ORANGE);    // Kolor tekstu
    canvas.setCursor(0, 310);             // Pozycja w dolnej części ekranu
    canvas.print(bankInfo);               // Wyświetlenie numeru banku i stacji

    // Parametry audio (bitrate, sample rate, bits per sample)
    String audioInfoDisplay = "";
    bitrateString.trim();      // Usuń białe znaki
    sampleRateString.trim();
    bitsPerSampleString.trim();

    if (bitrateString.length() > 0)  audioInfoDisplay += bitrateString + " b/s   ";
    if (sampleRateString.length() > 0)  audioInfoDisplay += sampleRateString + " Hz    ";
    if (bitsPerSampleString.length() > 0)  audioInfoDisplay += bitsPerSampleString + " bit";

    canvas.setFont(&FreeMonoBold12pt7b);  // Czcionka dla parametrów audio
    canvas.setTextColor(COLOR_YELLOW);    // Kolor tekstu
    canvas.setCursor(5, 250);             // Pozycja tekstu powyżej głośności
    canvas.print(audioInfoDisplay);       // Wyświetlenie parametrów audio

    // Wyświetlenie głośności
    volumeDisplay = "VOL " + String(volumeValue);
    canvas.setTextColor(COLOR_WHITE);     // Kolor dla głośności
    canvas.setCursor(5, 280);             // Pozycja tekstu
    canvas.print(volumeDisplay);          // Wyświetlenie głośności

    // Typ odtwarzanego pliku (MP3, FLAC, AAC, etc.)
    canvas.setTextColor(COLOR_SPRINGGREEN); // Kolor tekstu
    canvas.setCursor(150, 280);           // Pozycja w dolnej części ekranu
    canvas.print(fileType);               // Wyświetlenie typu pliku

    // Wysyłanie całego canvasu na ekran TFT
    //tft_pushCanvas(canvas);
  }
}


// Funkcja do przewijania w górę
void scrollUp()
{
  if (currentSelection > 0)
  {
    currentSelection--;

    // Jeśli wyszliśmy poza górną widoczną linię → przesuwamy okno
    if (currentSelection < firstVisibleLine)
    {
      firstVisibleLine = currentSelection;
    }
  }
  else
  {
    // Skok z góry listy na ostatnią stację
    currentSelection = maxSelection();

    // Ustaw okno tak, aby ostatnie maxVisibleLines było widoczne
    if (stationsCount > maxVisibleLines)
    {
      firstVisibleLine = stationsCount - maxVisibleLines;
    }
    else
    {
      firstVisibleLine = 0;
    }
  }

  // Korekta granic
  if (firstVisibleLine < 0)
    firstVisibleLine = 0;
  if (firstVisibleLine > stationsCount - maxVisibleLines)
    firstVisibleLine = stationsCount - maxVisibleLines;

  // Debug
  //Serial.print("Scroll Up: currentSelection = ");
  //Serial.println(currentSelection);
  //Serial.print("Scroll Up: firstVisibleLine = ");
  //Serial.println(firstVisibleLine);
}


// Funkcja do przewijania w dół
void scrollDown()
{
  if (currentSelection < maxSelection())
  {
    currentSelection++;

    // Jeśli wyszliśmy poza dolną widoczną linię → przesuwamy okno
    if (currentSelection >= firstVisibleLine + maxVisibleLines)
    {
      firstVisibleLine++;
    }
  }
  else
  {
    // Skok z końca listy na pierwszą stację
    currentSelection = 0;
    firstVisibleLine = 0;
  }

  // Korekta granic
  if (firstVisibleLine < 0)
    firstVisibleLine = 0;
  if (firstVisibleLine > stationsCount - maxVisibleLines)
    firstVisibleLine = stationsCount - maxVisibleLines;

  // Debug
  Serial.print("Scroll Down: currentSelection = ");
  Serial.println(currentSelection);
  Serial.print("Scroll Down: firstVisibleLine = ");
  Serial.println(firstVisibleLine);
}


// Funkcja zwracająca maksymalny możliwy wybór w zależności od opcji
int maxSelection()
{
  if (currentOption == INTERNET_RADIO)
    return stationsCount - 1;
  else if (currentOption == PLAY_FILES)
  {
    if (folderSelection)
      return folderCount - 1;
    if (fileSelection)
      return filesCount - 1;
  }
  return 0;
}


// Funkcja do przełączania danych pogodowych
void switchWeatherData()
{
  unsigned long now = millis();
  unsigned long interval = 10000; // co 10 s

  if (now - lastSwitchWeather > interval)
  {
    lastSwitchWeather = now;

    // Czyszczenie obszaru pogodynki
    canvas.fillRect(0, 200, TFT_WIDTH, 30, COLOR_BLACK);

    if (weatherServerConnection)
    {
      String line1, line2;

      if (cycle == 0)
      {
        line1 = tempStr;       // np. "Temperatura: 21.5 C"
        line2 = feels_likeStr; // np. "Odczuwalna: 22.0 C"
      }
      else if (cycle == 1)
      {
        line1 = windStr;       // np. "Wiatr: 3.5 m/s"
        line2 = windGustStr;   // np. "W porywach: 5.2 m/s"
      }
      else if (cycle == 2)
      {
        line1 = humidityStr;   // np. "Wilgotność: 60 %"
        line2 = pressureStr;   // np. "Ciśnienie: 1015 hPa"
      }

      if (displayActive == false)
      {
        // Rysowanie danych pogodowych
        canvas.setFont(&FreeSans12pt7b);
        canvas.setTextColor(COLOR_PINK);
        canvas.setCursor(0, 220);   // linia 1 → Y=220
        canvas.print(line1);
        canvas.setCursor(240, 220); // linia 2 → obok
        canvas.print(line2);
      }
    }
    else
    {
      // Brak połączenia z serwerem
      String errorText = "--- Brak polaczenia z serwerem pogody ---";
      canvas.fillRect(0, 200, TFT_WIDTH, 30, COLOR_BLACK);
      canvas.setFont(&FreeSans12pt7b);
      canvas.setTextColor(COLOR_RED);
      canvas.setCursor(0, 220);
      canvas.print(errorText);
    }

    // Zmiana cyklu
    cycle++;
    if (cycle > 2) cycle = 0;
  }
}


// Funkcja do przełączania karuzeli kalendarza
void showCalendarCarousel()
{
  unsigned long now = millis();                               // Pobiera aktualny czas systemowy w milisekundach
  unsigned long interval = (messageIndex == 3) ? 5000 : 10000; // Krótszy czas (5s) dla imienin, 10s dla pozostałych komunikatów

  if (now - lastSwitchCalendar > interval)                    // Sprawdza, czy upłynął czas na zmianę komunikatu
  {
    lastSwitchCalendar = now;                                 // Aktualizuje czas ostatniej zmiany

    // Czyścimy obszar kalendarza (160–200)
    canvas.fillRect(0, 160, TFT_WIDTH, 40, COLOR_BLACK);      // Czyszczenie prostokątnego obszaru ekranu przed wyświetleniem nowego tekstu

    String msg;                                                // Bufor na wiadomość do wyświetlenia

    if (messageIndex == 0)
    {
      calendar = normalizePolish(calendar);                   // Zamienia polskie znaki diakrytyczne na poprawne
      msg = "Dzis jest " + calendar + " r";                   // Tworzy komunikat z datą
      messageIndex++;                                         // Przechodzi do następnego etapu karuzeli
    }
    else if (messageIndex == 1)
    {
      msg = "Wschod Slonca " + sunrise + "  Zachod Slonca " + sunset; // Komunikat o wschodzie i zachodzie Słońca
      messageIndex++;
    }
    else if (messageIndex == 2)
    {
      msg = "Dlugosc dnia " + dayLength;                      // Komunikat o długości dnia
      messageIndex++;
    }
    else if (messageIndex == 3)
    {
      if (namedayLines.empty())                               // Jeśli imieniny nie zostały jeszcze przetworzone na linie
      {
        String tekst = "IMIENINY: " + namedays;               // Tworzy tekst imienin
        tekst = normalizePolish(tekst);                       // Ujednolica polskie znaki

        String tmp = tekst;                                   // Tymczasowa kopia tekstu do podziału na linie
        while (tmp.length() > 0)                              // Dopóki coś zostało do przetworzenia
        {
          String line = tmp;                                  // Początkowo całość traktujemy jako jedną linię
          while (line.length() > 0)
          {
            int16_t w = 0;                                    // Szerokość tekstu w pikselach
            for (int i = 0; i < line.length(); i++)           // Obliczanie szerokości tekstu znak po znaku
            {
              char c = line[i];
              if (c < FreeSans12pt7b.first || c > FreeSans12pt7b.last) continue; // Pomija znaki spoza zakresu czcionki
              GFXglyph *glyph = &FreeSans12pt7b.glyph[c - FreeSans12pt7b.first]; // Pobiera metryki znaku
              w += glyph->xAdvance;                           // Sumuje szerokość znaku
            }
            if (w <= 480) break;                              // Jeśli linia mieści się w szerokości ekranu – OK
            int lastSpace = line.lastIndexOf(' ');            // Szuka ostatniej spacji do złamania linii
            if (lastSpace < 0) break;                         // Jeśli brak spacji, przerwij (tekst za długi bez przerw)
            line = line.substring(0, lastSpace);              // Ucina tekst do ostatniej spacji, by nie przekroczyć szerokości
          }
          namedayLines.push_back(line);                       // Dodaje gotową linię do listy
          tmp = tmp.substring(line.length());                 // Usuwa przetworzoną część z tekstu
          tmp.trim();                                         // Usuwa spacje z początku i końca
        }
        namedayLineIndex = 0;                                 // Resetuje indeks linii do wyświetlenia
      }

      if (namedayLineIndex < namedayLines.size())              // Jeśli są jeszcze linie imienin do pokazania
      {
        msg = namedayLines[namedayLineIndex];                  // Pobiera bieżącą linię
        namedayLineIndex++;                                   // Przechodzi do następnej
      } 
      else                                                    // Jeśli wszystkie linie zostały pokazane
      {
        namedayLines.clear();                                 // Czyści wektor linii imienin
        namedayLineIndex = 0;                                 // Resetuje indeks
        messageIndex = 0;                                     // Wraca na początek karuzeli (cykl od nowa)
      }
    }

    // Rysowanie tekstu w obszarze kalendarza
    if (msg.length() > 0)                                    // Jeśli mamy tekst do pokazania
    {
      canvas.setFont(&FreeSans12pt7b);                       // Ustawia czcionkę
      canvas.setTextColor(COLOR_SKYBLUE);                    // Kolor tekstu – jasnoniebieski
      canvas.setCursor(0, 190);                              // Ustawia pozycję startową tekstu
      canvas.print(msg);                                     // Wyświetla wiadomość
    }
  }
}


// Obsługa callbacka info o audio
void my_audio_info(Audio::msg_t m)
{
  switch(m.e)
  {
    case Audio::evt_info:  
    {
      String msg = String(m.msg);   // zamiana na String
      msg.trim(); // usuń spacje i \r\n

      // --- BitRate ---
      int bitrateIndex = msg.indexOf("BitRate:");
      if (bitrateIndex != -1)
      {
        int endIndex = msg.indexOf('\n', bitrateIndex);
        if (endIndex == -1) endIndex = msg.length();
        bitrateString = msg.substring(bitrateIndex + 8, endIndex);
        bitrateString.trim();

        if (currentOption == PLAY_FILES && audio.isRunning())
        {
          displayPlayer();
        }
        if (currentOption == INTERNET_RADIO && audio.isRunning())
        {
          displayRadio();
        }
      }

      // --- SampleRate ---
      int sampleRateIndex = msg.indexOf("SampleRate:");
      if (sampleRateIndex != -1)
      {
        int endIndex = msg.indexOf('\n', sampleRateIndex);
        if (endIndex == -1) endIndex = msg.length();
        sampleRateString = msg.substring(sampleRateIndex + 11, endIndex);
        sampleRateString.trim();
      }

      // --- BitsPerSample ---
      int bitsPerSampleIndex = msg.indexOf("BitsPerSample:");
      if (bitsPerSampleIndex != -1)
      {
        int endIndex = msg.indexOf('\n', bitsPerSampleIndex);
        if (endIndex == -1) endIndex = msg.length();
        bitsPerSampleString = msg.substring(bitsPerSampleIndex + 15, endIndex);
        bitsPerSampleString.trim();
      }

      // --- Brak ID3 tagów ---
      int metadata = msg.indexOf("skip metadata");
      if (metadata != -1)
      {
        id3tag = false;
        Serial.println("Brak ID3 - nazwa pliku: " + fileNameString);
        if (fileNameString.length() > 84)
        {
          fileNameString = fileNameString.substring(0, 84);
        }
      }

      // --- Rozpoznawanie dekodera / formatu ---
      if (msg.indexOf("MP3Decoder") != -1)
      {
        mp3 = true; fileType = "MP3";
        flac = false; aac = false; vorbis = false;
      }
      if (msg.indexOf("FLACDecoder") != -1)
      {
        flac = true; fileType = "FLAC";
        mp3 = false; aac = false; vorbis = false;
      }
      if (msg.indexOf("AACDecoder") != -1)
      {
        aac = true; fileType = "AAC";
        flac = false; mp3 = false; vorbis = false;
      }
      if (msg.indexOf("VORBISDecoder") != -1)
      {
        vorbis = true; fileType = "VORB";
        aac = false; flac = false; mp3 = false;
      }

      // --- Nieznana zawartość ---
      int unknowContent = msg.indexOf("unknown content found at:");
      if (unknowContent != -1)
      {
        audio.stopSong();
        canvas.fillRect(0, 35, 480, 220, COLOR_BLACK);
        canvas.setFont(&FreeSans12pt7b);
        canvas.setTextColor(COLOR_PINK);
        canvas.setCursor(0, 70);
        canvas.print("Nieznana zawartosc linku");
        tft_pushCanvas(canvas);
      }

      // --- Łączenie ze stacją ---
      int connectTo = msg.indexOf("connect to:");
      if (connectTo != -1)
      {
        canvas.fillRect(0, 40, 480, 220, COLOR_BLACK);
        canvas.setFont(&FreeSans12pt7b);
        canvas.setTextColor(COLOR_PINK);
        canvas.setCursor(0, 70);
        canvas.print("Nawiazywanie polaczenia, czekaj...");
        tft_pushCanvas(canvas);
      }

      // --- Reading file: ---
      int readingFileIndex = msg.indexOf("Reading file:");
      if (readingFileIndex != -1)
      {
        // Reset parametrów audio dla nowego pliku
        bitrateString = "";
        sampleRateString = "";
        bitsPerSampleString = "";
        bitratePresent = false;

        int startQuote = msg.indexOf('"', readingFileIndex);
        int endQuote = msg.indexOf('"', startQuote + 1);
        if (startQuote != -1 && endQuote != -1)
        {
          String fullPath = msg.substring(startQuote + 1, endQuote);
          fullPath.trim();

          // Rozdziel folder i plik
          int lastSlash = fullPath.lastIndexOf('/');
          if (lastSlash != -1)
          {
            folderNameString = fullPath.substring(0, lastSlash);
            fileNameString = fullPath.substring(lastSlash + 1);
          }
          else
          {
            folderNameString = "/";
            fileNameString = fullPath;
          }

          // Usuń początkowy '/' z folderu
          if (folderNameString.startsWith("/"))
          {
            folderNameString = folderNameString.substring(1);
          }

          // Zastąp pozostałe '/' w folderze spacją
          folderNameString.replace("/", " ");

          Serial.println("Wybrany folder: " + folderNameString);
          Serial.println("Wybrany plik: " + fileNameString);

          if ((currentOption == PLAY_FILES) && audio.isRunning())
          {
            displayPlayer();
          }
        }
      }

      // --- Debug ---
      Serial.printf("info: ....... %s\n", m.msg);
    }
    break;

    case Audio::evt_eof:
    {
      Serial.printf("end of file:  %s\n", m.msg);
      fileEnd = true;
    }
    break;

    case Audio::evt_bitrate:        Serial.printf("bitrate: .... %s\n", m.msg); break; // icy-bitrate or bitrate from metadata
    case Audio::evt_icyurl:         Serial.printf("icy URL: .... %s\n", m.msg); break;
    case Audio::evt_id3data:
    {
      Serial.printf("ID3 data: ... %s\n", m.msg);
      String msg = String(m.msg);
      msg.trim(); // usuń spacje i \r\n

      // --- ARTYSTA ---
      if (msg.startsWith("Artist:") || msg.startsWith("Artist=") || msg.startsWith("ARTIST="))
      {
        int sep = msg.indexOf(':');
        if (sep == -1) sep = msg.indexOf('=');
        artistString = msg.substring(sep + 1);
        artistString.trim();
        id3tag = true;
        Serial.println("Znalazłem artystę: " + artistString);
      }

      // --- TYTUŁ ---
      if (msg.startsWith("Title:") || msg.startsWith("Title=") || msg.startsWith("TITLE="))
      {
        int sep = msg.indexOf(':');
        if (sep == -1) sep = msg.indexOf('=');
        titleString = msg.substring(sep + 1);
        titleString.trim();
        id3tag = true;
        Serial.println("Znalazłem tytuł: " + titleString);
      }

      // --- ALBUM ---
      if (msg.startsWith("Album:") || msg.startsWith("Album=") || msg.startsWith("ALBUM="))
      {
        int sep = msg.indexOf(':');
        if (sep == -1) sep = msg.indexOf('=');
        albumString = msg.substring(sep + 1);
        albumString.trim();
        id3tag = true;
        Serial.println("Znalazłem album: " + albumString);
      }

      // --- YEAR ---
      if (msg.startsWith("Year:") || msg.startsWith("Year=") || msg.startsWith("DATE=") || msg.startsWith("Date="))
      {
        int sep = msg.indexOf(':');
        if (sep == -1) sep = msg.indexOf('=');
        yearString = msg.substring(sep + 1);
        yearString.trim();
        id3tag = true;
        Serial.println("Znalazłem rok: " + yearString);
      }
    }
    break;

    case Audio::evt_lasthost:       Serial.printf("last URL: ... %s\n", m.msg); break;
    case Audio::evt_name:           Serial.printf("station name: %s\n", m.msg); break; // station name or icy-name
    case Audio::evt_streamtitle:
    {
      // Zapisz tytuł stacji/utworu
      stationInfo = String(m.msg);
      stationInfo.trim();

      // Odśwież pełny ekran radia
      displayRadio();

      // Debug
      Serial.printf("stream title: %s\n", m.msg);
    }
    break;

    case Audio::evt_icylogo:        Serial.printf("icy logo: ... %s\n", m.msg); break;
    case Audio::evt_icydescription: Serial.printf("icy descr: .. %s\n", m.msg); break;
    case Audio::evt_image: for(int i = 0; i < m.vec.size(); i += 2){
                                    Serial.printf("cover image:  segment %02i, pos %07lu, len %05lu\n", i / 2, m.vec[i], m.vec[i + 1]);} break; // APIC
    case Audio::evt_lyrics:         Serial.printf("sync lyrics:  %s\n", m.msg); break;
    case Audio::evt_log   :         Serial.printf("audio_logs:   %s\n", m.msg); break;
    default:                        Serial.printf("message:..... %s\n", m.msg); break;
  }
}


// Funkcja rysująca zegar
void drawClock()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
    return;

  char timeString[9];
  snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // Wyczyść obszar zegara
  canvas.fillRect(245, 265, 245, 60, COLOR_BLACK);

  // Narysuj zegar
  canvas.setFont(&DS_DIGII35pt7b);
  canvas.setTextColor(COLOR_GOLD);
  canvas.setCursor(245, 310);
  canvas.print(timeString);

  // Brak strumienia audio
  if (!audio.isRunning())
  {
    canvas.fillRect(0, 225, 480, 30, COLOR_BLACK);
    canvas.setFont(&FreeMonoBold12pt7b);
    canvas.setTextColor(COLOR_RED);
    canvas.setCursor(5, 250);
    canvas.print("---- Brak strumienia audio ! ----");
  }

  // Licznik odtwarzanego pliku
  if (currentOption == PLAY_FILES && audio.isRunning() && isPlaying == true)
  {
    unsigned long elapsed = millis() - trackStartMillis;
    int seconds = (elapsed / 1000) % 60;
    int minutes = (elapsed / 60000) % 60;
    int hours   = (elapsed / 3600000);

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02dh:%02dm:%02ds", hours, minutes, seconds);
    fileTime = String(buffer);

    // miejsce na licznik utworu
    canvas.fillRect(0, 285, 230, 35, COLOR_BLACK);
    canvas.setFont(&FreeMonoBold12pt7b);
    canvas.setTextColor(COLOR_CYAN);
    canvas.setCursor(0, 310);
    canvas.print(fileTime);

    if (randomMode == true)
    {
      canvas.setTextColor(COLOR_RED);
      canvas.setCursor(185, 310);
      canvas.print("RND");
    }
  }

  tft_pushCanvas(canvas);
}


void displayMenu()
{
  // Wyczyść obszar w canvas
  canvas.fillRect(0, 0, 480, 160, COLOR_BLACK);

  // nagłówek MENU
  canvas.setFont(&FreeSansBold18pt7b);
  canvas.setTextColor(COLOR_CYAN);  
  canvas.setCursor(30, 30);
  canvas.print("Wybierz zrodlo dzwieku:");

  switch (currentOption)
  {
    case PLAY_FILES:
      canvas.setTextColor(COLOR_YELLOW);
      canvas.setCursor(0, 80);
      canvas.print(">>  Odtwarzacz plikow");

      canvas.setTextColor(COLOR_WHITE);
      canvas.setCursor(0, 120);
      canvas.print("      Radio internetowe");
      break;

    case INTERNET_RADIO:
      canvas.setTextColor(COLOR_WHITE);
      canvas.setCursor(0, 80);
      canvas.print("      Odtwarzacz plikow");

      canvas.setTextColor(COLOR_YELLOW);
      canvas.setCursor(0, 120);
      canvas.print(">>  Radio internetowe");
      break;
  }

  // wyślij na ekran
  tft_pushCanvas(canvas);
}


// Funkcja dzieląca tekst na fragmenty mieszczące się w szerokości maxWidth
void printWrappedText(String text, int x, int y, int lineHeight, int maxWidth, uint16_t color)
{
  canvas.setTextColor(color);
  int start = 0;
  while (start < text.length())
  {
    int end = start;
    String line = "";
    while (end < text.length())
    {
      line += text[end];
      int16_t x1, y1;
      uint16_t w, h;
      canvas.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
      if (w > maxWidth)
      {
        line.remove(line.length() - 1); // usuń ostatni znak, który nie mieści się
        break;
      }
      end++;
    }
    canvas.setCursor(x, y);
    canvas.print(line);
    y += lineHeight;
    start = end;
  }
}


// Funkcja przycina tekst, aby zmieścił się w maxWidth pikseli
String fitTextToWidth(String text, int maxWidth)
{
  int16_t x1, y1;
  uint16_t w, h;

  for (int i = text.length(); i > 0; i--)
  {
    String sub = text.substring(0, i);
    canvas.getTextBounds(sub.c_str(), 0, 0, &x1, &y1, &w, &h);
    if (w <= maxWidth) return sub;
  }
  return ""; // jeśli nic się nie mieści, zwracamy pusty string
}



/*-------------------------------------------------------GŁÓWNY SETUP PROGRAMU----------------------------------------------------------*/


void setup()
{
  Audio::audio_info_callback = my_audio_info; // Przypisanie własnej funkcji callback do obsługi zdarzeń i informacji audio

  Serial.begin(115200);
  pinMode(TFT_CS,OUTPUT);
  pinMode(TFT_DC,OUTPUT);
  digitalWrite(TFT_CS,HIGH);

  pinMode(recv_pin, INPUT); // Ustawienie pinu odbiornika IR jako wejście

  // Przerwanie na zmianę stanu pinu (odczyt impulsu)
  attachInterrupt(digitalPinToInterrupt(recv_pin), pulseISR, CHANGE);

  spi.begin(TFT_SCLK,TFT_MISO,TFT_MOSI,-1);
  spi.beginTransaction(SPISettings(40000000,MSBFIRST,SPI_MODE0));

  // Inicjalizacja SPI z nowymi pinami dla czytnika kart SD
  customSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  tft_init();
  Serial.println("TFT zainicjowany");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT); // Konfiguruj pinout dla interfejsu I2S audio
  audio.setVolume(volumeValue); // Ustaw głośność na podstawie wartości zmiennej volumeValue w zakresie 0...21

  // Sprawdzamy, czy pamięć PSRAM została poprawnie zainicjowana
  if (psramInit())
  {
    Serial.println("Pamięć PSRAM zainicjowana poprawnie");
    Serial.print("Dostepna pamięć PSRAM:");
    Serial.println(ESP.getPsramSize());
    Serial.print("Wolna pamięć PSRAM:");
    Serial.println(ESP.getFreePsram());
  }
  else
  {
    Serial.println("Błąd pamięci PSRAM");
  }

  // Inicjalizacja karty SD wraz z pierwszyn utworzeniem wymaganych plików w głównym katalogu karty, jeśli pliki już istnieją to funkcja sprawdza ich obecność
  SDinit();

  // Wyczyść cały ekran
  canvas.fillScreen(COLOR_BLACK);

  // Napis powitalny
  canvas.setFont(&FreeSansBold18pt7b);
  canvas.setTextColor(COLOR_YELLOW);
  canvas.setCursor(50, 100);
  canvas.print("WITAJ SLUCHACZU !");

  // Wyślij canvas na ekran
  tft_pushCanvas(canvas);

  audioBuffer.changeMaxBlockSize(16384);  // Wywołanie metody na obiekcie audioBuffer, is default 1600 for mp3 and aac, set 16384 for FLAC 

  // Inicjalizuj pamięć EEPROM z odpowiednim rozmiarem
  EEPROM.begin(MAX_STATIONS * (STATION_NAME_LENGTH + 1));  // 99 * 43 = 4257 bajtów

  // Inicjalizacja WiFiManagera
  WiFiManager wifiManager;

  // Odczytaj numer banku i numer stacji z karty SD
  readStationFromSD();
  previous_bank_nr = bank_nr; // Wyrównanie numerów banku przy starcie

  loadFileAndFolderIndexes();

  // Rozpoczęcie konfiguracji Wi-Fi i połączenie z siecią
  if (wifiManager.autoConnect("ESP Internet Radio"))
  {
    Serial.println("Połączono z siecią WiFi");
    // Napis potwierdzający połączenie z WiFi
    canvas.setFont(&FreeSansBold18pt7b);
    canvas.setTextColor(COLOR_GREEN);
    canvas.setCursor(60, 200);
    canvas.print("POLACZONO Z WIFI");

    // Wyślij canvas na ekran
    tft_pushCanvas(canvas);

    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", ntpServer); // Konfiguracja strefy czasowej dla Polski z czasem letnim
    timer1.attach(1, timerCallback);   // Ustaw timer, aby wywoływał funkcję updateTimer co sekundę
    timer2.attach(300, getWeatherData);   // Ustaw timer, aby wywoływał funkcję getWeatherData co 5 minut
    //timer3.attach(10, switchWeatherData);   // Ustaw timer, aby wywoływał funkcję switchWeatherData co 10 sekund
    fetchStationsFromServer();
    canvas.fillScreen(COLOR_BLACK);
    tft_pushCanvas(canvas);
    changeStation();
    getWeatherData();
    fetchAndDisplayCalendar();
  }
  else
  {
    Serial.println("Brak połączenia z siecią WiFi");

  }
}


/*-------------------------------------------------------GŁÓWNA PĘTLA PROGRAMU----------------------------------------------------------*/


void loop()
{
  audio.loop();               // Wykonuje główną pętlę dla obiektu audio (np. odtwarzanie dźwięku, obsługa audio)
  processIRCode();            // Funkcja przypisująca odpowiednie flagi do użytych przyciskow z pilota zdalnego sterowania
  volumeSetFromRemote();      // Obsługa regulacji głośności z pilota zdalnego sterowania
  vTaskDelay(1);              // Krótkie opóźnienie, oddaje czas procesora innym zadaniom

  if ((displayActive == false) && (stationsList == false))
  {
    showCalendarCarousel();
    switchWeatherData();
  }

  if (updateClockFlag == true)
  {
    updateClockFlag = false;
    drawClock();
  }

  if (IRrightArrow == true)  // Prawy przycisk kierunkowy w pilocie
  {
    IRrightArrow = false;
    station_nr++;
    if (station_nr > stationsCount)
    {
      station_nr = 1;
    }
    Serial.print("Numer stacji do przodu: ");
    Serial.println(station_nr);
    changeStation();
  }

  if (IRleftArrow == true)  // Lewy przycisk kierunkowy w pilocie
  {
    IRleftArrow = false;
    station_nr--;
    if (station_nr < 1)
    {
      station_nr = stationsCount;
    }
    Serial.print("Numer stacji do tyłu: ");
    Serial.println(station_nr);
    changeStation();
  }

  if (IRokButton == true)  // Przycisk OK w pilocie
  {
    IRokButton = false;
    displayActive = false;
    //audio.stopSong();
    if (currentOption == INTERNET_RADIO)
    {
      if (bankSwitch == true)
      {
        bankSwitch = false;
        canvas.fillRect(0, 285, 240, 35, COLOR_BLACK);
        canvas.setFont(&FreeMonoBold12pt7b);
        canvas.setTextColor(COLOR_RED);
        canvas.setCursor(0, 310);
        canvas.print("Pobieram stacje");
        tft_pushCanvas(canvas);
        currentSelection = 0;
        firstVisibleLine = 0;
        station_nr = 1;
        fetchStationsFromServer();
      }

      changeStation();
    }
    
    if (currentOption == PLAY_FILES)
    {
      // --- Pierwsze wejście: wczytanie folderów z SD ---
      if (!displayActive && !folderSelection)
      {
        displayActive = true;

        // Próba inicjalizacji karty SD
        if (!SD.begin(SD_CS))
        {
          Serial.println("Błąd inicjalizacji karty SD!");
          return;  // Przerwij jeśli SD nie jest gotowe
        }

        // Reset pozycji i indeksów folderów i plików
        folderIndex = 0;
        currentSelection = 0;
        firstVisibleLine = 0;

        // Wyczyść obszar w canvas
        canvas.fillRect(0, 0, 480, 225, COLOR_BLACK);

        canvas.setFont(&FreeSans12pt7b);
        canvas.setTextColor(COLOR_CYAN);
        canvas.setCursor(0, 25);
        canvas.print("Ladowanie folderow z karty SD, czekaj...");
        tft_pushCanvas(canvas);
        audio.stopSong();
        listDirectories("/");        // Odczyt folderów z katalogu głównego
        folderSelection = true;
        Serial.println("Lista folderów gotowa, wybierz folder i zatwierdź OK");
      }
      else
      {
        // --- Drugie OK: potwierdzenie wyboru folderu i start odtwarzania ---
        if (folderSelection == true)
        {
          folderSelection = false;
          Serial.println("Start odtwarzania folderu");
          volumeValue = 15;            
          audio.setVolume(volumeValue);
          playFromSelectedFolder();
        }
      }
    }
  }

  // Przycisk pilota FF+ zwiększanie numeru banku
  if (IRbankUp == true)
  {
    if (currentOption == INTERNET_RADIO)
    {
      IRbankUp = false;
      bankSwitch = true;
      displayActive = true;
      displayStartTime = millis();

      bank_nr++;
      if (bank_nr > 18)
      {
        bank_nr = 1;
      }
      canvas.fillRect(70, 285, 30, 35, COLOR_BLACK);
      canvas.setFont(&FreeMonoBold12pt7b);
      canvas.setTextColor(COLOR_YELLOW);
      canvas.setCursor(70, 310);
      canvas.print(bank_nr);
      tft_pushCanvas(canvas);
    }
  }

  // Przycisk pilota FF- zmniejszanie numeru banku
  if (IRbankDown == true)
  {
    if (currentOption == INTERNET_RADIO)
    {
      IRbankDown = false;
      bankSwitch = true;
      displayActive = true;
      displayStartTime = millis();

      bank_nr--;
      if (bank_nr < 1)
      {
        bank_nr = 18;
      }
      canvas.fillRect(70, 285, 30, 35, COLOR_BLACK);
      canvas.setFont(&FreeMonoBold12pt7b);
      canvas.setTextColor(COLOR_YELLOW);
      canvas.setCursor(70, 310);
      canvas.print(bank_nr);
      tft_pushCanvas(canvas);
    }
  }

  if (IRupArrow == true)  // Górny przycisk kierunkowy
  {
    IRupArrow = false;
    displayActive = true;
    displayStartTime = millis();

    if (currentOption == INTERNET_RADIO)
    {
      stationsList = true;
      bank_nr = previous_bank_nr;
      scrollUp(); 
      station_nr = currentSelection + 1;
      Serial.print("Numer stacji do tyłu: ");
      Serial.println(station_nr);
      displayStations();
    }

    if (currentOption == PLAY_FILES)
    {
      folderIndex = currentSelection;
      Serial.print("Numer folderu do tyłu: ");
      Serial.println(folderIndex);
      scrollUpFolders();
    }
  }

  if (IRdownArrow == true)  // Dolny przycisk kierunkowy
  {
    IRdownArrow = false;
    displayActive = true;
    displayStartTime = millis();

    if (currentOption == INTERNET_RADIO)
    {
      stationsList = true;
      bank_nr = previous_bank_nr;
      scrollDown(); 
      station_nr = currentSelection + 1;
      Serial.print("Numer stacji do przodu: ");
      Serial.println(station_nr);
      displayStations();
    }

    if (currentOption == PLAY_FILES)
    {
      folderIndex = currentSelection;
      Serial.print("Numer folderu do przodu: ");
      Serial.println(folderIndex);
      scrollDownFolders();
    }
  }

  if (IRsourceButton == true)  // Przycisk SOURCE w pilocie
  {
    IRsourceButton = false;
    menuEnable = true;
    displayActive = true;
    isPlaying = false;
    displayStartTime = millis();

    // Przełączanie opcji menu na drugą (cyklicznie)
    if (currentOption == INTERNET_RADIO)
    {
      currentOption = PLAY_FILES;
    }
    else
    {
      currentOption = INTERNET_RADIO;
    }

    displayMenu();
  }

  if (IRstopButton == true)  // Przycisk STOP w pilocie
  {
    IRstopButton = false;
    displayActive = false;
    stationsList = true;
    currentSelection = station_nr - 1;
    firstVisibleLine = currentSelection;
    audio.stopSong();
    Serial.println("Wciśnięto przycisk STOP - wyświetlam listę stacji");
    displayStations();
  }

  // Powrót do wyświetlania radia po bezczynności
  if (displayActive && (millis() - displayStartTime > DISPLAY_TIMEOUT) && (currentOption == INTERNET_RADIO)) 
  {
    station_nr = previous_station_nr; 
    bank_nr = previous_bank_nr;
    displayActive = false;
    displayStartTime = millis();

    stationsList = false;
    inputBuffer = "";
    inputActive = false;
    bankSwitch = false;

    Serial.println("Timeout: powrót do głównego ekranu radia");
    displayRadio();
  }

}
