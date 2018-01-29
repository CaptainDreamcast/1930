#include "gamescreen.h"

#include <tari/input.h>
#include <tari/mugenanimationhandler.h>
#include <tari/mugentexthandler.h>
#include <tari/animation.h>
#include <tari/tweening.h>

#include "characterhandler.h"
#include "tweethandler.h"
#include "evaluationscreen.h"
#include "titlescreen.h"

static struct {
	int mDay;
	int mCurrentHate;
	int mMaximumHate;
	int mMoney;

	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;

	MugenSpriteFile mCharacterSprites;
	MugenAnimations mCharacterAnimations;

	int mBarID;

	int mTweetBGID;
	int mNameID;
	int mAccountNameID;
	int mTextID;
	int mDateID;

	int mDeleteButtonID;
	int mKeepButtonID;

	int mAvatarID;

	int mDayID;
	int mTimeID;
	int mTweetsLeftID;

	int mCurrentTweet;
	int mTweetsLeft;
	Duration mMaxTime;
	Duration mTimeLeft;

	int mAreButtonsActive;
	int mHasSetCurrentTweet;
} gData;


static void loadCurrentTweet();

void loadSpritesAndAnimations() {
	gData.mSprites = loadMugenSpriteFileWithoutPalette("assets/gamescreen/GAME.sff");
	gData.mAnimations = loadMugenAnimationFile("assets/gamescreen/GAME.air");

	gData.mCharacterSprites = loadMugenSpriteFileWithoutPalette("assets/characters/CHARACTERS.sff");
	gData.mCharacterAnimations = loadMugenAnimationFile("assets/characters/CHARACTERS.air");
}

MugenSpriteFile * getGameSprites()
{
	return &gData.mSprites;
}

MugenAnimations * getGameAnimations()
{
	return &gData.mAnimations;
}

static void loadGameScreen() {
	instantiateActor(getMugenAnimationHandlerActorBlueprint());
	instantiateActor(MugenTextHandler);

	loadSpritesAndAnimations();

	gData.mDay++;

	addMugenAnimation(getMugenAnimation(&gData.mAnimations, 1), &gData.mSprites, makePosition(0, 0, 1));
	gData.mBarID = addMugenAnimation(getMugenAnimation(&gData.mAnimations, 20), &gData.mSprites, makePosition(62, 94, 10));
	addMugenAnimation(getMugenAnimation(&gData.mAnimations, 21), &gData.mSprites, makePosition(62, 94, 11));

	generateDayTweets();
	loadCurrentTweet();

	char text[200];
	sprintf(text, "Day %d", gData.mDay);
	gData.mDayID = addMugenText(text, makePosition(298, 47, 21), 1);
	setMugenTextColor(gData.mDayID, COLOR_BLACK);

	gData.mTweetsLeft = getTweetAmount();
	sprintf(text, "%d Posts left", gData.mTweetsLeft);
	gData.mTweetsLeftID = addMugenText(text, makePosition(225, 79, 21), 1);
	setMugenTextColor(gData.mTweetsLeftID, COLOR_BLACK);

	gData.mCurrentTweet = 0;

	gData.mMaxTime = 20*60;
	gData.mTimeLeft = gData.mMaxTime;
}

static void getDateTextFromTweet(char* tDst, int tHour, int tMinute, int tDay) {
	char timePart[100];

	if (tHour < 12) {
		sprintf(timePart, "%d:%d AM", tHour, tMinute);
	}
	else{
		if (tHour != 12) tHour -= 12;
		sprintf(timePart, "%d:%d PM", tHour, tMinute);
	}

	int day = 27 + tDay;
	if (day <= 31) {
		sprintf(tDst, "%s - %d Jan 2018", timePart, day);
	}
	else {
		sprintf(tDst, "%s - %d Feb 2018", timePart, day - 31 + 1);
	}
}

static void setButtonsActive(void* tCaller) {
	gData.mDeleteButtonID = addMugenAnimation(getMugenAnimation(&gData.mAnimations, 10), &gData.mSprites, makePosition(220, 360, 20));
	gData.mKeepButtonID = addMugenAnimation(getMugenAnimation(&gData.mAnimations, 11), &gData.mSprites, makePosition(420, 360, 20));

	gData.mAreButtonsActive = 1;
}



void loadTweet(Tweet tTweet, void(*tCB)(void*)) {
	double tweenOffset = 550;
	Duration tweenDuration = 30;
	TweeningFunction tweenFunction = linearTweeningFunction;

	gData.mTweetBGID = addMugenAnimation(getMugenAnimation(&gData.mAnimations, 30), &gData.mSprites, makePosition(320, 240, 20));
	tweenDouble(&getMugenAnimationPositionReference(gData.mTweetBGID)->x, getMugenAnimationPositionReference(gData.mTweetBGID)->x + tweenOffset, getMugenAnimationPositionReference(gData.mTweetBGID)->x, tweenFunction, tweenDuration, NULL, NULL);

	gData.mNameID = addMugenText(getCharacterName(tTweet.mAvatarAnimationNumber), makePosition(170, 170, 21), 2);
	setMugenTextColor(gData.mNameID, COLOR_BLACK);
	tweenDouble(&getMugenTextPositionReference(gData.mNameID)->x, getMugenTextPositionReference(gData.mNameID)->x + tweenOffset, getMugenTextPositionReference(gData.mNameID)->x, tweenFunction, tweenDuration, NULL, NULL);

	gData.mAccountNameID = addMugenText(getCharacterAccountName(tTweet.mAvatarAnimationNumber), makePosition(170, 182, 21), 3);
	setMugenTextColorRGB(gData.mAccountNameID, 0.4, 0.4, 0.4);
	tweenDouble(&getMugenTextPositionReference(gData.mAccountNameID)->x, getMugenTextPositionReference(gData.mAccountNameID)->x + tweenOffset, getMugenTextPositionReference(gData.mAccountNameID)->x, tweenFunction, tweenDuration, NULL, NULL);


	gData.mTextID = addMugenText(tTweet.mText, makePosition(130, 205, 21), 10);
	setMugenTextColor(gData.mTextID, COLOR_BLACK);
	setMugenTextTextBoxWidth(gData.mTextID, 400);
	tweenDouble(&getMugenTextPositionReference(gData.mTextID)->x, getMugenTextPositionReference(gData.mTextID)->x + tweenOffset, getMugenTextPositionReference(gData.mTextID)->x, tweenFunction, tweenDuration, NULL, NULL);

	char text[200];
	getDateTextFromTweet(text, tTweet.mHour, tTweet.mMinute, gData.mDay);
	gData.mDateID = addMugenText(text, makePosition(130, 270, 21), 3);
	setMugenTextColorRGB(gData.mDateID, 0.4, 0.4, 0.4);
	tweenDouble(&getMugenTextPositionReference(gData.mDateID)->x, getMugenTextPositionReference(gData.mDateID)->x + tweenOffset, getMugenTextPositionReference(gData.mDateID)->x, tweenFunction, tweenDuration, NULL, NULL);

	gData.mAvatarID = addMugenAnimation(getMugenAnimation(&gData.mCharacterAnimations, tTweet.mAvatarAnimationNumber), &gData.mCharacterSprites, makePosition(145, 166, 19));
	tweenDouble(&getMugenAnimationPositionReference(gData.mAvatarID)->x, getMugenAnimationPositionReference(gData.mAvatarID)->x + tweenOffset, getMugenAnimationPositionReference(gData.mAvatarID)->x, tweenFunction, tweenDuration, tCB, NULL);
	
	gData.mHasSetCurrentTweet = 0;
}

static void loadCurrentTweet() {
	Tweet tweet = getTweet(gData.mCurrentTweet);
	loadTweet(tweet, setButtonsActive);
	gData.mAreButtonsActive = 0;
}

void unloadCurrentTweet() {
	removeMugenAnimation(gData.mTweetBGID);
	removeMugenText(gData.mNameID);
	removeMugenText(gData.mAccountNameID);
	removeMugenText(gData.mTextID);
	removeMugenText(gData.mDateID);
	removeMugenAnimation(gData.mAvatarID);
}

static void moveTweetLeftFinishedCB(void* tCaller) {
	(void)tCaller;
	unloadCurrentTweet();
	gData.mCurrentTweet++;
	if (gData.mCurrentTweet >= getTweetAmount()) {
		setNewScreen(&EvaluationScreen);
		return;
	}

	loadCurrentTweet();
}

void moveTweetLeft(void(*tCB)(void*)) {
	double tweenOffset = -550;
	Duration tweenDuration = 30;
	TweeningFunction tweenFunction = linearTweeningFunction;

	tweenDouble(&getMugenAnimationPositionReference(gData.mTweetBGID)->x, getMugenAnimationPositionReference(gData.mTweetBGID)->x, getMugenAnimationPositionReference(gData.mTweetBGID)->x + tweenOffset, tweenFunction, tweenDuration, NULL, NULL);
	tweenDouble(&getMugenTextPositionReference(gData.mNameID)->x, getMugenTextPositionReference(gData.mNameID)->x, getMugenTextPositionReference(gData.mNameID)->x + tweenOffset, tweenFunction, tweenDuration, NULL, NULL);
	tweenDouble(&getMugenTextPositionReference(gData.mAccountNameID)->x, getMugenTextPositionReference(gData.mAccountNameID)->x, getMugenTextPositionReference(gData.mAccountNameID)->x + tweenOffset, tweenFunction, tweenDuration, NULL, NULL);
	tweenDouble(&getMugenTextPositionReference(gData.mTextID)->x, getMugenTextPositionReference(gData.mTextID)->x, getMugenTextPositionReference(gData.mTextID)->x + tweenOffset, tweenFunction, tweenDuration, NULL, NULL);
	tweenDouble(&getMugenTextPositionReference(gData.mDateID)->x, getMugenTextPositionReference(gData.mDateID)->x, getMugenTextPositionReference(gData.mDateID)->x + tweenOffset, tweenFunction, tweenDuration, NULL, NULL);
	tweenDouble(&getMugenAnimationPositionReference(gData.mAvatarID)->x, getMugenAnimationPositionReference(gData.mAvatarID)->x, getMugenAnimationPositionReference(gData.mAvatarID)->x + tweenOffset, tweenFunction, tweenDuration, tCB, NULL);

	gData.mHasSetCurrentTweet = 1;
}

static void setMissedTweetsKept() {

	int i = gData.mCurrentTweet;
	if (gData.mHasSetCurrentTweet) {
		i++;
	}

	for (i; i < getTweetAmount(); i++) {
		setTweetKept(i);	
	}
}

static void updateTime() {

	if (gData.mTimeLeft <= 0) {
		setMissedTweetsKept();
		setNewScreen(&EvaluationScreen);
		return;
	}
	gData.mTimeLeft--;

	double factor = gData.mTimeLeft / gData.mMaxTime;
	setMugenAnimationDrawScale(gData.mBarID, makePosition(factor, 1, 1));
}


static void decreaseTweetAmount() {
	char text[200];
	gData.mTweetsLeft = getTweetAmount() - gData.mCurrentTweet - 1;
	sprintf(text, "%d Posts left", gData.mTweetsLeft);
	changeMugenText(gData.mTweetsLeftID, text);
}

static void gotoNextTweet() {
	gData.mAreButtonsActive = 0;
	decreaseTweetAmount();
	moveTweetLeft(moveTweetLeftFinishedCB);
}

static void keepTweet() {
	setTweetKept(gData.mCurrentTweet);

	changeMugenAnimation(gData.mKeepButtonID, getMugenAnimation(&gData.mAnimations, 14));
	setMugenAnimationNoLoop(gData.mKeepButtonID);

	changeMugenAnimation(gData.mDeleteButtonID, getMugenAnimation(&gData.mAnimations, 12));
	setMugenAnimationNoLoop(gData.mDeleteButtonID);

	gotoNextTweet();
}

static void deleteTweet() {
	setTweetDeleted(gData.mCurrentTweet);

	changeMugenAnimation(gData.mKeepButtonID, getMugenAnimation(&gData.mAnimations, 13));
	setMugenAnimationNoLoop(gData.mKeepButtonID);

	changeMugenAnimation(gData.mDeleteButtonID, getMugenAnimation(&gData.mAnimations, 14));
	setMugenAnimationNoLoop(gData.mDeleteButtonID);

	gotoNextTweet();
}



static int hasPressedKept(int tFlank) {
	if (!tFlank) return 0;

	Position p = getShotPosition();
	GeoRectangle rect = makeGeoRectangle(420 - 30, 360, 61, 88);

	return checkPointInRectangle(rect, p);
}

static int hasPressedDeleted(int tFlank) {
	if (!tFlank) return 0;

	Position p = getShotPosition();
	GeoRectangle rect = makeGeoRectangle(220 - 46, 360, 93, 88);

	return checkPointInRectangle(rect, p);
}

static void updateButtonInput() {
	if (!gData.mAreButtonsActive) return;

	int flank = hasShotGunFlank();

	if (hasPressedBFlank() || hasPressedKept(flank)) {
		keepTweet();
	}
	else if (hasPressedAFlank() || hasPressedDeleted(flank)) {
		deleteTweet();
	}

}

static void updateGameScreen() {
	updateTime();
	updateButtonInput();

	if (hasPressedAbortFlank()) {
		setNewScreen(&TitleScreen);
	}
}

Screen GameScreen = {
	.mLoad = loadGameScreen,
	.mUpdate = updateGameScreen,
};

int getGameDay()
{
	return gData.mDay;
}

void resetGameState() {
	gData.mDay = 0;
	gData.mCurrentHate = 0;
	gData.mMaximumHate = 100;
	gData.mMoney = 10000000;
	resetTweethandler();
}

int getCurrentHate() {
	return gData.mCurrentHate;
}

int getMaximumHate() {
	return gData.mMaximumHate;
}

void increaseHate(int tDelta)
{
	gData.mCurrentHate = min(gData.mMaximumHate, gData.mCurrentHate + tDelta);
}

int getCurrentMoney()
{
	return gData.mMoney;
}

void decreaseMoney(int tDelta)
{
	gData.mMoney = max(0, gData.mMoney - tDelta);
}
