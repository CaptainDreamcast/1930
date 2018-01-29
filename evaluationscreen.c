#include "evaluationscreen.h"

#include <tari/mugenanimationhandler.h>
#include <tari/input.h>
#include <tari/mugentexthandler.h>
#include <tari/tweening.h>
#include <tari/timer.h>
#include <tari/screeneffect.h>

#include "gamescreen.h"
#include "tweethandler.h"

#include "storyscreen.h"
#include "titlescreen.h"

static struct {
	MugenSpriteFile* mSprites;
	MugenAnimations* mAnimations;

	int mDay;
	int mDayID;

	int mTallyupTextID;

	int mStage;

	int mCurrentTweet;
	int mTweetAmount;
	int mResultAnimationID;

	int mBarBGID;
	int mBarID;
	int mBombID;

	int mMoneyTextID;
} gData;

static void startMissflaggedPosts();

static void loadEvaluationScreen() {
	instantiateActor(getMugenAnimationHandlerActorBlueprint());
	instantiateActor(MugenTextHandler);

	gData.mDay = getGameDay();

	loadSpritesAndAnimations();
	gData.mSprites = getGameSprites();
	gData.mAnimations = getGameAnimations();

	char text[200];
	sprintf(text, "Day %d", gData.mDay);
	gData.mDayID = addMugenText(text, makePosition(298, 47, 21), 1);
	setMugenTextColor(gData.mDayID, COLOR_WHITE);

	sprintf(text, "TALLY-UP");
	gData.mTallyupTextID = addMugenText(text, makePosition(275, 79, 21), 1);
	setMugenTextColor(gData.mTallyupTextID, COLOR_WHITE);

	gData.mStage = 0;
	startMissflaggedPosts();
}

static void gotoBadEnding1(void* tCaller) {
	setStoryDefinitionFile("assets/story/ENDING1.def");
	setNewScreen(&StoryScreen);
}

static void gotoBadEnding2(void* tCaller) {
	setStoryDefinitionFile("assets/story/ENDING2.def");
	setNewScreen(&StoryScreen);
}

static void gotoGoodEnding(void* tCaller) {
	setStoryDefinitionFile("assets/story/ENDING3.def");
	setNewScreen(&StoryScreen);
}

static void startEnding(void(*tCB)(void*)) {
	addFadeOut(30, tCB, NULL);
}

static void gotoGameScreen(void* tCaller) {
	setNewScreen(&GameScreen);
}

static void gotoNextDay(void* tCaller) {
	if (gData.mDay == 7) {
		startEnding(gotoGoodEnding);
		return;
	}

	addFadeOut(30, gotoGameScreen, NULL);
}

static void moneyDisplayFinished(void* tCaller) {
	int delta = gData.mTweetAmount * 1000000;
	decreaseMoney(delta);
	
	if (getCurrentMoney() == 0) {
		startEnding(gotoBadEnding1);
		return;
	}
	
	gotoNextDay(NULL);
}

static void showMoneyResult(void* tCaller) {
	int delta = gData.mTweetAmount * 1000000;
	int result = max(0, getCurrentMoney() - delta);
	char text[200];
	sprintf(text, "$%d - $%d = $%d", getCurrentMoney(), gData.mTweetAmount * 1000000, result);
	changeMugenText(gData.mMoneyTextID, text);

	addTimerCB(180, moneyDisplayFinished, NULL);
}

static void showMoneyCalculations(void* tCaller) {
	char text[200];
	sprintf(text, "$%d - $%d", getCurrentMoney(), gData.mTweetAmount * 1000000);
	changeMugenText(gData.mMoneyTextID, text);

	addTimerCB(120, showMoneyResult, NULL);
}

static void showTwitterMoneyChange() {
	char text[200];
	sprintf(text, "$%d", getCurrentMoney());
	gData.mMoneyTextID = addMugenText(text, makePosition(120, 240, 21), 1);

	addTimerCB(120, showMoneyCalculations, NULL);
}

static void deletedTweetInCenter(void* tCaller);
static void showNextDeletedTweet() {
	Tweet t = getDeletedTweet(gData.mCurrentTweet);
	loadTweet(t, deletedTweetInCenter);
}

static void deletedTweetMovedLeft(void* tCaller) {
	unloadCurrentTweet();
	gData.mCurrentTweet++;

	if (gData.mCurrentTweet >= gData.mTweetAmount) {
		showTwitterMoneyChange();
		return;
	}
	else {
		showNextDeletedTweet();
	}
}

static void moveDeletedTweetLeft(void* tCaller) {
	moveTweetLeft(deletedTweetMovedLeft);
}

static void deletedTweetInCenter(void* tCaller) {
	addTimerCB(120, moveDeletedTweetLeft, NULL);
}

static void moveDeletedAnimationFinished(void* tCaller) {
	showNextDeletedTweet();
}

static void moveDeletedPointerDown(void* tCaller) {
	tweenDouble(&getMugenAnimationPositionReference(gData.mResultAnimationID)->y, getMugenAnimationPositionReference(gData.mResultAnimationID)->y, getMugenAnimationPositionReference(gData.mResultAnimationID)->y + 150, linearTweeningFunction, 30, moveDeletedAnimationFinished, NULL);
}

static void startDeletedPosts() {
	gData.mTweetAmount = getDeletedTweetAmountAndLoadDeletedTweets(gData.mDay);
	gData.mCurrentTweet = 0;

	if (gData.mTweetAmount) {
		gData.mResultAnimationID = addMugenAnimation(getMugenAnimation(gData.mAnimations, 42), gData.mSprites, makePosition(320, 240, 10));
		addTimerCB(180, moveDeletedPointerDown, NULL);
	}
	else {
		gData.mResultAnimationID = addMugenAnimation(getMugenAnimation(gData.mAnimations, 43), gData.mSprites, makePosition(320, 240, 11));
		addTimerCB(120, gotoNextDay, NULL);
	}

	gData.mStage = 1;
}

static void gotoUndeletedPosts(void* tCaller) {
	removeMugenAnimation(gData.mResultAnimationID);
	startDeletedPosts();
}

static void unloadHateBar() {
	removeMugenAnimation(gData.mBarBGID);
	removeMugenAnimation(gData.mBarID);
	removeMugenAnimation(gData.mBombID);
	gotoUndeletedPosts(NULL);
}

static void removeHateBarCB(void* tCaller) {
	if (getCurrentHate() == getMaximumHate()) {
		startEnding(gotoBadEnding2);
	}
	else {
		unloadHateBar();
	}
}

static void increaseHateBar(void* tCaller) {
	increaseHate(gData.mTweetAmount * 5);

	double factor = getCurrentHate() / (double)getMaximumHate();
	setMugenAnimationDrawScale(gData.mBarID, makePosition(factor, 1, 1));

	addTimerCB(60, removeHateBarCB, NULL);
}

static void showHateIncrease() {
	
	gData.mBarBGID = addMugenAnimation(getMugenAnimation(gData.mAnimations, 20), gData.mSprites, makePosition(62, 194, 10));
	setMugenAnimationColor(gData.mBarBGID, 1, 0, 0);

	gData.mBarID = addMugenAnimation(getMugenAnimation(gData.mAnimations, 20), gData.mSprites, makePosition(62, 194, 11));

	double factor = getCurrentHate() / (double)getMaximumHate();
	setMugenAnimationDrawScale(gData.mBarID, makePosition(factor, 1, 1));

	gData.mBombID = addMugenAnimation(getMugenAnimation(gData.mAnimations, 22), gData.mSprites, makePosition(62, 194, 12));

	addTimerCB(30, increaseHateBar, NULL);
}

static void missflaggedTweetInCenter(void* tCaller);
static void showNextMissflaggedTweet() {
	Tweet t = getMissflaggedTweet(gData.mCurrentTweet);
	loadTweet(t, missflaggedTweetInCenter);
}

static void missflaggedTweetMovedLeft(void* tCaller) {
	unloadCurrentTweet();
	gData.mCurrentTweet++;
	
	if (gData.mCurrentTweet >= gData.mTweetAmount) {
		clearMissflaggedTweets();
		showHateIncrease();
		return;
	}
	else {
		showNextMissflaggedTweet();
	}
}

static void moveMissflaggedTweetLeft(void* tCaller) {
	moveTweetLeft(missflaggedTweetMovedLeft);
}

static void missflaggedTweetInCenter(void* tCaller) {
	addTimerCB(120, moveMissflaggedTweetLeft, NULL);
}

static void moveMissflaggedAnimationFinished(void* tCaller) {
	showNextMissflaggedTweet();
}

static void moveMissflaggedPointerDown(void* tCaller) {
	tweenDouble(&getMugenAnimationPositionReference(gData.mResultAnimationID)->y, getMugenAnimationPositionReference(gData.mResultAnimationID)->y, getMugenAnimationPositionReference(gData.mResultAnimationID)->y + 150, linearTweeningFunction, 30, moveMissflaggedAnimationFinished, NULL);
}

static void startMissflaggedPosts() {
	gData.mTweetAmount = getMissflaggedTweetAmountAndLoadMissflaggedTweets();
	gData.mCurrentTweet = 0;

	if (gData.mTweetAmount) {
		gData.mResultAnimationID = addMugenAnimation(getMugenAnimation(gData.mAnimations, 40), gData.mSprites, makePosition(320, 240, 10));
		addTimerCB(180, moveMissflaggedPointerDown, NULL);
	}
	else {
		gData.mResultAnimationID = addMugenAnimation(getMugenAnimation(gData.mAnimations, 41), gData.mSprites, makePosition(320, 240, 11));
		addTimerCB(120, gotoUndeletedPosts, NULL);
	}

	gData.mStage = 0;
}

static void updateEvaluationScreen() {
	if (hasPressedAbortFlank()) {
		setNewScreen(&TitleScreen);
	}
}

Screen EvaluationScreen = {
	.mLoad = loadEvaluationScreen,
	.mUpdate = updateEvaluationScreen,
};