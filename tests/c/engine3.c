#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <stdlib.h>

/* our system libraries addresses */
struct GfxBase* GfxBase = 0;
struct IntuitionBase* IntuitionBase = 0;

struct RenderEngineData
{
	struct Window* window;
	struct tPoint outputSize;
	struct BitMap* backBuffer;
	struct tPoint backBufferSize;
	struct RastPort renderPort;
	int returnCode;
	BOOL run;
	BOOL doRepaint;
	BOOL doRender;
};

void MyFreeBitMap(struct BitMap* bitmap)
{
	/* FreeBitMap() is available since OS3.0 */
	if(GfxBase->LibNode.lib_Version < 39)
	{
		ULONG width;
		WORD i;

		/* warning: this assumption is only safe for our own bitmaps */
		width = bitmap->BytesPerRow * 8;

		/* free all the bitplanes... */
		for(i = 0; i < bitmap->Depth; i++)
		{
			if(bitmap->Planes[i])
			{
				FreeRaster(bitmap->Planes[i], width, (ULONG)bitmap->Rows);
				bitmap->Planes[i] = 0;
			}
		}
		/* ... and finally free the bitmap itself */
		FreeMem(bitmap, sizeof(struct BitMap));
	}
	else
	{
		FreeBitMap(bitmap);
	}
}

struct BitMap* MyAllocBitMap(ULONG width, ULONG height,
                             ULONG depth, struct BitMap* likeBitMap)
{
	struct BitMap* bitmap;

	/* AllocBitMap() is available since OS3.0 */
	if(GfxBase->LibNode.lib_Version < 39)
	{
		/* sanity check */
		if(depth <= 8)
		{
			/* let's allocate our BitMap */
			bitmap = (struct BitMap*)
			       AllocMem(sizeof(struct BitMap), MEMF_ANY | MEMF_CLEAR);
			if(bitmap)
			{
				WORD i;
				InitBitMap(bitmap, depth, width, height);

				/* now allocate all our bitplanes */
				for(i = 0; i < bitmap->Depth; i++)
				{
					bitmap->Planes[i] = AllocRaster(width, height);
					if(!(bitmap->Planes[i]))
					{
						MyFreeBitMap(bitmap);
						bitmap = 0;
						break;
					}
				}
			}
		}
		else
		{
			bitmap = 0;
		}
	}
	else
	{
		bitmap = AllocBitMap(width, height, depth, 0, likeBitMap);
	}

	return bitmap;
}

void RepaintWindow(struct RenderEngineData* rd)
{
	/* on repaint we simply blit our backbuffer into our window's RastPort */
	BltBitMapRastPort(rd->backBuffer, 0, 0, rd->window->RPort,
	                  (LONG)rd->window->BorderLeft,
	                  (LONG)rd->window->BorderTop,
	                  (LONG)rd->outputSize.x, (LONG)rd->outputSize.y,
	                  (ABNC | ABC));
}

int PrepareBackBuffer(struct RenderEngineData* rd)
{
	int result;

	if(rd->outputSize.x != rd->backBufferSize.x ||
	   rd->outputSize.y != rd->backBufferSize.y)
	{
		/* if output size changed free our current bitmap... */
		if(rd->backBuffer)
		{
			MyFreeBitMap(rd->backBuffer);
			rd->backBuffer = 0;
		}

		/* ... allocate a new one... */
		rd->backBuffer = MyAllocBitMap((ULONG)rd->outputSize.x,
		                               (ULONG)rd->outputSize.y,
		                               1, rd->window->RPort->BitMap);
		if(rd->backBuffer)
		{
			/* and on success remember its size */
			rd->backBufferSize = rd->outputSize;
		}

		/* link the bitmap into our render port */
		InitRastPort(&rd->renderPort);
		rd->renderPort.BitMap = rd->backBuffer;
	}

	result = rd->backBuffer ? RETURN_OK : RETURN_ERROR;

	return result;
}

int RenderBackbuffer(struct RenderEngineData* rd)
{
	int result;

	result = PrepareBackBuffer(rd);

	if(result == RETURN_OK)
	{
		struct RastPort* rastPort;
		struct tPoint maxPos;
		struct tPoint lineStep;
		struct tPoint pos;
		WORD i;

		const WORD stepCount = 32;

		/* we make a local copy of our RastPort pointer for ease of use */
		rastPort = &rd->renderPort;

		/* clear our bitmap */
		SetRast(rastPort, 0);

		/* now draw our line pattern */
		maxPos.x = rd->backBufferSize.x - 1;
		maxPos.y = rd->backBufferSize.y - 1;

		lineStep.x = maxPos.x / stepCount;
		lineStep.y = maxPos.y / stepCount;

		SetAPen(rastPort, 1);
		pos.x = pos.y = 0;
		for(i = 0; i < stepCount; i++)
		{
			Move(rastPort, 0, (LONG)pos.y);
			Draw(rastPort, (LONG)(maxPos.x - pos.x), 0);
			Draw(rastPort, (LONG)maxPos.x, (LONG)(maxPos.y - pos.y));
			Draw(rastPort, (LONG)pos.x, (LONG)maxPos.y);
			Draw(rastPort, 0, (LONG)pos.y);

			pos.x += lineStep.x;
			pos.y += lineStep.y;
		}
	}

	return result;
}

void ComputeOutputSize(struct RenderEngineData* rd)
{
	/* our output size is simply the window's size minus its borders */
	rd->outputSize.x =
	rd->window->Width - rd->window->BorderLeft - rd->window->BorderRight;
	rd->outputSize.y =
	rd->window->Height - rd->window->BorderTop - rd->window->BorderBottom;
}

void DispatchWindowMessage(struct RenderEngineData* rd,
                           struct IntuiMessage* msg)
{
	switch(msg->Class)
	{
		case IDCMP_CLOSEWINDOW:
		{
			/* User pressed the window's close gadget: exit the main loop as
			 * soon as possible */
			rd->run = FALSE;
			break;
		}
		case IDCMP_NEWSIZE:
		{
			/* On resize we compute our new output size... */
			ComputeOutputSize(rd);

			/* ... and trigger a render call */
			rd->doRender = TRUE;
			break;
		}
		case IDCMP_REFRESHWINDOW:
		{
			BeginRefresh(rd->window);

			/* We do only repaint here if there is no pending
			 * render call */
			if(!rd->doRender)
			{
				RepaintWindow(rd);
			}

			EndRefresh(rd->window, TRUE);
			break;
		}
	}
}

int MainLoop(struct RenderEngineData* rd)
{
	struct MsgPort* winport;
	ULONG winSig;

	/* remember the window port in a local variable for more easy use */
	winport = rd->window->UserPort;

	/* create our waitmask for the window port */
	winSig = 1 << winport->mp_SigBit;

	/* paint our window for the first time */
	rd->doRender = TRUE;

	/* we need to compute our output size initially */
	ComputeOutputSize(rd);

	/* enter our main loop */
	while(rd->run)
	{
		struct Message* msg;

		if(rd->doRender)
		{
			rd->returnCode = RenderBackbuffer(rd);
			if(rd->returnCode == RETURN_OK)
			{
				/* Rendering succeeded, we need to repaint */
				rd->doRepaint = TRUE;
				rd->doRender = FALSE;
			}
			else
			{
				/* Rendering failed, do not repaint, leave our main
				 * loop instead */
				rd->doRepaint = FALSE;
				rd->run = FALSE;
			}
		}

		if(rd->doRepaint)
		{
			RepaintWindow(rd);
			rd->doRepaint = FALSE;
		}

		if(rd->run)
		{
			/* let's sleep until a message from our window arrives */
			Wait(winSig);
		}

		/* our window signaled us, so let's harvest all its messages
		 * in a loop... */
		while((msg = GetMsg(winport)))
		{
			/* ...and dispatch and reply each of them */
			DispatchWindowMessage(rd, (struct IntuiMessage*)msg);
			ReplyMsg(msg);
		}
	}

	if(rd->backBuffer)
	{
		MyFreeBitMap(rd->backBuffer);
		rd->backBuffer = 0;
	}

	return rd->returnCode;
}

int RunEngine(void)
{
	struct RenderEngineData* rd;

	/* as long we did not enter our main loop we report an error */
	int result = RETURN_ERROR;

	/* allocate the memory for our runtime data and ititialize it
	 * with zeros */
	if((rd = (struct RenderEngineData*)
	        AllocMem(sizeof(struct RenderEngineData), MEMF_ANY | MEMF_CLEAR)))
	{
		/* now let's open our window */
		static struct NewWindow newWindow =
		{
			0, 14,
			320, 160,
			(UBYTE)~0, (UBYTE)~0,
			IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW,
			WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET |
			WFLG_SIMPLE_REFRESH | WFLG_SIZEBBOTTOM | WFLG_SIZEGADGET,
			0, 0,
			"Gfx Workshop",
			0,
			0,
			96, 48,
			(UWORD)~0, (UWORD)~0,
			WBENCHSCREEN
		};
		if((rd->window = OpenWindow(&newWindow)))
		{
			/* the main loop will run as long this is TRUE */
			rd->run = TRUE;

			result = MainLoop(rd);

			/* cleanup: close the window */
			CloseWindow(rd->window);
			rd->window = 0;
		}

		/* free our runtime data */
		FreeMem(rd, sizeof(struct RenderEngineData));
		rd = 0;
	}

	return result;
}

int main(int argc, char* argv[])
{
	/* as long we did not execute RunEngine() we report a failure */
	int result = RETURN_FAIL;

	/* we need at least 1.2 graphic.library's drawing functions */
	if((GfxBase = (struct GfxBase*)OpenLibrary("graphics.library", 33)))
	{
		/* we need at least 1.2 intuition.library for our window */
		if((IntuitionBase = (struct IntuitionBase*)
		                   OpenLibrary("intuition.library", 33)))
		{
			/* All libraries needed are available, so let's run... */
			result = RunEngine();

			CloseLibrary((struct Library*)IntuitionBase);
			IntuitionBase = 0;
		}
		CloseLibrary((struct Library*)GfxBase);
		GfxBase = 0;
	}

	/* some startup codes do ignore main's return value, that's
	 * why we use exit() here instead of a simple return */
	exit(result);
}
