#include "Marilyn.h"
#include "DataHelper.h"
#include "Train.h"
#include "Chat.h"

int main()
{
	int mode = TRAIN;

	mode == TRAIN ? initTrainModule() : initChatModule();

	return 0;
}

