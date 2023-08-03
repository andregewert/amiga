#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <clib/all_protos.h>

BOOL wbCloseWindow(struct Window* windowToClose);
BOOL wbHideWindow(struct Window* windowToHide);
BOOL wbUnhideWindow(struct Window* windowToUnhide);

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
            if (!strcmp(window->Title, "AmigaShell")) {
                shellWindow = window;
            }
            window = window->NextWindow;
        }
    }
    UnlockPubScreen(NULL, wbScreen);
    printf("Unlock wb\n");

    if (shellWindow != NULL) {
        printf("Unhide Shell-Fenster\n");
        wbUnhideWindow(shellWindow);
    }
    shellWindow = NULL;

    printf("Programmende\n");
    return EXIT_SUCCESS;
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