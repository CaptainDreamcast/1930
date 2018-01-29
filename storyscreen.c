#include "storyscreen.h"

#include <assert.h>

#include <tari/log.h>
#include <tari/system.h>
#include <tari/animation.h>
#include <tari/math.h>
#include <tari/input.h>
#include <tari/screeneffect.h>
#include <tari/mugendefreader.h>
#include <tari/mugenspritefilereader.h>
#include <tari/mugenanimationreader.h>
#include <tari/mugenanimationhandler.h>
#include <tari/mugentexthandler.h>

#include "titlescreen.h"
#include "gamescreen.h"

static struct {
	MugenDefScript mScript;
	MugenDefScriptGroup* mCurrentGroup;
	MugenSpriteFile mSprites;

	MugenAnimation* mOldAnimation;
	MugenAnimation* mAnimation;
	int mAnimationID;
	int mOldAnimationID;

	Position mOldAnimationBasePosition;
	Position mAnimationBasePosition;

	int mSpeakerID;
	int mTextID;

	int mIsStoryOver;

	char mDefinitionPath[1024];
} gData;

static int isImageGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Image", firstW);
}

static void increaseGroup() {
	gData.mCurrentGroup = gData.mCurrentGroup->mNext;
}

static void loadImageGroup() {
	if (gData.mOldAnimationID != -1) {
		removeMugenAnimation(gData.mOldAnimationID);
		destroyMugenAnimation(gData.mOldAnimation);
	}

	if (gData.mAnimationID != -1) {
		setMugenAnimationBasePosition(gData.mAnimationID, &gData.mOldAnimationBasePosition);
	}

	gData.mOldAnimationID = gData.mAnimationID;
	gData.mOldAnimation = gData.mAnimation;


	int group = getMugenDefNumberVariableAsGroup(gData.mCurrentGroup, "group");
	int item = getMugenDefNumberVariableAsGroup(gData.mCurrentGroup, "item");
	gData.mAnimation = createOneFrameMugenAnimationForSprite(group, item);
	gData.mAnimationID = addMugenAnimation(gData.mAnimation, &gData.mSprites, makePosition(0, 0, 0));
	setMugenAnimationBasePosition(gData.mAnimationID, &gData.mAnimationBasePosition);

	increaseGroup();
}

static int isTextGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Text", firstW);
}

static void loadTextGroup() {
	if (gData.mTextID != -1) {
		removeMugenText(gData.mTextID);
		removeMugenText(gData.mSpeakerID);
	}

	char* speaker = getAllocatedMugenDefStringVariableAsGroup(gData.mCurrentGroup, "speaker");
	char* text = getAllocatedMugenDefStringVariableAsGroup(gData.mCurrentGroup, "text");

	gData.mSpeakerID = addMugenText(speaker, makePosition(40, 340, 10), 11);
	setMugenTextColor(gData.mSpeakerID, COLOR_WHITE);

	int dur = strlen(text);
	gData.mTextID = addMugenText(text, makePosition(50, 360, 10), 11);
	setMugenTextTextBoxWidth(gData.mTextID, 540);
	setMugenTextBuildup(gData.mTextID, 1);

	freeMemory(speaker);
	freeMemory(text);

	increaseGroup();
}

static int isTitleGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Title", firstW);
}

static void goToTitle(void* tCaller) {
	(void)tCaller;
	setNewScreen(&TitleScreen);
}

static void loadTitleGroup() {
	gData.mIsStoryOver = 1;
	addFadeOut(30, goToTitle, NULL);
}

static int isGameGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Game", firstW);
}

static void goToGame(void* tCaller) {
	(void)tCaller;
	setNewScreen(&GameScreen);
}

static void loadGameGroup() {
	gData.mIsStoryOver = 1;
	addFadeOut(30, goToGame, NULL);
}

static void loadNextStoryGroup() {
	int isRunning = 1;
	while (isRunning) {
		if (isImageGroup()) {
			loadImageGroup();
		}
		else if (isTextGroup()) {
			loadTextGroup();
			break;
		}
		else if (isTitleGroup()) {
			loadTitleGroup();
			break;
		}
		else if (isGameGroup()) {
			loadGameGroup();
			break;
		}
		else {
			logError("Unidentified group type.");
			logErrorString(gData.mCurrentGroup->mName);
			abortSystem();
		}
	}
}

static void findStartOfStoryBoard() {
	gData.mCurrentGroup = gData.mScript.mFirstGroup;

	while (gData.mCurrentGroup && strcmp("STORYSTART", gData.mCurrentGroup->mName)) {
		gData.mCurrentGroup = gData.mCurrentGroup->mNext;
	}

	assert(gData.mCurrentGroup);
	gData.mCurrentGroup = gData.mCurrentGroup->mNext;
	assert(gData.mCurrentGroup);

	gData.mAnimationID = -1;
	gData.mOldAnimationID = -1;
	gData.mTextID = -1;

	gData.mOldAnimationBasePosition = makePosition(0, 0, 1);
	gData.mAnimationBasePosition = makePosition(0, 0, 2);

	loadNextStoryGroup();
}



static void loadStoryScreen() {
	gData.mIsStoryOver = 0;

	instantiateActor(getMugenAnimationHandlerActorBlueprint());
	instantiateActor(MugenTextHandler);

	gData.mScript = loadMugenDefScript(gData.mDefinitionPath);

	char* spritePath = getAllocatedMugenDefStringVariable(&gData.mScript, "Header", "sprites");
	gData.mSprites = loadMugenSpriteFileWithoutPalette(spritePath);
	freeMemory(spritePath);

	findStartOfStoryBoard();
}


static void updateText() {
	if (gData.mIsStoryOver) return;
	if (gData.mTextID == -1) return;

	if (hasPressedAFlankSingle(0) || hasPressedAFlankSingle(1) || hasPressedStartSingle(0) || hasPressedStartSingle(1) || hasShotGunFlank()) {
		if (isMugenTextBuiltUp(gData.mTextID)) {
			loadNextStoryGroup();
		}
		else {
			setMugenTextBuiltUp(gData.mTextID);
		}
	}
}

static void updateStoryScreen() {

	updateText();

	if (hasPressedAbortFlank()) {
		setNewScreen(&TitleScreen);
	}

}


Screen StoryScreen = {
	.mLoad = loadStoryScreen,
	.mUpdate = updateStoryScreen,
};

void setStoryDefinitionFile(char * tPath)
{
	strcpy(gData.mDefinitionPath, tPath);
}
