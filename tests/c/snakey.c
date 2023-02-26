/**
 * This should become a conversion of my game "Snakey" which has been written for
 * the Arduino / Arduboy platform.
 * 
 * Since this is my first attempt to develop a simple game for Amiga there are
 * likely some errors in this source. Hints to do things better are appreciated.
 * 
 * This code was developed with VBCC and compilation has been tested on Linux
 * and AmigaOS 3.2.
 * 
 * @created 2022-05-08
 * @author agewert@ubergeek.de
 * @see www.ubergeek.de
 */

#include <stdlib.h>
#include <stdio.h>
#include <clib/all_protos.h>


#pragma region Prototypes

void exitWithError(char* text);
void exitNormally();
void closeResources();
void mainLoop();

#pragma endregion


#pragma region Globals

struct Screen* mainScreen;
struct Window* mainWindow;
struct ViewPort* viewPort;
struct RastPort* rastPort;

#pragma endregion


int main(int argc, char** argv) {
    //UWORD pens[] = {~0};

    if (argc == 0) {
        //printf("Started from workbench\n");
    } else {
        //printf("Started from cli");
    }

    mainScreen = OpenScreenTags(NULL,
        SA_Title, "Snakey",
        SA_Type, LORES_KEY,
        SA_Width, 320,
        SA_Height, 256,
        SA_Draggable, FALSE,
        SA_ShowTitle, FALSE,
        SA_Depth, 5,
        TAG_DONE
    );

    if (!mainScreen) exitWithError("Konnte Screen nicht öffnen!");

    mainWindow = OpenWindowTags(NULL,
        WA_CustomScreen, mainScreen,
        WA_Backdrop, TRUE,
        WA_Borderless, TRUE,
        WA_Left, 0,
        WA_Top, 0,
        WA_Width, 320,
        WA_Height, 256,
        TAG_DONE
    );

    if (!mainWindow) exitWithError("Konnte Fenster nicht öffnen!");

    rastPort = mainWindow->RPort;
    viewPort = &mainScreen->ViewPort;
    //rastPort = &mainScreen->RastPort;

    SetRGB32(viewPort, 0, 0, 0, 0);
    SetRGB32(viewPort, 1, 0xffffffff, 0xffffffff, 0xffffffff);
    mainLoop();

    Delay(100);
    exitNormally();
}

void exitWithError(char* text) {
    printf(text);
    closeResources();
    exit(EXIT_FAILURE);
}

void exitNormally() {
    closeResources();
    exit(EXIT_SUCCESS);
}

void mainLoop() {
    int x = 20;
    int y = 20;
    SetAPen(rastPort, 1);

    for (long t = 0; t <= 100; t++) {
        Move(rastPort, x, y);
        Draw(rastPort, x +1, y +1);
        x++;
        y++;
        WaitBlit();
        Delay(5);
    }
}

void closeResources() {
    if (mainWindow) CloseWindow(mainWindow);
    if (mainScreen) CloseScreen(mainScreen);
}