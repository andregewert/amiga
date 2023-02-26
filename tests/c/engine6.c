#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <devices/timer.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/timer_protos.h>
#include <clib/alib_protos.h>
#include <stdlib.h>

/* our system libraries addresses */
struct GfxBase* GfxBase = 0;
struct IntuitionBase* IntuitionBase = 0;
struct Library* TimerBase = 0;

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
	FLOAT currentStep;

	struct MsgPort* timerPort;
	struct timerequest* timerIO;
	struct timerequest tickRequest;
	struct timerequest getTimeRequest;
	struct timeval lastRenderTime;
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
		struct tPoint pos;
		struct timeval diff;
		FLOAT secondsPassed;
		ULONG micros;

		/* get our current system time */
		rd->getTimeRequest.tr_node.io_Command = TR_GETSYSTIME;
		rd->getTimeRequest.tr_node.io_Flags = IOF_QUICK;
		DoIO((struct IORequest*)&rd->getTimeRequest);

		/* get the time passed since our last render call */
		diff = rd->getTimeRequest.tr_time;
		SubTime(&diff, &rd->lastRenderTime);

		if(diff.tv_secs)
		{
			micros = diff.tv_secs * 1000000l;
		}
		else
		{
			micros = 0;
		}
		micros += diff.tv_micro;
		secondsPassed = ((FLOAT)micros) / 1000000.0f;


		rd->lastRenderTime = rd->getTimeRequest.tr_time;

		/* we make a local copy of our RastPort pointer for ease of use */
		rastPort = &rd->renderPort;

		/* clear our bitmap */
		SetRast(rastPort, 0);

		/* now draw our line pattern */
		maxPos.x = rd->backBufferSize.x - 1;
		maxPos.y = rd->backBufferSize.y - 1;

		/* we do a quarter rotate every four seconds */
		rd->currentStep += (secondsPassed / 4.0f);
		while(rd->currentStep >= 1.0f)
		{
			rd->currentStep -= 1.0f;
		}

		pos.x = (WORD)(rd->currentStep * (FLOAT)maxPos.x);
		pos.y = (WORD)(rd->currentStep * (FLOAT)maxPos.y);

		SetAPen(rastPort, 1);
		Move(rastPort, 0l,                       (LONG)pos.y             );
		Draw(rastPort, (LONG)(maxPos.x - pos.x), 0l                      );
		Draw(rastPort, (LONG)maxPos.x,           (LONG)(maxPos.y - pos.y));
		Draw(rastPort, (LONG)pos.x,              (LONG)maxPos.y          );
		Draw(rastPort, 0l,                       (LONG)pos.y             );
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
	ULONG tickSig;
	ULONG signals;
	BOOL tickRequestPending;

	/* remember the window port in a local variable for more easy use */
	winport = rd->window->UserPort;

	/* create our waitmask for the window port */
	winSig = 1 << winport->mp_SigBit;

	/* create our waitmask for the timer port */
	tickSig = 1 << rd->timerPort->mp_SigBit;

	/* combine them to our final waitmask */
	signals = winSig | tickSig | SIGBREAKF_CTRL_C;

	/* we need to compute our output size initially */
	ComputeOutputSize(rd);

	/* we start with a no-time request so we receive a tick immediatly
	 * (we have to set 2 micros because of a bug in timer.device for 1.3) */
	rd->tickRequest.tr_time.tv_secs = 0;
	rd->tickRequest.tr_time.tv_micro = 2;
	SendIO((struct IORequest*)&rd->tickRequest);
	tickRequestPending = TRUE;

	/* enter our main loop */
	while(rd->run && tickRequestPending)
	{
		ULONG sig;
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
				/* Rendering failed, do not repaint.. */
				rd->doRepaint = FALSE;

				/* but signal ourself to leave instead */
				Signal(FindTask(0), SIGBREAKF_CTRL_C);
			}
		}

		if(rd->doRepaint)
		{
			RepaintWindow(rd);
			rd->doRepaint = FALSE;
		}

		sig = Wait(signals);

		if(sig & winSig)
		{
			/* our window signaled us, so let's harvest all its messages
			 * in a loop... */
			while((msg = GetMsg(winport)))
			{
				/* ...and dispatch and reply each of them */
				DispatchWindowMessage(rd, (struct IntuiMessage*)msg);
				ReplyMsg(msg);
			}
		}

		if(sig & tickSig)
		{
			/* our tickRequest signalled us, let's remove it from the
			 * replyport */
			WaitIO((struct IORequest*)&rd->tickRequest);

			if(rd->run)
			{
				/* if we are running then we immediately request another
				 * tick... */
				rd->tickRequest.tr_time.tv_secs = 0;
				rd->tickRequest.tr_time.tv_micro = 1000000 / 25;
				SendIO((struct IORequest*)&rd->tickRequest);
				rd->doRender = TRUE;
			}
			else
			{
				/* ... if not we acknowledge that our tickRequest returned */
				tickRequestPending = FALSE;
			}
		}

		if(sig & SIGBREAKF_CTRL_C)
		{
			/* we leave on CTRL-C */
			rd->run = FALSE;
		}

		if(!rd->run && tickRequestPending)
		{
			/* We want to leave, but there is still a tick request pending?
			 * Let's abort it */
			AbortIO((struct IORequest*)&rd->tickRequest);
		}
	}

	if(rd->backBuffer)
	{
		MyFreeBitMap(rd->backBuffer);
		rd->backBuffer = 0;
	}

	return rd->returnCode;
}

void FreeTimerDevice(struct RenderEngineData* rd)
{
	/* close the timer.device */
	if(TimerBase)
	{
		CloseDevice((struct IORequest*)rd->timerIO);
		TimerBase = 0;
	}

	/* free our timerequest */
	if(rd->timerIO)
	{
		DeleteExtIO((struct IORequest*)rd->timerIO);
		rd->timerIO = 0;
	}

	/* free our message port */
	if(rd->timerPort)
	{
		DeletePort(rd->timerPort);
		rd->timerPort = 0;
	}
}

int InitTimerDevice(struct RenderEngineData* rd)
{
	/* we do not return success until we've opened the timer.device */
	BOOL result = RETURN_FAIL;

	/* create a message port through which we will communicate with the
	 * timer.device */
	if((rd->timerPort = CreatePort(0, 0)))
	{
		/* create a timerequest which we will we pass between the
		 * timer.device and ourself */
		rd->timerIO = (struct timerequest*)
		              CreateExtIO(rd->timerPort, sizeof(struct timerequest));
		if(rd->timerIO)
		{
			/* open the timer.device */
			if(OpenDevice(TIMERNAME, UNIT_MICROHZ,
			              (struct IORequest*)rd->timerIO, 0) == 0)
			{
				/* Success: let's set the TimerBase so we can call
				 * timer.device's functions */
				TimerBase = (struct Library*)rd->timerIO->tr_node.io_Device;
				result = RETURN_OK;
			}
		}
	}
	if(result != RETURN_OK)
	{
		/* in case of an error: cleanup immediatly */
		FreeTimerDevice(rd);
	}

	return result;
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
		result = InitTimerDevice(rd);

		if(result == RETURN_OK)
		{
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

			/* setup our tick request */
			rd->tickRequest = *rd->timerIO;
			rd->tickRequest.tr_node.io_Command = TR_ADDREQUEST;

			/* setup our getTime request... */
			rd->getTimeRequest = *rd->timerIO;

			/* ... get the current time... */
			rd->getTimeRequest.tr_node.io_Command = TR_GETSYSTIME;
			rd->getTimeRequest.tr_node.io_Flags = IOF_QUICK;
			DoIO((struct IORequest*)&rd->getTimeRequest);

			/* ... and initialize our lastRenderTime */
			rd->lastRenderTime = rd->getTimeRequest.tr_time;

			/* now let's open our window */
			if((rd->window = OpenWindow(&newWindow)))
			{
				/* the main loop will run as long this is TRUE */
				rd->run = TRUE;

				result = MainLoop(rd);

				/* cleanup: close the window */
				CloseWindow(rd->window);
				rd->window = 0;
			}
			FreeTimerDevice(rd);
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
