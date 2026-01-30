#include "Chat.h"
#include "DataHelper.h"

Brain brain1;
std::string userChat = "";

void initChatModule() {
	std::cout << "Initializing..." << std::endl;
	if (!brain1.loadBrain() || !brain1.loadNeurons()) return;

	brain1.beginChat();
}

void Brain::beginChat() {
	std::cout << std::endl << "Thanks for talking to my cat Marilyn. Just know that she doesn't understand English that well :)"
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