#include <stdio.h>
#include <stdlib.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/utility.h>
#include <mui/Clock_mcc.h>

static VOID fail(APTR app, const char *str)
{
	if (app)
		MUI_DisposeObject(app);

	if (MUIMasterBase)
		CloseLibrary(MUIMasterBase);
	if(IntuitionBase)
		CloseLibrary((struct Library *)IntuitionBase);
	if(UtilityBase)
		CloseLibrary(UtilityBase);
	if(GfxBase)
		CloseLibrary((struct Library *)GfxBase);
	if(IconBase)
		CloseLibrary(IconBase);
	if (str)
	{
		puts(str);
		exit(20);
	}
	exit(0);
}

static VOID init(VOID)
{
	if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME,MUIMASTER_VMIN)))
		fail(NULL,"Failed to open "MUIMASTER_NAME".");
	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 37);
	UtilityBase = OpenLibrary("utility.library", 37);
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 37);
	IconBase = OpenLibrary("icon.library", 37);
}

/*
A simple demo application to show some new features of MUI 5.0.
*/

int main(int argc, char *argv[])
{
	Object *app, *window;
	static const char *const UsedClasses[] =
	{
		"Clock.mcc",
		NULL
	};

	init();

	app = ApplicationObject,
		MUIA_Application_Title      , "Clock",
		MUIA_Application_Version    , "$VER: Clock 21.1 (02.01.2017)",
		MUIA_Application_Copyright  , "(C) 2015-2020 Thore Boeckelmann",
		MUIA_Application_Author     , "Thore Boeckelmann",
		MUIA_Application_Description, "Demonstrate the use of Clock.mcc in MUI 5.0",
		MUIA_Application_Base       , "CLOCK",
		MUIA_Application_UsedClasses, UsedClasses,

		SubWindow, window = WindowObject,
			MUIA_Window_Title, "Clock.mcc",
			MUIA_Window_ID   , MAKE_ID('C','L','O','K'),
			WindowContents, VGroup,
				Child, TextObject,
					MUIA_Text_Contents, "a simple analog clock",
					End,
				Child, VCenter(HCenter((ClockObject, End))),
				End,
			End,

		End;

	if (!app)
		fail(app,"Failed to create Application.");

	DoMethod(window,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
		app,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);

	set(window,MUIA_Window_Open,TRUE);

/*
** This is the ideal input loop for an object oriented MUI application.
** Everything is encapsulated in classes, no return ids need to be used,
** we just call a single method which does all the dirty work.
*/

	DoMethod(app, MUIM_Application_Run);

	set(window,MUIA_Window_Open,FALSE);


/*
** Shut down...
*/

	MUI_DisposeObject(app);     /* dispose all objects. */
	fail(NULL,NULL);            /* exit, app is already disposed. */

	// just to please the compiler
	return 0;
}
