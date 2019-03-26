#include <string.h>
#include <photon/PtProto.h>
#include <photon/PtWindow.h>
#include "game.h"
#include "render.h"

#define BYTES_PER_PIXEL 4

const char* WINDOWNAME = "Tetris";

PtWidget_t* Window;
char* Scene;

bool_t M_Init () {
	if (PtInit (NULL) == -1)
		return FALSE;

	if ((Window = PtCreateWidget (PtWindow, Pt_NO_PARENT, 0, NULL)) == NULL)
		return FALSE;

	PtSetResource (Window, Pt_ARWindow_TITLE, WINDOWNAME, 0);
	PtSetResource (Window, Pt_ARWindow_RENDER_FLAGS, Pt_FALSE, Ph_WM_RENDER_MIN);
	PtSetResource (Window, Pt_ARWindow_RENDER_FLAGS, Pt_FALSE, Ph_WM_RENDER_MAX);
	PtSetResource (Window, Pt_ARWindow_RENDER_FLAGS, Pt_FALSE, Ph_WM_RENDER_COLLAPSE);
	PtSetResource (Window, Pt_ARWindow_RENDER_FLAGS, Pt_FALSE, Ph_WM_RENDER_RESIZE);
	
	PtSetResource (Window, Pt_ARWindow_MANAGED_FLAGS, Pt_FALSE, Ph_WM_COLLAPSE);
	PtSetResource (Window, Pt_ARWindow_MANAGED_FLAGS, Pt_FALSE, Ph_WM_MAX);
	PtSetResource (Window, Pt_ARWindow_MANAGED_FLAGS, Pt_FALSE, Ph_WM_RESIZE);

	PtSetResource (Window, Pt_ARG_HEIGHT, R_GetWindowHeight (), 0);
	PtSetResource (Window, Pt_ARG_WIDTH, R_GetWindowWidth (), 0);

	PtRealizeWidget (Window);

	memset (Scene, 0, R_GetWindowWidth () * R_GetWindowHeight () * BYTES_PER_PIXEL);
	R_Init();
	GM_Init (S_PLAYING);

	return TRUE;
}

int M_RenderScene () {
	return 0;
}

int M_MainLoop () {
	bool_t exit;
	PhEvent_t* event;
	unsigned int evsize;

	exit = FALSE;
	evsize = sizeof (PhEvent_t);

	if ((event = (PhEvent_t*) malloc (evsize)) == NULL)
		return -1;

	memset (event, 0, evsize);
	while (!exit) {
		switch (PhEventPeek (event, evsize)) {
			case Ph_EVENT_MSG:
				PtEventHandler (event);
				break;

			case Ph_RESIZE_MSG:
				evsize = PhGetMsgSize (event);
				free (event);

				if ((event = (PhEvent_t*) malloc (evsize)) == NULL)
					return -1;

				break;
			case -1:
				free (event);
				return -1;
				break;
		};
		
		GM_Update ();

		if (!M_RenderScene ())
			return -1;
	}

	PtExit (0);

	return 0; 
}

void M_Terminate() {
}

int main(int argc, char** argv) {
	if (!M_Init ())
		return -1;	

	M_MainLoop ();
	M_Terminate ();

	return 0;
}
