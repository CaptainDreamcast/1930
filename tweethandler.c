#include "tweethandler.h"

#include <tari/math.h>
#include <tari/mugendefreader.h>
#include <tari/memoryhandler.h>

#include "gamescreen.h"

#define TWEET_AMOUNT 500

static struct {
	int mDayTweetAmount;
	Tweet mDayTweets[20];

	int mArchivedTweetAmount;
	Tweet mTweetArchive[TWEET_AMOUNT];

	int mMissflaggedTweetAmount;
	Tweet mMissflaggedTweets[100];

	int mKeptTweetAmount;
	Tweet mKeptTweets[100];

	int mActiveDeletedTweetAmount;
	Tweet mActiveDeletedTweets[100];


}	gData;

static void generateTweet(int i) {
	int j;
	for (j = 0; j < 100; j++) {
		int index = randfromInteger(0, gData.mArchivedTweetAmount - 1);

		gData.mDayTweets[i] = gData.mTweetArchive[index];
		gData.mDayTweets[i].mDay = getGameDay();
		gData.mDayTweets[i].mHour = randfromInteger(0, 23);
		gData.mDayTweets[i].mMinute = randfromInteger(0, 59);
		break;
	}

}

void generateDayTweets()
{
	gData.mDayTweetAmount = randfromInteger(9, 11);
	int i;
	for (i = 0; i < gData.mDayTweetAmount; i++) {
		generateTweet(i);
	}
}

int getTweetAmount()
{
	return gData.mDayTweetAmount;
}

Tweet getTweet(int i)
{
	return gData.mDayTweets[i];
}

void setTweetKept(int i)
{
	Tweet t = gData.mDayTweets[i];
	if (!t.mIsIllegal) return;

	gData.mKeptTweets[gData.mKeptTweetAmount] = t;
	gData.mKeptTweetAmount++;
}

void setTweetDeleted(int i)
{
	Tweet t = gData.mDayTweets[i];
	if (!t.mIsIllegal) {
		gData.mMissflaggedTweets[gData.mMissflaggedTweetAmount] = t;
		gData.mMissflaggedTweetAmount++;
	}
}

static void loadSingleTweet(MugenDefScriptGroup* tGroup) {
	Tweet* e = &gData.mTweetArchive[gData.mArchivedTweetAmount];
	e->mAvatarAnimationNumber = getMugenDefIntegerOrDefaultAsGroup(tGroup, "avatar", 1);
	e->mHour = randfromInteger(0, 23);
	e->mMinute = randfromInteger(0, 59);
	
	char* text = getAllocatedMugenDefStringVariableAsGroup(tGroup, "illegal");
	if (!strcmp("now", text)) {
		e->mIsIllegal = 1;
	}
	else if (!strcmp("then", text)) {
		e->mIsIllegal = 2;
	}
	else {
		e->mIsIllegal = 0;
	}

	freeMemory(text);

	text = getAllocatedMugenDefStringVariableAsGroup(tGroup, "text");
	strcpy(e->mText, text);
	freeMemory(text);

	e->mWasUsed = 0;

	gData.mArchivedTweetAmount++;
}

void loadTweetArchive()
{
	MugenDefScript script = loadMugenDefScript("assets/tweets/TWEETS.def");

	gData.mArchivedTweetAmount = 0;
	MugenDefScriptGroup* group = script.mFirstGroup;
	while (group != NULL) {
		loadSingleTweet(group);
		group = group->mNext;
	}

	unloadMugenDefScript(script);
}

void resetTweethandler()
{
	gData.mMissflaggedTweetAmount = 0;
	gData.mKeptTweetAmount = 0;
	gData.mActiveDeletedTweetAmount = 0;
}

int getMissflaggedTweetAmountAndLoadMissflaggedTweets()
{
	return gData.mMissflaggedTweetAmount; 
}

Tweet getMissflaggedTweet(int i)
{
	return gData.mMissflaggedTweets[i];
}

void clearMissflaggedTweets()
{
	gData.mMissflaggedTweetAmount = 0;
}

int getDeletedTweetAmountAndLoadDeletedTweets(int tDay)
{
	int deletedTweetAmountBuffer;
	Tweet deletedTweetBuffer[100];

	deletedTweetAmountBuffer = 0;
	gData.mActiveDeletedTweetAmount = 0;
	int i;
	for (i = 0; i < gData.mKeptTweetAmount; i++) {
		Tweet t = gData.mKeptTweets[i];
		if (t.mIsIllegal == 1 || (t.mIsIllegal == 2 && abs(tDay - t.mDay) >= 7)) {
			gData.mActiveDeletedTweets[gData.mActiveDeletedTweetAmount] = t;
			gData.mActiveDeletedTweetAmount++;
		}
		else {
			deletedTweetBuffer[deletedTweetAmountBuffer] = t;
			deletedTweetAmountBuffer++;
		}
	}


	for (i = 0; i < deletedTweetAmountBuffer; i++) {
		gData.mKeptTweets[i] = deletedTweetBuffer[i];
	}
	gData.mKeptTweetAmount = deletedTweetAmountBuffer;

	return gData.mActiveDeletedTweetAmount;
}

Tweet getDeletedTweet(int i)
{
	return gData.mActiveDeletedTweets[i];
}
