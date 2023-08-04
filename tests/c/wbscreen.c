#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <clib/all_protos.h>

#define PRA_FIR0_BIT (1 << 6)

volatile UBYTE* ciaa_pra = (volatile UBYTE*)0xbfe001;

BOOL wbCloseWindow(struct Window* windowToClose);
BOOL wbHideWindow(struct Window* windowToHide);
BOOL wbUnhideWindow(struct Window* windowToUnhide);
void waitmouse(void);
struct Window* wbFindWindowByName(CONST STRPTR windowName);

int main(int argc, char** argv) {
    struct Screen* wbScreen = LockPubScreen("Workbench");
    struct Window* shellWindow = NULL;

    printf("Screen: %d\n", wbScreen);
    if (wbScreen != NULL) {
        struct Window* window = wbScreen->FirstWindow;
        while (window != NULL) {
            if (!strcmp(window->Title, "")) {
                printf("Empty title\n");
            } else {
                printf("Fenster: %s\n", window->Title);
            }
            if (window->Flags & WFLG_BORDERLESS) {
                printf("-borderless\n");
            }
            window = window->NextWindow;
        }
    }
    printf("Unlock wb\n");
    UnlockPubScreen(NULL, wbScreen);

    WORD screenWidth = wbScreen->Width;
    WORD screenHeight = wbScreen->Height;

    printf("Open program window\n");
    struct Window* mainWindow = OpenWindowTags(NULL,
        WA_Left, 0,
        WA_Top, 160,
        WA_Width, screenWidth,
        WA_Height, 20,
        WA_Title, "Ich bin ein zweites Testfenster",
        WA_Flags, WFLG_BORDERLESS | WFLG_ACTIVATE,
        TAG_DONE
    );

    shellWindow = wbFindWindowByName("AmigaShell");
    if (shellWindow != NULL) {
        printf("Hide shell window\n");
        wbHideWindow(shellWindow);
    } else {
        printf("Did not find shell window!\n");
    }

    printf("Waiting for mouse\n");
    waitmouse();
    printf("Close program window\n");
    CloseWindow(mainWindow);

    shellWindow = wbFindWindowByName("AmigaShell");
    if (shellWindow != NULL) {
        printf("Unhide Shell-Fenster\n");
        wbUnhideWindow(shellWindow);
    } else {
        printf("Did not find shell window!\n");
    }

    printf("Programmende\n");
    return EXIT_SUCCESS;
}

struct Window* wbFindWindowByName(CONST STRPTR windowName) {
    struct Screen* wbScreen = LockPubScreen("Workbench");
    struct Window* foundWindow = NULL;

    if (wbScreen != NULL) {
        struct Window* window = wbScreen->FirstWindow;
        while (window != NULL) {
            if (!strcmp(window->Title, windowName)) {
                foundWindow = window;
                break;
            }
            window = window->NextWindow;
        }
    }
    UnlockPubScreen(NULL, wbScreen);
    return foundWindow;
}

BOOL wbCloseWindow(struct Window* windowToClose) {
    BOOL closed = FALSE;
    struct Screen* wbScreen = LockPubScreen("Workbench");
    if (wbScreen != NULL) {
        struct Window* window = wbScreen->FirstWindow;
        while (window != NULL) {
            if (window == windowToClose) {
                CloseWindow(windowToClose);
                closed = TRUE;
                break;
            }
            window = window->NextWindow;
        }
    }
    UnlockPubScreen(NULL, wbScreen);
    return closed;
}

BOOL wbHideWindow(struct Window* windowToHide) {
    BOOL closed = FALSE;
    struct Screen* wbScreen = LockPubScreen("Workbench");
    if (wbScreen != NULL) {
        struct Window* window = wbScreen->FirstWindow;
        while (window != NULL) {
            if (window == windowToHide) {
                HideWindow(windowToHide);
                closed = TRUE;
                break;
            }
            window = window->NextWindow;
        }
    }
    UnlockPubScreen(NULL, wbScreen);
    return closed;
}

BOOL wbUnhideWindow(struct Window* windowToUnhide) {
    BOOL closed = FALSE;
    struct Screen* wbScreen = LockPubScreen("Workbench");
    if (wbScreen != NULL) {
        struct Window* window = wbScreen->FirstWindow;
        while (window != NULL) {
            if (window == windowToUnhide) {
                ShowWindow(windowToUnhide);
                closed = TRUE;
                break;
            }
            window = window->NextWindow;
        }
    }
    UnlockPubScreen(NULL, wbScreen);
    return closed;
}

void waitmouse(void) {
    while ((*ciaa_pra & PRA_FIR0_BIT) != 0);
}