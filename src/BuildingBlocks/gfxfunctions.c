/*
 * Copyright (c) 2024 André Gewert <agewert@ubergeek.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <proto/ttengine.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/guigfx_lib.h>

void mainWindowEventLoop(struct Window* win);
void drawTest(struct Window* win);
void guiGfxTest(struct Window* win);
struct Library* TTEngineBase;
struct Library* GuiGFXBase;

int main(int argc, char* argv[]) {
    struct Window* window = OpenWindowTags(
        NULL,
        WA_Left, 100,
        WA_Top, 100,
        WA_Width, 400,
        WA_Height, 300,
        WA_MinHeight, 200,
        WA_MinWidth, 300,
        WA_Title, (ULONG) "My Window",
        /*
        WA_CloseGadget, TRUE,
        WA_DragBar, TRUE,
        WA_DepthGadget, TRUE,
        WA_Activate, TRUE,
        WA_SmartRefresh, TRUE,
        */
        WA_IDCMP, IDCMP_CLOSEWINDOW | IMSGCODE_INTUIWHEELDATA,
        WA_Flags, WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_SMART_REFRESH,
        TAG_END
    );
    if (window == NULL) {
        fprintf(stderr, "Failed to open window\n");
        return 1;
    }

    TTEngineBase = OpenLibrary("ttengine.library", 5);
    if (!TTEngineBase) {
        fprintf(stderr, "Could not open TTEngine!\n");
        CloseWindow(window);
        return 1;
    }

    GuiGFXBase = OpenLibrary("guigfx.library", 0);
    if (!GuiGFXBase) {
        fprintf(stderr, "Could not open guigfx library!\n");
        CloseLibrary(TTEngineBase);
        CloseWindow(window);
        return 1;
    }

    drawTest(window);
    guiGfxTest(window);
    mainWindowEventLoop(window);
    CloseWindow(window);
    CloseLibrary(TTEngineBase);
    CloseLibrary(GuiGFXBase);
    return 0;
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

void drawTest(struct Window* win) {
    struct RastPort* rastPort = win->RPort;
    LONG pen = ObtainBestPen(win->WScreen->ViewPort.ColorMap, 0xFFFFFFFF, 0x00000000, 0x00000000, TAG_END);

    if (pen >= 0) {
        printf("Obtained pen is %d\n", pen);
        SetAPen(rastPort, (ULONG)pen);
    } else {
        fprintf(stderr, "Could not obtain desired pen\n");
        SetAPen(rastPort, 1);
    }
    rastPort->cp_x = 20;
    rastPort->cp_y = 40;
    Text(rastPort, "Hallo\0", 5);
    Draw(rastPort, 60, 60);

    //STRPTR table[] = {"Bitstream Vera Serif", "serif", "default", NULL};
    APTR font = TT_OpenFont(
            TT_FontFile, (ULONG)"sys:fonts/truetype/VeraSerif.ttf",
            //TT_FamilyTable, (ULONG)table,
            TT_FontStyle, TT_FontStyle_Regular,
            TT_FontWeight, TT_FontWeight_Normal,
            TT_FontSize, 48,
            TAG_END
    );
    if (font == NULL) {
        fprintf(stderr, "Could not load tt font!\n");
    } else {
        TT_SetFont(rastPort, font);
        TT_SetAttrs(
                rastPort,
                TT_Window, (ULONG) win,
                TAG_END
        );
        Move(rastPort, 40, 80);
        SetDrMd(rastPort, JAM1);
        TT_Text(rastPort, "Hello world!", 12);
        TT_DoneRastPort(rastPort);
        TT_CloseFont(font);
    }

    if (pen >= 0) {
        ReleasePen(win->WScreen->ViewPort.ColorMap, (ULONG)pen);
    }
}

void guiGfxTest(struct Window* win) {
    APTR picture = NULL, psm = NULL, drawhandle = NULL;

    picture = LoadPicture((STRPTR)"test.iff", TAG_DONE);
    psm = CreatePenShareMap(TAG_DONE);

    if (psm && picture) {
        if (AddPicture(psm, picture, TAG_DONE)) {
            drawhandle = ObtainDrawHandle(psm, win->RPort,
                                          win->WScreen->ViewPort.ColorMap, TAG_DONE);
        }
    } else {
        if (picture == NULL) {
            fprintf(stderr, "Could not open iff file\n");
        }
        if (psm  == NULL) {
            fprintf(stderr, "Could not create pen share map\n");
        }
    }

    if (drawhandle) {
        DrawPicture(drawhandle, picture, 0, 0, TAG_DONE);
    } else {
        fprintf(stderr, "Could not obtain draw handle\n");
    }

    if (psm) {
        DeletePenShareMap(psm);
    }
    if (picture) {
        DeletePicture(picture);
    }
    if (drawhandle) {
        ReleaseDrawHandle(drawhandle);
    }
}
