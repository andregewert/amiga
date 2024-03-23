/*
 * Copyright (c) 2023-2024 André Gewert <agewert@ubergeek.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "windows.h"

struct Window* sbFindWindowByName(CONST_STRPTR windowName) {
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

BOOL sbCloseWindow(struct Window* windowToClose) {
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

BOOL sbHideWindow(struct Window* windowToHide) {
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

BOOL sbUnhideWindow(struct Window* windowToUnhide) {
    BOOL closed = FALSE;
    struct Screen* wbScreen = LockPubScreen("Workbench");
    if (wbScreen != NULL) {
        struct Window* window = wbScreen->FirstWindow;
        while (window != NULL) {
            if (window == windowToUnhide) {
                ShowWindow(windowToUnhide, NULL);
                closed = TRUE;
                break;
            }
            window = window->NextWindow;
        }
    }
    UnlockPubScreen(NULL, wbScreen);
    return closed;
}
