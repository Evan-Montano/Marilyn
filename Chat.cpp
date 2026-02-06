#include "Chat.h"
#include "DataHelper.h"

Brain brain1;
std::string userChat = "";

void initChatModule() {
	std::cout << "Initializing. This will take 30-60s..." << std::endl;

	{
		std::lock_guard<std::mutex> lock(brain1.coutMutex);
		std::cout << "\n\n";
		std::cout.flush();
	}

	auto brainFuture = std::async(std::launch::async, [&] {
		return brain1.loadBrain();
	});
	auto neuronFuture = std::async(std::launch::async, [&] {
		return brain1.loadNeurons();
	});

	if (!brainFuture.get() || !neuronFuture.get()) return;

	brain1.beginChat();
}

void Brain::beginChat() {
	std::cout << std::endl << "Thanks for talking to my cat Marilyn. Just know that she doesn't understand English that well :)"
		<< std::endl << "Tip: Punctuation helps with responses."
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