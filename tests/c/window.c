#include <stdlib.h>
#include <stdio.h>
#include <clib/all_protos.h>

#pragma region Prototypes

struct Window* openFirstWindow();
void mainWindowEventLoop(struct Window* win);
void exitWithError(char* text);
void exitNormally();
void closeResources();

#pragma endregion


int main(int argc, char** argv) {
    if (argc == 0) {
        printf("Started from workbench\n");
    } else  {
        printf("Started from cli");
    }

    struct Window* mainWindow = openFirstWindow();
    if (mainWindow == NULL) {
        exitWithError("Fehler beim Öffnen des Fensters!");
    }

    mainWindow->RPort->cp_x = 20;
    mainWindow->RPort->cp_y = 20;
    SetAPen(mainWindow->RPort, 1);
    Text(mainWindow->RPort, "Hallo\0", 5);
    Draw(mainWindow->RPort, 20, 20);

    mainWindowEventLoop(mainWindow);
    CloseWindow(mainWindow);
    exitNormally();
}

struct Window* openFirstWindow() {
    return OpenWindowTags(NULL,
        WA_Left, 50,
        WA_Top, 160,
        WA_Width, 320,
        WA_Height, 200,
        WA_Title, "Ich bin ein zweites Testfenster",
        WA_IDCMP, IDCMP_CLOSEWINDOW,
        WA_MaxWidth, 640,
        WA_MaxHeight, 256,
        WA_GimmeZeroZero, TRUE,
        WA_Flags, WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_SIZEGADGET | WFLG_ACTIVATE,
        TAG_DONE
    );
}

void mainWindowEventLoop(struct Window* win) {
    struct IntuiMessage* msg;
    BOOL done = FALSE;

    while (!done) {
        WaitPort(win->UserPort);
        while (!done && (msg = (struct IntuiMessage*)GetMsg(win->UserPort))) {
            if (msg->Class == IDCMP_CLOSEWINDOW) done = TRUE;
            ReplyMsg((struct Message*)msg);
        }
    }
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

void closeResources() {
    // libauto
}
