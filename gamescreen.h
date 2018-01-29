#pragma once

#include <tari/wrapper.h>
#include <tari/mugenanimationhandler.h>

#include "tweethandler.h"

extern Screen GameScreen;

void loadSpritesAndAnimations();
MugenSpriteFile* getGameSprites();
MugenAnimations* getGameAnimations();


int getGameDay();
void loadTweet(Tweet tTweet, void(*tCB)(void*));
void unloadCurrentTweet();
void moveTweetLeft(void(*tCB)(void*));

void resetGameState();
int getCurrentHate();
int getMaximumHate();
void increaseHate(int tDelta);

int getCurrentMoney();
void decreaseMoney(int tDelta);