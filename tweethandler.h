#pragma once

typedef struct {
	int mAvatarAnimationNumber;
	char mText[200];

	int mDay;
	int mHour;
	int mMinute;

	int mIsIllegal;

	int mWasUsed;
} Tweet;

void generateDayTweets();
int getTweetAmount();
Tweet getTweet(int i);
void setTweetKept(int i);
void setTweetDeleted(int i);

void loadTweetArchive();
void resetTweethandler();

int getMissflaggedTweetAmountAndLoadMissflaggedTweets();
Tweet getMissflaggedTweet(int i);
void clearMissflaggedTweets();

int getDeletedTweetAmountAndLoadDeletedTweets(int tDay);
Tweet getDeletedTweet(int i);