#include "Marilyn.h"
#include "DataHelper.h"
#include "Train.h"
#include "Chat.h"

int main()
{
	int mode = CHAT;

	mode == TRAIN ? initTrainModule() : initChatModule();

	return 0;
}

