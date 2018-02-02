#include <tari/framerateselectscreen.h>
#include <tari/pvr.h>
#include <tari/physics.h>
#include <tari/file.h>
#include <tari/drawing.h>
#include <tari/log.h>
#include <tari/wrapper.h>
#include <tari/system.h>
#include <tari/stagehandler.h>
#include <tari/logoscreen.h>
#include <tari/mugentexthandler.h>

#include "titlescreen.h"
#include "gamescreen.h"
#include "tweethandler.h"
#include "characterhandler.h"

#ifdef DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

#endif


void exitGame() {
	shutdownTariWrapper();

#ifdef DEVELOP
	abortSystem();
#else
	returnToMenu();
#endif
}

void setMainFileSystem() {
#ifdef DEVELOP
	setFileSystem("/pc");
#else
	setFileSystem("/cd");
#endif
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	setGameName("2018");
	setScreenSize(640, 480);
	
	initTariWrapperWithDefaultFlags();
	setFont("$/rd/fonts/segoe.hdr", "$/rd/fonts/segoe.pkg");
	loadMugenTextHandler();
	addMugenFont(1, "Myriad-Pro-Semibold.def");
	addMugenFont(2, "Myriad-Pro-Bold-Condensed.def");
	addMugenFont(3, "Myriad-Pro-Bold-Condensed-small.def");
	addMugenFont(10, "Myriad-Pro-Regular.def");
	addMugenFont(11, "Myriad-Pro-Regular20.def");

	logg("Check framerate");
	FramerateSelectReturnType framerateReturnType = selectFramerate();
	if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
		exitGame();
	}

	setMainFileSystem();
	
	loadCharacters();
	loadTweetArchive();
	resetGameState();
	setScreenAfterWrapperLogoScreen(&TitleScreen);
	startScreenHandling(&TitleScreen);

	exitGame();
	
	return 0;
}


