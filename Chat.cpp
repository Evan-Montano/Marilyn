#include "Chat.h"
#include "DataHelper.h"

Brain chattyKitty;
std::string userChat = "";

void initChatModule() {
	chattyKitty.initChat();
	chattyKitty.beginChat();
}

bool Brain::initChat() {
	memoryWorker = std::fstream("../../../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
	neuronWorker = std::fstream("../../../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

	if (!memoryWorker) {
		std::cout << "Failed to access Marilyn\'s smooth brain." << std::endl;
		return false;
	}

	if (!neuronWorker) {
		std::cout << "Failed to access Marilyn\'s neuron index." << std::endl;
		return false;
	}

	return true;
}

void Brain::beginChat() {
	std::cout << "Thanks for talking to my cat Marilyn. Just know that she doesn't understand English that well :)"
		<< std::endl << "Enter '0' to exit." << std::endl;

	while (userChat != "0") {
		std::cout << std::endl << ">>";
		std::getline(std::cin, userChat);
		if (userChat.size() == 0) continue;

		getMeow(userChat);
	}
}