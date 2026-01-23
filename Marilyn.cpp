#include "Marilyn.h"
#include "DataHelper.h"
#include "Train.h"
#include "Chat.h"

int main()
{
	int mode = TRAIN;

	mode == TRAIN ? initTrainModule() : initChatModule();

	/*Brain br;
	br.initChat();
	std::string n = br.getHighestNeruon();
	std::cout << n;*/
	return 0;
}

