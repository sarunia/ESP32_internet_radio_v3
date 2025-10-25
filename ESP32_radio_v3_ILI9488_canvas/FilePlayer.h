#ifndef FILE_PLAYER_H
#define FILE_PLAYER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include "Audio.h"
#include <Adafruit_GFX.h>

#define RGB565(r,g,b)  (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

#define COLOR_RED         RGB565(255, 0, 0)       // Czerwony
#define COLOR_GREEN       RGB565(0, 255, 0)       // Zielony
#define COLOR_BLUE        RGB565(0, 0, 255)       // Niebieski
#define COLOR_YELLOW      RGB565(255, 255, 0)     // Żółty
#define COLOR_CYAN        RGB565(0, 255, 255)     // Turkus / błękitny
#define COLOR_MAGENTA     RGB565(255, 0, 255)     // Magenta (fuksja)
#define COLOR_ORANGE      RGB565(255, 128, 0)     // Pomarańczowy
#define COLOR_PURPLE      RGB565(128, 0, 255)     // Fiolet
#define COLOR_PINK        RGB565(255, 0, 128)     // Różowy
#define COLOR_LIME        RGB565(128, 255, 0)     // Jasna limonka
#define COLOR_TURQUOISE   RGB565(0, 128, 255)     // Turkus
#define COLOR_WHITE       RGB565(255, 255, 255)   // Biały
#define COLOR_GOLD        RGB565(231, 211, 90)    // Złoty
#define COLOR_BLACK       RGB565(0, 0, 0)         // Czarny
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

extern int volumeValue;
extern int previous_fileIndex;
extern int previous_folderIndex;
extern int folderCount;
extern int filesCount;
extern int fileIndex;
extern int folderIndex;
extern int firstVisibleLine;
extern int maxVisibleLines;
extern int maxVisibleFolders;
extern int currentSelection;

extern bool IRokButton;
extern bool IRstopButton;
extern bool IRupArrow;
extern bool IRdownArrow;
extern bool IRrightArrow;
extern bool IRleftArrow;
extern bool IRsourceButton;

extern bool fileSelection;
extern bool folderSelection;
extern bool folderList;
extern bool id3tag;
extern bool isPlaying;
extern bool displayActive;
extern bool inputActive;
extern bool menuEnable;
extern bool playNextFile;
extern bool playPreviousFile;
extern bool fileEnd;
extern volatile bool updateClockFlag;

extern String files[];
extern String directories[];
extern String currentDirectory;
extern String artistString;
extern String titleString;
extern String albumString;
extern String yearString;
extern String folderNameString;
extern String fileNameString;
extern String bitrateString;
extern String sampleRateString;
extern String bitsPerSampleString;
extern String fileType;
extern String volumeDisplay;
extern String fileTime;
extern String inputBuffer;

extern unsigned long trackStartMillis;
extern unsigned long displayStartTime;
extern const unsigned long DISPLAY_TIMEOUT;

enum MenuOption
{
  PLAY_FILES,
  INTERNET_RADIO,
};

extern MenuOption currentOption;
extern File myFile;
extern Audio audio;
extern GFXcanvas16 canvas;

String normalizePolish(String text);
String fitTextToWidth(String text, int width);

void printWrappedText(String text, int x, int y, int lineHeight, int maxWidth, uint16_t color);
void tft_pushCanvas(GFXcanvas16 &c);

void displayPlayer();
void displayFiles();
void displayFolders();
void scrollUpFiles();
void scrollDownFiles();
void scrollDownFolders();
void scrollUpFolders();
void playFromSelectedFolder();
void playFile(String path);
void stopFile();
void saveFileAndFolderIndexes();
void loadFileAndFolderIndexes();
void printDirectoriesAndSavePaths(File dir, int numTabs, String currentPath);

extern void processIRCode();
extern void volumeSetFromRemote();
extern void drawClock();
extern bool isAudioFile(const char* name);

extern void displayMenu();
extern void listDirectories(const char *dirname);

int compareStringsWithNumbers(const String &a, const String &b);

#endif
