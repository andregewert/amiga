#include <stdio.h>
#include <stdlib.h>
#include <exec/types.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include "include/Cheetah.pic.c"

// Prototypes
void exitWithError(char* text);
void exitNormally();
void closeResources();
void mainLoop();

// Global vars
struct Screen* mainScreen;
struct Window* mainWindow;
struct ViewPort* viewPort;
struct RastPort* rastPort;

int main(int argc, char* argv[]) {
    if (argc == 0) {
        printf("Started from workbench\n");
    } else {
        printf("Started from cli\n");
    }

    mainScreen = OpenScreenTags(NULL,
        SA_Title, "Grafiktest",
        SA_Type, LORES_KEY,
        SA_Width, 320,
        SA_Height, 200,
        SA_Draggable, FALSE,
        SA_ShowTitle, FALSE,
        SA_Depth, 5,
        SA_Colors32, CheetahPaletteRGB32,
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
        WA_Height, 200,
        TAG_DONE
    );

    if (!mainWindow) exitWithError("Konnte Fenster nicht öffnen!");
    
    rastPort = mainWindow->RPort;
    viewPort = &mainScreen->ViewPort;

    //SetRGB32(viewPort, 0, 0, 0, 0);
    //SetRGB32(viewPort, 1, 0xffffffff, 0xffffffff, 0xffffffff);
    
    DrawImage(rastPort, &Cheetah, 0, 0);
    
    mainLoop();

    Delay(100);
    exitNormally();
}

void exitWithError(char* text) {
    printf("%s", text);
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
        Delay(1);
    }
}

void closeResources() {
    if (mainWindow) CloseWindow(mainWindow);
    if (mainScreen) CloseScreen(mainScreen);
}
