#include <exec/exec.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/graphint.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

struct IntuitionBase* IntuitionBase;
struct GfxBase* GfxBase;

struct TextAttr NeuerFont = {
    "topaz.font",
    TOPAZ_SIXTY,
    FS_NORMAL,
    FPF_ROMFONT
};

struct NewScreen NeuerBildschirm = {
    0, 0, 320, 256, 2,
    0, 1,
    NULL, CUSTOMSCREEN,
    &NeuerFont,
    "Demo-Bildschirm",
    NULL,
    NULL
};

struct NewWindow NeuesFenster = {
    20, 20, 300, 100,
    0, 1,
    CLOSEWINDOW | NEWSIZE,
    WINDOWCLOSE | SMART_REFRESH | ACTIVATE | WINDOWSIZING | WINDOWDRAG | WINDOWDEPTH | NOCAREREFRESH | GIMMEZEROZERO,
    NULL, NULL,
    "Demo-Fenster",
    0, NULL,
    100, 25, 320, 256,
    CUSTOMSCREEN
};

int main() {
    struct Screen* Bildschirm;
    struct Window* Fenster;
    struct IntuiMessage* Meldung;
    struct RastPort* rasterport;

    LONG i;
    LONG fensterbreite, fensterhoehe;
    ULONG class;

    IntuitionBase = (struct IntuitionBase*)OpenLibrary("intuition.library", 0L);
    if (IntuitionBase == NULL) Exit(FALSE);

    GfxBase = (struct GfxBase*)OpenLibrary("graphics.library", 0L);
    if (GfxBase == NULL) {
        CloseLibrary((struct Library*)IntuitionBase);
        Exit(FALSE);
    }

    Bildschirm = (struct Screen*)OpenScreen(&NeuerBildschirm);
    if (Bildschirm == NULL) {
        CloseLibrary((struct Library*)GfxBase);
        CloseLibrary((struct Library*)IntuitionBase);
        Exit(FALSE);
    }

    NeuesFenster.Screen = Bildschirm;

    Fenster = (struct Window*)OpenWindow(&NeuesFenster);
    if (Fenster == NULL) {
        CloseScreen(Bildschirm);
        CloseLibrary((struct Library*)GfxBase);
        CloseLibrary((struct Library*)IntuitionBase);
        Exit(FALSE);
    }

    rasterport = Fenster->RPort;

    do {
        fensterbreite = Fenster->Width -1;
        fensterhoehe = Fenster->Height -1;

        SetAPen(rasterport, 2L);
        SetDrMd(rasterport, (LONG)JAM1);

        for (i = 0; i < fensterbreite +1; i = i +3) {
            Move(rasterport, fensterbreite -i, fensterhoehe);
            Draw(rasterport, i, 0L);
        }

        for (i = 0; i < fensterhoehe +1; i = i +3) {
            Move(rasterport, fensterbreite, i);
            Draw(rasterport, 0L, fensterhoehe -i);
        }

        SetDrMd(rasterport, (LONG)JAM2);
        Move(rasterport, fensterbreite /2 -45L, fensterhoehe /2 +3L);
        Text(rasterport, "Fensterdemo", 11L);

        Wait(1L << Fenster->UserPort->mp_SigBit);

        Meldung = (struct IntuiMessage*)GetMsg(Fenster->UserPort);
        class = Meldung->Class;
        ReplyMsg((struct Message*)Meldung);

        if ((class && NEWSIZE) != 0) {
            SetAPen(rasterport, 0L);
            SetDrMd(rasterport, (LONG)JAM1);
            RectFill(rasterport, 0L, 0L, fensterbreite, fensterhoehe);
        }

    } while ((class & CLOSEWINDOW) == 0);

    CloseWindow(Fenster);
    CloseScreen(Bildschirm);
    CloseLibrary((struct Library*)GfxBase);
    CloseLibrary((struct Library*)IntuitionBase);

    Exit(TRUE);
    return 0;
}
