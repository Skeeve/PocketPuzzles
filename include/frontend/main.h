#ifndef MAIN_HEADER
#define MAIN_HEADER

#include "common.h"

extern void chooserInit();
extern void chooserShowPage();
extern void chooserTap(int x, int y);
extern void chooserLongTap(int x, int y);
extern void chooserDrag(int x, int y);
extern void chooserRelease(int x, int y);

extern void gameInit();
extern void gameShowPage();
extern void gameTap(int x, int y);
extern void gameLongTap(int x, int y);
extern void gameDrag(int x, int y);
extern void gameRelease(int x, int y);

typedef enum {
    SCREEN_CHOOSER,
    SCREEN_GAME,
    SCREEN_HELP,
    SCREEN_PARAMS
} SCREENTYPE;

struct screen {
    SCREENTYPE currentScreen;
    void (*tap)(int x, int y);
    void (*long_tap)(int x, int y);
    void (*drag)(int x, int y);
    void (*release)(int x, int y);
} SCREEN;

void switchToChooser();
void switchToGame(const struct game *thegame);

static void setupApp();
void exitApp();

#endif

