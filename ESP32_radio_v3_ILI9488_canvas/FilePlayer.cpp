#include "FilePlayer.h"
#include <FreeSans12pt7b.h>
#include <FreeMonoBold12pt7b.h>
#include <FreeSansBold18pt7b.h>

void displayPlayer()
{
  // Czyszczenie całego ekranu
  canvas.fillScreen(COLOR_BLACK);

  int x = 0;
  int y = 25;
  int lineHeight = 30;  // wysokość linii w pikselach

  canvas.setFont(&FreeSans12pt7b);

  // Nagłówek pliku/folderu
  canvas.setTextColor(COLOR_SKYBLUE);
  String header = "Odtwarzam Plik  " + String(previous_fileIndex + 1) + "/" + String(filesCount) +
                  "  Folder  " + String(previous_folderIndex + 1) + "/" + String(folderCount);
  canvas.setCursor(x, y);
  canvas.print(fitTextToWidth(header, 460));
  y += lineHeight;

  if (id3tag)
  {
    // Artysta
    artistString = normalizePolish(artistString);
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(x, y);
    canvas.print("Artysta: ");
    canvas.setTextColor(COLOR_WHITE);
    canvas.print(fitTextToWidth(artistString, 460 - canvas.getCursorX()));

    y += lineHeight;

    // Tytuł
    titleString = normalizePolish(titleString);
    String labelTitle = "Tytuł: ";
    labelTitle = normalizePolish(labelTitle);
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(x, y);
    canvas.print(labelTitle);
    canvas.setTextColor(COLOR_WHITE);
    canvas.print(fitTextToWidth(titleString, 460 - canvas.getCursorX()));

    y += lineHeight;

    // Album
    albumString = normalizePolish(albumString);
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(x, y);
    canvas.print("Album: ");
    canvas.setTextColor(COLOR_WHITE);
    canvas.print(fitTextToWidth(albumString, 460 - canvas.getCursorX()));

    y += lineHeight;

    // Rok
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(x, y);
    canvas.print("Rok: ");
    canvas.setTextColor(COLOR_WHITE);
    canvas.print(fitTextToWidth(yearString, 460 - canvas.getCursorX()));

    y += lineHeight;

    // Folder
    folderNameString = normalizePolish(folderNameString);
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(x, y);
    canvas.print("Folder: ");
    canvas.setTextColor(COLOR_WHITE);
    canvas.print(fitTextToWidth(folderNameString, 460 - canvas.getCursorX()));

    y += lineHeight;  // przejście do kolejnej linii

    // Plik
    fileNameString = normalizePolish(fileNameString);
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(x, y);
    canvas.print("Plik: ");
    canvas.setTextColor(COLOR_WHITE);
    canvas.print(fitTextToWidth(fileNameString, 460 - canvas.getCursorX()));
  }
  else
  {
    canvas.setTextColor(COLOR_RED);
    canvas.setCursor(x, y);
    canvas.print("Brak danych ID3 utworu");
    y += lineHeight;

    // Folder
    folderNameString = normalizePolish(folderNameString);
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(x, y);
    canvas.print("Folder: ");
    y += lineHeight;
    printWrappedText(folderNameString, x, y, lineHeight, 460 - x, COLOR_WHITE);
    y += ((folderNameString.length() / 20) + 1) * lineHeight;

    // Plik
    fileNameString = normalizePolish(fileNameString);
    canvas.setTextColor(COLOR_YELLOW);
    canvas.setCursor(x, y);
    canvas.print("Plik: ");
    y += lineHeight;
    printWrappedText(fileNameString, x, y, lineHeight, 460 - x, COLOR_WHITE);
  }

  // --- Parametry audio (bitrate, sample rate, bits per sample) ---
  String audioInfoDisplay = "";
  bitrateString.trim();      // Usuń białe znaki
  sampleRateString.trim();
  bitsPerSampleString.trim();

  if (bitrateString.length() > 0)  audioInfoDisplay += bitrateString + " b/s   ";
  if (sampleRateString.length() > 0)  audioInfoDisplay += sampleRateString + " Hz    ";
  if (bitsPerSampleString.length() > 0)  audioInfoDisplay += bitsPerSampleString + " bit";

  canvas.setFont(&FreeMonoBold12pt7b);  // Czcionka dla parametrów audio
  canvas.setTextColor(COLOR_GREEN);    // Kolor tekstu
  canvas.setCursor(5, 250);             // Pozycja tekstu powyżej głośności
  canvas.print(audioInfoDisplay);       // Wyświetlenie parametrów audio

  // --- Wyświetlenie głośności ---
  volumeDisplay = "VOL " + String(volumeValue);
  canvas.setTextColor(COLOR_WHITE);     // Kolor dla głośności
  canvas.setCursor(5, 280);             // Pozycja tekstu
  canvas.print(volumeDisplay);          // Wyświetlenie głośności

  // --- Typ odtwarzanego pliku (MP3, FLAC, AAC, etc.) ---
  canvas.setTextColor(COLOR_SPRINGGREEN); // Kolor tekstu
  canvas.setCursor(115, 280);           // Pozycja w dolnej części ekranu
  canvas.print(fileType);               // Wyświetlenie typu pliku

  // --- Wysyłanie całego canvasu na ekran TFT ---
  //tft_pushCanvas(canvas);

}


// Wyświetlanie przewijalnej listy plików z podświetleniem
void displayFiles()
{
  fileSelection = true;
  folderSelection = false;
  if (filesCount <= 0)
  {
    Serial.println("Brak plików do wyświetlenia!");
    canvas.fillRect(0, 0, 480, 320, COLOR_BLACK);
    canvas.setFont(&FreeSans12pt7b);
    canvas.setTextColor(COLOR_RED);
    canvas.setCursor(10, 50);
    canvas.print("Brak plikow w folderze!");
    tft_pushCanvas(canvas);
    return;
  }

  //Serial.printf("filesCount=%d fileIndex=%d currentSelection=%d firstVisibleLine=%d\n", filesCount, fileIndex, currentSelection, firstVisibleLine);

  const int maxVisibleFiles = 6;
  const int rowHeight = 30;
  const int maxChars = 46;  // maksymalna liczba znaków w wierszu

  // Czyszczenie ekranu
  canvas.fillRect(0, 0, 480, 230, COLOR_BLACK);

  // Nagłówek
  String header = "LISTA PLIKOW   " + String(fileIndex + 1) + " / " + String(filesCount);
  canvas.setFont(&FreeSans12pt7b);
  canvas.setTextColor(COLOR_CYAN);
  canvas.setCursor(50, 25);
  canvas.print(header);

  // Korekta firstVisibleLine
  if (currentSelection < firstVisibleLine)
  {
    firstVisibleLine = currentSelection;
  }
  if (currentSelection >= firstVisibleLine + maxVisibleFiles)
  {
    firstVisibleLine = currentSelection - maxVisibleFiles + 1;
  }

  //Serial.printf("Po korekcie: currentSelection=%d firstVisibleLine=%d\n", currentSelection, firstVisibleLine);

  int displayRow = 0;

  for (int i = firstVisibleLine; i < min(firstVisibleLine + maxVisibleFiles, filesCount); i++)
  {
    //Serial.printf("Rysuję plik index=%d (displayRow=%d)\n", i, displayRow);

    if (i < 0 || i >= filesCount)
    {
      Serial.printf("BŁĄD: i=%d poza zakresem (filesCount=%d)\n", i, filesCount);
      continue;
    }

    String fileNameDisplay = files[i];

    // Nazwa bez ścieżki
    int lastSlashIndex = fileNameDisplay.lastIndexOf('/');
    if (lastSlashIndex != -1)
    {
      fileNameDisplay = fileNameDisplay.substring(lastSlashIndex + 1);
    }

    // Przycinanie do max 42 znaków
    if (fileNameDisplay.length() > maxChars)
    {
      fileNameDisplay = fileNameDisplay.substring(0, maxChars - 3) + "...";
    }

    //Serial.printf("Plik do wyswietlenia='%s'\n", fileNameDisplay.c_str());

    int y = 65 + displayRow * rowHeight;

    // Podświetlenie zaznaczonego
    if (i == currentSelection)
    {
      //Serial.println(" -> Ten plik jest zaznaczony!");
      canvas.fillRect(0, y - 22, 480, rowHeight, COLOR_ORANGE);
      canvas.setTextColor(COLOR_BLACK);
    }
    else
    {
      canvas.setTextColor(COLOR_WHITE);
    }

    canvas.setCursor(0, y);
    canvas.print(fileNameDisplay);

    displayRow++;
  }

  tft_pushCanvas(canvas);
}


// Przewijanie listy plików w dół
void scrollDownFiles()
{
  fileIndex++;
  if (fileIndex >= filesCount) 
  {
    fileIndex = 0; // wróć na początek
  }
  Serial.print("Numer pliku do przodu: ");
  Serial.println(fileIndex + 1);

  currentSelection = fileIndex;

  if (currentSelection >= firstVisibleLine + 6)  // max 6 wierszy
  {
    firstVisibleLine++;
    if (firstVisibleLine > filesCount - 6) firstVisibleLine = filesCount - 6;
  }

  //displayFiles();
}


// Przewijanie listy plików w górę
void scrollUpFiles()
{
  fileIndex--;
  if (fileIndex < 0) 
  {
    fileIndex = filesCount - 1; // przejdź na ostatni
  }
  Serial.print("Numer pliku do tyłu: ");
  Serial.println(fileIndex + 1);

  currentSelection = fileIndex;

  if (currentSelection < firstVisibleLine) 
  {
    firstVisibleLine--;
    if (firstVisibleLine < 0) firstVisibleLine = 0;
  }

  //displayFiles();
}


// Przewijanie listy folderów w górę
void scrollUpFolders()
{
  if (folderCount <= 0) return;

  if (currentSelection > 0)
  {
    currentSelection--;
    if (currentSelection < firstVisibleLine) firstVisibleLine = currentSelection;
  }
  else
  {
    currentSelection = folderCount - 1;
    if (folderCount > maxVisibleFolders) firstVisibleLine = folderCount - maxVisibleFolders;
    else firstVisibleLine = 0;
  }

  // korekty granic
  if (firstVisibleLine < 0)
    firstVisibleLine = 0;
  if (firstVisibleLine > max(0, folderCount - maxVisibleFolders))
    firstVisibleLine = max(0, folderCount - maxVisibleFolders);

  // Zaktualizuj folderIndex, który może być używany w innych częściach programu
  folderIndex = currentSelection;

  //Serial.printf("scrollUpFolders: currentSelection=%d firstVisibleLine=%d folderIndex=%d\n", currentSelection, firstVisibleLine, folderIndex);

  displayFolders();
}


void scrollDownFolders()
{
  if (folderCount <= 0) return;

  if (currentSelection < folderCount - 1)
  {
    currentSelection++;
    if (currentSelection >= firstVisibleLine + maxVisibleFolders)
      firstVisibleLine++;
  }
  else
  {
    currentSelection = 0;
    firstVisibleLine = 0;
  }

  if (firstVisibleLine < 0)
    firstVisibleLine = 0;
  if (firstVisibleLine > max(0, folderCount - maxVisibleFolders))
    firstVisibleLine = max(0, folderCount - maxVisibleFolders);

  folderIndex = currentSelection;

  //Serial.printf("scrollDownFolders: currentSelection=%d firstVisibleLine=%d folderIndex=%d\n", currentSelection, firstVisibleLine, folderIndex);

  displayFolders();
}


void saveFileAndFolderIndexes()
{
  // ---- ZAPIS fileIndex ----
  if (SD.exists("/fileIndex.txt"))
  {
    Serial.println("Plik fileIndex.txt już istnieje. Aktualizuję...");
    myFile = SD.open("/fileIndex.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(fileIndex);
      myFile.close();
      Serial.print("Zapisano fileIndex = ");
      Serial.println(fileIndex);
    }
    else
    {
      Serial.println("Błąd podczas zapisu do fileIndex.txt");
    }
  }
  else
  {
    Serial.println("Plik fileIndex.txt nie istnieje. Tworzę nowy...");
    myFile = SD.open("/fileIndex.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(fileIndex);
      myFile.close();
      Serial.print("Utworzono i zapisano fileIndex = ");
      Serial.println(fileIndex);
    }
    else
    {
      Serial.println("Błąd podczas tworzenia fileIndex.txt");
    }
  }

  // ---- ZAPIS folderIndex ----
  if (SD.exists("/folderIndex.txt"))
  {
    Serial.println("Plik folderIndex.txt już istnieje. Aktualizuję...");
    myFile = SD.open("/folderIndex.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(folderIndex);
      myFile.close();
      Serial.print("Zapisano folderIndex = ");
      Serial.println(folderIndex);
    }
    else
    {
      Serial.println("Błąd podczas zapisu do folderIndex.txt");
    }
  }
  else
  {
    Serial.println("Plik folderIndex.txt nie istnieje. Tworzę nowy...");
    myFile = SD.open("/folderIndex.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(folderIndex);
      myFile.close();
      Serial.print("Utworzono i zapisano folderIndex = ");
      Serial.println(folderIndex);
    }
    else
    {
      Serial.println("Błąd podczas tworzenia folderIndex.txt");
    }
  }
}


void loadFileAndFolderIndexes()
{
  if (SD.exists("/fileIndex.txt"))
  {
    File f = SD.open("/fileIndex.txt");
    if (f)
    {
      fileIndex = f.parseInt();
      f.close();
      Serial.print("Odczytano fileIndex = ");
      Serial.println(fileIndex + 1);
    }
  }

  if (SD.exists("/folderIndex.txt"))
  {
    File f = SD.open("/folderIndex.txt");
    if (f)
    {
      folderIndex = f.parseInt();
      f.close();
      Serial.print("Odczytano folderIndex = ");
      Serial.println(folderIndex + 1);
    }
  }
}


void playFile()
{
  // Pobierz ścieżkę pliku z tablicy
  String fullPath = files[fileIndex];

  // Odtwórz tylko w przypadku, gdy to jest plik audio
  if (isAudioFile(fullPath.c_str()))
  {
    audio.connecttoFS(SD, fullPath.c_str());
    // Zerowanie poprzednich tagów ID3
    artistString = "";
    titleString = "";
    albumString = "";
    yearString = "";
    id3tag = false;  // Flaga oznaczająca, że jeszcze nie mamy danych ID3

    trackStartMillis = millis();
    fileTime = "00h:00m:00s";
    isPlaying = true;
    Serial.print("Odtwarzanie pliku: ");
    Serial.print(previous_fileIndex + 1);  // Liczymy od 1, nie od 0 na serialu
    Serial.print("/");
    Serial.print(filesCount); // Łączna liczba plików w folderze
    Serial.print(" - ");
    Serial.println(fullPath); // Pełna ścieżka pliku

    // Usunięcie folderu ze ścieżki pliku (zostaje tylko nazwa pliku)
    int lastSlashIndex = fullPath.lastIndexOf('/');
    if (lastSlashIndex != -1)
    {
      fileNameString = fullPath.substring(lastSlashIndex + 1);  // Wycinanie nazwy pliku po ostatnim ukośniku
    }
  }
}


// Funkcja do wyświetlania folderów na ekranie z uwzględnieniem zaznaczenia
void displayFolders()
{
  fileSelection = false;
  folderSelection = true;
  canvas.fillRect(0, 0, 480, 230, COLOR_BLACK);

  int displayIndex = 0;
  if (folderCount > 0)
  {
    displayIndex = constrain(currentSelection, 0, folderCount - 1);
  }

  // Nagłówek
  String header = "LISTA FOLDERÓW   " + String(displayIndex + 1) + " / " + String(folderCount);
  header = normalizePolish(header);
  canvas.setFont(&FreeSans12pt7b);
  canvas.setTextColor(COLOR_CYAN);
  canvas.setCursor(50, 25);
  canvas.print(header);

  int displayRow = 0;  
  int rowHeight = 30;   // odstęp pionowy między wpisami

  // Wyświetlanie maksymalnie 6 katalogów
  for (int i = firstVisibleLine; i < min(firstVisibleLine + 6, folderCount); i++)
  {
    String fullPath = directories[i];

    if (fullPath != "/System Volume Information")
    {
      // Nazwa skrócona (bez aktualnego katalogu "/")
      String displayedPath = fullPath.substring(currentDirectory.length(), currentDirectory.length() + 42);

      int y = 65 + displayRow * rowHeight;

      // Podświetlenie zaznaczonego katalogu
      if (i == currentSelection)
      {
        canvas.fillRect(0, y - 20, 480, rowHeight, COLOR_ORANGE);
        canvas.setTextColor(COLOR_BLACK);
      }
      else
      {
        canvas.setTextColor(COLOR_WHITE);
      }

      canvas.setCursor(0, y);
      canvas.print(displayedPath);

      displayRow++;
    }
  }

  // Wyślij całość na ekran
  tft_pushCanvas(canvas);
  
  //Serial.printf("displayFolders: folderCount=%d currentSelection=%d firstVisibleLine=%d folderIndex=%d\n", folderCount, currentSelection, firstVisibleLine, folderIndex);

}


// -------------------------------------------------------------
// Wyświetlanie menu ustawień (program menu w trybie odtwarzacza)
// Pokazuje opcje związane z odtwarzaniem plików
// -------------------------------------------------------------
void displayProgMenuPlayer()
{
  canvas.fillRect(0, 0, 480, 320, COLOR_BLACK);

  canvas.setFont(&FreeSansBold18pt7b);
  canvas.setTextColor(COLOR_CYAN);
  canvas.setCursor(30, 30);
  canvas.println("USTAWIENIA PLAYERA:");

  canvas.setFont(&FreeSans12pt7b);

  String fileTime = cfgFileCountdown ? "Czas utworu: OD KOŃCA" : "Czas utworu: OD POCZĄTKU";
  fileTime = normalizePolish(fileTime);

  String resume = lastPlaying ? "Wznów odtwarzanie: ON" : "Wznów odtwarzanie: OFF";
  resume = normalizePolish(resume);

  const char* options[3] =
  {
    fileTime.c_str(),
    randomMode ? "Losowe odtwarzanie: ON" : "Losowe odtwarzanie: OFF",
    resume.c_str()
  };

  for (int i = 0; i < 3; i++)
  {
    if (i == progMenuIndexPlayer)
      canvas.setTextColor(COLOR_YELLOW);
    else
      canvas.setTextColor(COLOR_WHITE);

    canvas.setCursor(40, 80 + i * 50);
    canvas.print(options[i]);
  }

  tft_pushCanvas(canvas);
}


// Funkcja do odtwarzania plików audio z wybranego folderu
void playFromSelectedFolder()
{
  // Ustawienia startowe
  fileSelection = false;
  folderSelection = false;
  folderList = false;
  folderNameString = directories[folderIndex];
  Serial.println("Odtwarzanie plików z wybranego folderu: " + folderNameString);

  // Otwórz folder
  File root = SD.open(folderNameString);
  if (!root)
  {
    Serial.println("Błąd otwarcia katalogu!");
    return;
  }

  filesCount = 0;

  if (!lastPlaying)   // jeśli nie wznawiamy, startujemy od pierwszego pliku
  {
    fileIndex = 0;
  }

  // Zbuduj listę plików audio w folderze
  while (true)
  {
    File entry = root.openNextFile();
    if (!entry) break;

    String name = entry.name();
    if (isAudioFile(name.c_str()))
    {
      files[filesCount] = String(folderNameString) + "/" + name;

      // Wydrukuj pełną ścieżkę i numer pliku
      Serial.print("Dodano plik [");
      Serial.print(filesCount + 1);
      Serial.print("]: ");
      Serial.println(files[filesCount]);

      filesCount++;
    }
    entry.close();
  }

  Serial.print("Łącznie znaleziono plików audio: ");
  Serial.println(filesCount);

  // Jeśli brak plików, zamknij i wróć
  if (filesCount == 0)
  {
    Serial.println("Brak plików audio w folderze: " + folderNameString);
    root.close();
    return;
  }

  // Przygotuj katalog do odczytu od początku (używane przy rewindingu)
  root.rewindDirectory();

  bool playNextFolder = false;   // flaga: przejście do następnego folderu
  bool menuRequested = false;    // flaga: żądanie wyjścia do menu (przycisk MENU)
  bool stopAll = false;          // ewentualne dodatkowe wyjście

  // Główna pętla odtwarzania plików w folderze
  while (fileIndex < filesCount && !playNextFolder && !menuRequested && !stopAll)
  {
    String fullPath = files[fileIndex];
    Serial.print("Odtwarzanie pliku ");
    Serial.print(fileIndex + 1);
    Serial.print("/");
    Serial.println(filesCount);
    Serial.println(fullPath);

    // Start odtwarzania pliku audio z systemu plików
    audio.connecttoFS(SD, fullPath.c_str());

    // Zerowanie poprzednich tagów ID3 (tytuł, artysta, album, rok)
    artistString = "";
    titleString = "";
    albumString = "";
    yearString = "";
    id3tag = false;              // Flaga oznaczająca brak aktualnie odczytanych danych ID3

    // Zerowanie informacji o czasie trwania pliku
    audioDurationFromDecoder = false; // Flaga: czy czas utworu pochodzi z dekodera
    audioDurationSec = 0;             // Czas trwania w sekundach (liczony później)
    audioDurationString = "";         // Czas trwania w formacie tekstowym (hh:mm:ss lub mm:ss)

    // Ustawienie czasu startu odtwarzania i inicjalizacja licznika czasu wyświetlanego na ekranie
    trackStartMillis = millis();      // Zapamiętanie momentu rozpoczęcia odtwarzania
    fileTime = "00h:00m:00s";         // Wstępna wartość wyświetlanego czasu utworu

    isPlaying = true;                 // Flaga informująca, że plik jest aktualnie odtwarzany
    previous_fileIndex = fileIndex;   // Zapamiętanie indeksu poprzedniego pliku
    previous_folderIndex = folderIndex; // Zapamiętanie indeksu poprzedniego folderu

    displayPlayer();                  // Wyświetlenie ekranu odtwarzacza z aktualnym plikiem


    // Pętla oczekiwania na koniec pliku / sterowanie z pilota
    while (isPlaying && !menuRequested && !stopAll)
    {
      audio.loop();
      processIRCode();              // odczyt flag z pilota
      volumeSetFromRemote();        // regulacja głośności z pilota
      vTaskDelay(1);

      // Aktualizacja zegara / czasu utworu co sekundę
      if (updateClockFlag)
      {
        updateClockFlag = false;
        drawClock();
      }
      // Aktualizacja czasu trwania pliku (liczony od startu trackStartMillis)
      if (currentOption == PLAY_FILES)
      {
        unsigned long elapsed = (millis() - trackStartMillis) / 1000; // sekundy
        unsigned int h = elapsed / 3600;
        unsigned int m = (elapsed % 3600) / 60;
        unsigned int s = elapsed % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02dh:%02dm:%02ds", h, m, s);
        fileTime = String(buf);
      }

      // --- MENU PLAYERA ---
      if (IRprogButton)
      {
        IRprogButton = false;
        progMenuActivePlayer = !progMenuActivePlayer;
        displayActive = progMenuActivePlayer;

        if (progMenuActivePlayer)
        {
          progMenuIndexPlayer = 0;
          displayProgMenuPlayer();
        }
        else
        {
          displayPlayer(); // powrót do ekranu playera
        }
      }

      if (progMenuActivePlayer)
      {
        // --- Nawigacja w menu playera ---
        if (IRupArrow)
        {
          IRupArrow = false;
          progMenuIndexPlayer--;
          if (progMenuIndexPlayer < 0)
            progMenuIndexPlayer = progMenuPlayerOptions - 1;
          displayProgMenuPlayer();
        }

        if (IRdownArrow)
        {
          IRdownArrow = false;
          progMenuIndexPlayer++;
          if (progMenuIndexPlayer >= progMenuPlayerOptions)
            progMenuIndexPlayer = 0;
          displayProgMenuPlayer();
        }

        if (IRokButton)
        {
          IRokButton = false;

          switch (progMenuIndexPlayer)
          {
            case 0: // Czas utworu
              cfgFileCountdown = !cfgFileCountdown;
              Serial.println(cfgFileCountdown ? "Czas utworu: OD KOŃCA" : "Czas utworu: OD POCZĄTKU");
              break;

            case 1: // Losowe odtwarzanie
              randomMode = !randomMode;
              Serial.println(randomMode ? "Losowe odtwarzanie: ON" : "Losowe odtwarzanie: OFF");
              break;

            case 2: // WZNÓW ODTWARZANIE
              lastPlaying = !lastPlaying;
              Serial.println(lastPlaying ? "Wznów odtwarzanie: ON" : "Wznów odtwarzanie: OFF");
              break;
          }

          displayProgMenuPlayer();
          saveSettingsToSD();          // Zapis aktualnych ustawień na kartę SD
        }

        continue; // menu playera ma priorytet
      }


      // Wyjście do menu wyboru źródła audio - przycisk SOURCE
      if (IRsourceButton)
      {
        IRsourceButton = false;
        menuRequested = true;
        Serial.println("Włączenie MENU wyboru źródła dźwięku - wychodzę z playera");
        audio.stopSong();
        isPlaying = false;
        break;
      }

      if (randomMode && fileEnd)
      {
        fileEnd = false;
        audio.stopSong();
        isPlaying = false;

        fileIndex = getRandomFileIndex(filesCount);
        Serial.printf("Tryb RANDOM: wylosowano plik %d/%d\n", fileIndex + 1, filesCount);
        saveFileAndFolderIndexes();

        break; // wyjdź z wewnętrznej pętli, kontynuuj odtwarzanie z nowym fileIndex
      }

      // Następny plik (pilot lub koniec pliku)
      if (playNextFile || IRrightArrow || fileEnd)
      {
        IRrightArrow = false;
        playNextFile = false;
        fileEnd = false;
        audio.stopSong();
        isPlaying = false;
        fileIndex++;
        saveFileAndFolderIndexes();
        if (fileIndex >= filesCount)
        {
          Serial.println("To jest ostatni plik w folderze - przechodzę do następnego folderu");
          playNextFolder = true;
        }
        else
        {
          // normalne przejście do następnego pliku w pętli
        }
        break; // wyjście z wewnętrznej pętli, kontynuacja w zewnętrznej
      }

      // Poprzedni plik
      if (playPreviousFile || IRleftArrow)
      {
        IRleftArrow = false;
        playPreviousFile = false;
        audio.stopSong();
        isPlaying = false;
        if (fileIndex > 0)
          fileIndex--;
        else
          fileIndex = 0;
        saveFileAndFolderIndexes();
        break;
      }

      // Zatwierdzenie wyboru folderu -> odtwarzaj wybrany (podświetlony) folder
      if (IRokButton && folderSelection)
      {
        IRokButton = false;
        folderSelection = false;
        displayActive = false;
        audio.stopSong();
        isPlaying = false;

        // Ustaw folderIndex na aktualne zaznaczenie
        folderIndex = constrain(currentSelection, 0, folderCount - 1);
        fileIndex = 0;

        // Zabezpieczenie: zamknij katalog przed rekurencyjnym wywołaniem
        root.close();

        Serial.printf("Zatwierdzono folder: %d -> %s\n", folderIndex + 1, directories[folderIndex].c_str());

        // Rozpocznij odtwarzanie z wybranego folderu
        saveFileAndFolderIndexes();
        playFromSelectedFolder();
        return; // nie kontynuuj starej pętli
      }

      // Zatwierdzenie wyboru pliku
      if (IRokButton && fileSelection)
      {
        IRokButton = false;
        fileSelection = false;
        displayActive = false;
        audio.stopSong();
        isPlaying = false;
        saveFileAndFolderIndexes();
        break;
      }


      // --- OBSŁUGA STRZAŁEK GÓRA / DÓŁ (z wrap-around i aktualizacją okna) ---
      if (IRupArrow)
      {
        IRupArrow = false;
        displayActive = true;
        displayStartTime = millis();

        if (folderSelection)
        {
          if (folderCount > 0)
          {
            if (currentSelection > 0) currentSelection--;
            else currentSelection = folderCount - 1;
            folderIndex = currentSelection;
            if (currentSelection < firstVisibleLine) firstVisibleLine = currentSelection;
            if (currentSelection >= firstVisibleLine + maxVisibleLines) firstVisibleLine = currentSelection - maxVisibleLines + 1;
          }
          displayFolders();
        }
        else
        {
          fileSelection = true;
          scrollUpFiles();
          fileIndex = currentSelection;
          folderIndex = previous_folderIndex;
          displayFiles();
        }
      }

      if (IRdownArrow)
      {
        IRdownArrow = false;
        displayActive = true;
        displayStartTime = millis();

        if (folderSelection)
        {
          if (folderCount > 0)
          {
            if (currentSelection < folderCount - 1) currentSelection++;
            else currentSelection = 0;
            folderIndex = currentSelection;
            if (currentSelection < firstVisibleLine) firstVisibleLine = currentSelection;
            if (currentSelection >= firstVisibleLine + maxVisibleLines) firstVisibleLine = currentSelection - maxVisibleLines + 1;
          }
          displayFolders();
        }
        else
        {
          fileSelection = true;
          scrollDownFiles();
          fileIndex = currentSelection;
          folderIndex = previous_folderIndex;
          displayFiles();
        }
      }

      // Jeśli aktywne wyświetlenie listy folderów
      if (folderList == true)
      {
        folderList = false;
        folderSelection = true;
        displayActive = true;
        displayStartTime = millis();

        currentSelection = constrain(folderIndex, 0, folderCount - 1);
        firstVisibleLine = max(0, currentSelection - maxVisibleLines / 2);

        Serial.printf("Lista folderów -> currentSelection=%d firstVisibleLine=%d\n", currentSelection, firstVisibleLine);

        displayFolders();
      }

      if (IRstopButton == true)  // Przycisk STOP w pilocie
      {
        IRstopButton = false;
        displayActive = false;
        folderList = true;
        audio.stopSong();
        Serial.println("Wciśnięto przycisk STOP - wyświetlam listę katalogów");
      }



      // Powrót do wyświetlania playera po bezczynności
      if (displayActive && (millis() - displayStartTime > displayTimeout)) 
      {
        inputBuffer = "";
        inputActive = false;
        fileSelection = false;
        folderSelection = false;
        folderList = false;
        displayActive = false;
        displayStartTime = millis();
        fileIndex = previous_fileIndex;
        folderIndex = previous_folderIndex;
        Serial.println("Timeout: powrót do głównego ekranu playera");
        displayPlayer();
      }

    } // koniec while(isPlaying)

    // Jeśli przycisk MENU menu został wywołany wewnątrz, przerwij odtwarzanie i wyjdź
    if (menuRequested)
    {
      // Zamknij katalog i pokaż menu (przejście do wyboru źródła)
      root.close();
      menuEnable = true;
      displayActive = true;
      displayStartTime = millis();
      displayMenu();
      return;          // Wyjście z funkcji playFromSelectedFolder
    }

    // Jeśli ustawiono playNextFolder -> ustaw indeks folderu i powtórz
    if (playNextFolder)
    {
      folderIndex++;
      if (folderIndex >= folderCount)
      {
        Serial.println("To był ostatni folder.");
        break;
      }
      fileIndex = 0;

      root.close();
      playFromSelectedFolder();
      return;
    }

  }

  // Zamknij katalog przy normalnym wyjściu
  root.close();

  Serial.println("Wyjście z playFromSelectedFolder()");
}


void printDirectoriesAndSavePaths(File dir, int numTabs, String currentPath)
{
  folderCount = 0;

  // Przejrzyj wszystkie pliki w katalogu
  while (true)
  {
    File entry = dir.openNextFile();
    
    if (!entry)
    {
      break; // Koniec plików
    }

    if (entry.isDirectory())
    {
      // Utwórz pełną ścieżkę
      String path = currentPath + "/" + entry.name();
      
      // Sprawdź, czy katalog to nie "System Volume Information"
      if (path != "/System Volume Information")
      {
        directories[folderCount] = path; // Zapisz ścieżkę do tablicy
        folderCount++; // Zwiększ licznik katalogów
      }
    }

    entry.close();
  }

  // Sortowanie katalogów za pomocą funkcji porównującej
  for (int i = 0; i < folderCount - 1; i++)
  {
    for (int j = i + 1; j < folderCount; j++)
    {
      if (compareStringsWithNumbers(directories[i], directories[j]) > 0)
      {
        String temp = directories[i];
        directories[i] = directories[j];
        directories[j] = temp;
      }
    }
  }

  // Wydrukuj na serial terminalu alfabetycznie posortowane katalogi
  for (int i = 0; i < folderCount; i++)
  {
    Serial.print(i + 1); // Drukuje alfabetyczny numer katalogu
    Serial.print(": ");
    Serial.println(directories[i].substring(1)); // Drukuje ścieżkę bez pierwszego znaku
  }

  // Wyświetl na ekranie, jeśli to nie System Volume Information
  for (int i = 0; i < folderCount; i++)
  {
    String fullPath = directories[i];
  }
}


// Funkcja do porównywania ciągów uwzględniająca liczby
int compareStringsWithNumbers(const String &a, const String &b)
{
  int i = 0, j = 0;
  
  while (i < a.length() && j < b.length())
  {
    // Wyciągnij kolejne znaki
    char charA = a[i];
    char charB = b[j];
    
    // Sprawdź, czy mamy do czynienia z liczbą w obu ciągach
    if (isdigit(charA) && isdigit(charB))
    {
      // Wyciągnij pełne liczby
      String numA, numB;
      while (i < a.length() && isdigit(a[i]))
      {
        numA += a[i++];
      }
      while (j < b.length() && isdigit(b[j]))
      {
        numB += b[j++];
      }

      // Porównaj liczby jako liczby (nie tekstowo)
      int intA = numA.toInt();
      int intB = numB.toInt();
      
      if (intA != intB)
      {
        return intA - intB;
      }
    } 
    else
    {
      // Porównaj inne znaki
      if (charA != charB)
      {
        return charA - charB;
      }
      i++;
      j++;
    }
  }
  
  // Jeżeli wszystko jest równe, porównaj długości
  return a.length() - b.length();
}


// Funkcja do wylistowania katalogów z karty 
void listDirectories(const char *dirname)
{
  File root = SD.open(dirname);
  if (!root)
  {
    Serial.println("Błąd otwarcia katalogu!");
    Serial.println(dirname);
    return;
  }
  printDirectoriesAndSavePaths(root, 0, ""); // Początkowo pełna ścieżka jest pusta
  Serial.println("Wylistowano katalogi z karty SD");
  root.close();

  displayFolders();
}

// Funkcja przełączająca tryb odtwarzania plików w cyklu losowy <--> normalny
void displayRandomMode(bool randomEnabled)
{
  // Czyścimy górny obszar
  canvas.fillRect(0, 0, 480, 145, COLOR_BLACK);

  canvas.setFont(&FreeSansBold18pt7b);
  canvas.setTextColor(COLOR_CYAN);
  canvas.setCursor(50, 40);
  canvas.print("Tryb odtwarzania:");

  if (randomEnabled)
  {
    canvas.setTextColor(COLOR_GREEN);
    canvas.setCursor(0, 100);
    canvas.print("Tryb losowy ON");
    Serial.println("Tryb losowy ON");
  }
  else
  {
    canvas.setTextColor(COLOR_WHITE);
    canvas.setCursor(0, 100);
    canvas.print("Tryb losowy OFF");
    Serial.println("Tryb losowy OFF");
  }

  tft_pushCanvas(canvas);
}


// ------------------------------------------------------------
// Funkcja: getRandomFileIndex(int totalFiles)
// ------------------------------------------------------------
// Opis:
//   Zwraca losowy indeks pliku z zakresu [0, totalFiles),
//   gwarantując, że dany plik nie powtórzy się dopóki wszystkie
//   pliki z folderu nie zostaną już odtworzone.
// 
// Zastosowanie:
//   Używana w trybie RANDOM odtwarzacza plików (randomMode),
//   aby każdy utwór został wylosowany tylko raz.
// ------------------------------------------------------------
int getRandomFileIndex(int totalFiles)
{
  static std::vector<int> usedIndices;        // pamięta indeksy już użytych plików między wywołaniami funkcji
  if (totalFiles <= 0) return 0;              // zabezpieczenie: brak plików → zwróć 0

  // Reset puli, jeśli wszystkie pliki zostały już wykorzystane
  if ((int)usedIndices.size() >= totalFiles)
  {
    usedIndices.clear();                      // czyścimy listę, żeby rozpocząć nowy cykl losowań
  }

  int randomIndex;
  bool unique = false;                        // flaga oznaczająca, czy wylosowany indeks jest unikalny

  // Pętla losowania dopóki nie trafimy na indeks nieużywany wcześniej
  do
  {
    randomIndex = random(0, totalFiles);      // losujemy numer pliku z zakresu [0, totalFiles)
    // sprawdzamy, czy taki indeks już był użyty
    unique = (std::find(usedIndices.begin(), usedIndices.end(), randomIndex) == usedIndices.end());
  }
  while (!unique);                            // powtarzaj, dopóki nie trafisz na unikalny numer

  usedIndices.push_back(randomIndex);         // dodaj nowy indeks do listy wykorzystanych
  return randomIndex;                         // zwróć wylosowany, unikalny indeks pliku
}