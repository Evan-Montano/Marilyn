#include "Chat.h"
#include "DataHelper.h"

Brain chattyKitty;
std::string userChat = "";

void initChatModule() {
	if (chattyKitty.initChat() == false) return;
	chattyKitty.beginChat();
}

bool Brain::initChat() {
	std::cout << "Loading Marilyn\'s Brain..." << std::endl;
	memoryWorker = std::fstream("../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
	neuronWorker = std::fstream("../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

	if (!memoryWorker) {
		std::cout << "Failed to access Marilyn\'s smooth brain." << std::endl;
		return false;
	}

	if (!neuronWorker) {
		std::cout << "Failed to access Marilyn\'s neuron index." << std::endl;
		return false;
	}

	loadBrain();

	return true;
}

void Brain::beginChat() {
	std::cout << "Thanks for talking to my cat Marilyn. Just know that she doesn't understand English that well :)"
		<< std::endl << "Enter 'exit' to leave." << std::endl;

	while (true) {
		std::cout << std::endl << ">>";
		std::getline(std::cin, userChat);
		if (userChat.size() == 0) continue;
		if (userChat == "exit")break;

		getMeow(userChat);
		std::cout << std::endl;
	}
}