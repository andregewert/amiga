/*
 Copyright (c) 2023 André Gewert <agewert@ubergeek.de>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <clib/all_protos.h>
#include "functions.h"

#pragma region Prototypes

void closeApp();
void openMainWindow();
void refreshGadgets();
void removeGadget(struct Gadget *remove);

#pragma endregion

struct Screen *pubScreen;
struct Window* mainWindow;
struct Gadget *gadgetList = NULL;
APTR visualInfo;

struct {
    struct Window* window;
    struct Gadget* gadget;
} windowListItem;

int main(int argc, char** argv) {
    BOOL close = FALSE;
    struct IntuiMessage *msg;
    ULONG msgClass;

    pubScreen = LockPubScreen(NULL);
    if (pubScreen == NULL) {
        closeApp();
    }

    visualInfo = GetVisualInfo(pubScreen, TAG_DONE);
    if (visualInfo == NULL) {
        closeApp();
    }

    openMainWindow();
    refreshGadgets();

    while (!close) {
        Wait(1L << mainWindow->UserPort->mp_SigBit);
        msg = GT_GetIMsg(mainWindow->UserPort);
        msgClass = msg->Class;
        GT_ReplyIMsg(msg);

        printf("Message class: %d\n", msgClass);

        if (msgClass == IDCMP_CLOSEWINDOW) {
            close = TRUE;
        }

        if (msgClass == IDCMP_GADGETUP) {
            struct Gadget* pressed = msg->IAddress;
            printf("Button pressed: %d\n", pressed->GadgetID);
            if (pressed->GadgetID == 1) {
                removeGadget(pressed->NextGadget);
            }
        }
    }

    closeApp();
    return EXIT_SUCCESS;
}

/**
 * Should be complete, but does not redraw the window's contents
 */
void removeGadget(struct Gadget* remove) {
    if (remove == NULL) return;
    struct Gadget* current = gadgetList;
    struct Gadget *next = NULL;
    while (current->NextGadget != NULL) {
        next = current->NextGadget;
        if (next == remove) {
            current->NextGadget = next->NextGadget;
            next->NextGadget = NULL;
            RemoveGadget(mainWindow, next);
            FreeGadgets(next);
            RefreshGList(gadgetList, mainWindow, NULL, -1);
            GT_RefreshWindow(mainWindow, NULL);
            return;
        }
        current = next;
    }
}

void refreshGadgets() {
    if (mainWindow == NULL) return;

    if (gadgetList != NULL) {
        RemoveGList(mainWindow, gadgetList, -1);
        FreeGadgets(gadgetList);
        gadgetList = NULL;
    }

    struct Gadget *currentGadget = CreateContext(&gadgetList);
    if (currentGadget == NULL) {
        printf("Could not create glist!\n");
        return;
    }

    for (int i = 0; i < 3; i++) {
        printf("Creating gadget %d\n", i);

        struct NewGadget newGadget;
        newGadget.ng_LeftEdge = i *40;
        newGadget.ng_TopEdge = pubScreen->BarHeight;
        newGadget.ng_Width = 38;
        newGadget.ng_Height = pubScreen->BarHeight;
        newGadget.ng_GadgetText = "Test";
        newGadget.ng_TextAttr = pubScreen->Font;
        newGadget.ng_GadgetID = i;
        newGadget.ng_Flags = PLACETEXT_IN;
        newGadget.ng_VisualInfo = visualInfo;
        newGadget.ng_UserData = NULL;
        currentGadget = CreateGadgetA(
            BUTTON_KIND,
            currentGadget,
            &newGadget,
            TAG_DONE
        );
    }
    AddGList(mainWindow, gadgetList, 0, -1, NULL);
    RefreshGList(gadgetList, mainWindow, NULL, -1);
    GT_RefreshWindow(mainWindow, NULL);
}

void closeApp() {
    printf("Freeing resources ...");
    if (mainWindow != NULL) {
        CloseWindow(mainWindow);
    }
    if (gadgetList != NULL) {
        FreeGadgets(gadgetList);
    }
    if (visualInfo != NULL)
    {
        FreeVisualInfo(visualInfo);
    }
    if (pubScreen != NULL)
    {
        UnlockPubScreen(NULL, pubScreen);
    }
    printf(" done!\n");
}

void openMainWindow() {
    if (pubScreen == NULL || mainWindow != NULL) return;

    WORD winWidth = pubScreen->Width;
    WORD winHeight = pubScreen->BarHeight *2;

    struct TextAttr winFont = {
        pubScreen->Font->ta_Name,
        pubScreen->Font->ta_YSize,
        pubScreen->Font->ta_Style,
        pubScreen->Font->ta_Flags
    };

    mainWindow = OpenWindowTags(
        NULL,
        WA_Left, 0,
        WA_Top, 0,
        WA_Width, winWidth,
        WA_Height, winHeight,
        WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_GADGETUP,
        WA_Flags, WFLG_ACTIVATE | WFLG_BORDERLESS | WFLG_SMART_REFRESH | WFLG_CLOSEGADGET,
        WA_Title, "SystemBar",
        TAG_DONE
    );
}
