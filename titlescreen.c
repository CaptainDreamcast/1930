#include "titlescreen.h"

#include <tari/mugenanimationhandler.h>
#include <tari/screeneffect.h>
#include <tari/input.h>

#include "storyscreen.h"
#include "gamescreen.h"

static struct {
	MugenSpriteFile mSprites;

	int mAnimationID;
} gData;

static void loadTitleScreen() {
	instantiateActor(getMugenAnimationHandlerActorBlueprint());

	gData.mSprites = loadMugenSpriteFileWithoutPalette("assets/title/TITLE.sff");
	gData.mAnimationID = addMugenAnimation(createOneFrameMugenAnimationForSprite(1, 0), &gData.mSprites, makePosition(0, 0, 1));

	addFadeIn(30, NULL, NULL);
}

static void gotoIntroScreen(void* tCaller) {
	resetGameState();
	setStoryDefinitionFile("assets/story/INTRO.def");
	setNewScreen(&StoryScreen);
}

static void updateTitleScreen() {
	if (hasPressedStartFlank() || hasShotGunFlank()) {
		addFadeOut(30, gotoIntroScreen, NULL);
	}

	if (hasPressedAbortFlank()) {
		abortScreenHandling();
	}
}

Screen TitleScreen = {
	.mLoad = loadTitleScreen,
	.mUpdate = updateTitleScreen,
};