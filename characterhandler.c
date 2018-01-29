#include "characterhandler.h"

#include <tari/mugendefreader.h>
#include <tari/memoryhandler.h>

typedef struct {
	char mName[200];
	char mAccountName[200];
} Character;

static struct {
	int mCharacterAmount;
	Character mCharacters[200];

} gData;

static void loadSingleCharacter(MugenDefScriptGroup* tGroup) {
	Character* character = &gData.mCharacters[gData.mCharacterAmount];

	char* text = getAllocatedMugenDefStringVariableAsGroup(tGroup, "name");
	strcpy(character->mName, text);
	freeMemory(text);

	text = getAllocatedMugenDefStringVariableAsGroup(tGroup, "accountname");
	strcpy(character->mAccountName, text);
	freeMemory(text);

	gData.mCharacterAmount++;
}

void loadCharacters()
{
	gData.mCharacterAmount = 0;

	MugenDefScript script = loadMugenDefScript("assets/characters/CHARACTERS.def");
	MugenDefScriptGroup* group = script.mFirstGroup;
	while (group != NULL) {
		loadSingleCharacter(group);
		group = group->mNext;
	}

	unloadMugenDefScript(script);
}

char * getCharacterName(int i)
{
	return gData.mCharacters[i].mName;
}

char * getCharacterAccountName(int i)
{
	return gData.mCharacters[i].mAccountName;
}
