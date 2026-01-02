#include "Marilyn.h"
#include "Train.h"
#include "DataHelper.h"

BrainSignal sig;

void initTrainModule() {
	sig.initTraining();
	sig.processAttachedFile();
}

bool BrainSignal::initTraining() {
	std::cout << std::endl << "Paste the file path: ";
	std::string path;

	if (std::cin.peek() == '\n') {
		std::cin.ignore();
	}
	std::getline(std::cin, path);

	eyes = std::ifstream(path, std::ios::in);
	memoryWriter = std::ofstream("../../../SmoothBrain/Marilyn.brain", std::ios::binary);
	neuronWriter = std::ofstream("../../../SmoothBrain/Marilyn.neurons", std::ios::binary);

	if (!eyes) {
		std::cout << "Failed to open file from path." << std::endl;
		return false;
	}

	if (!memoryWriter) {
		std::cout << "Failed to access Marilyn\'s smooth brain." << std::endl;
		return false;
	}

	if (!neuronWriter) {
		std::cout << "Failed to access Marilyn\'s neuron index." << std::endl;
		return false;
	}

	return true;
}

void BrainSignal::processAttachedFile() {
	typedef std::istreambuf_iterator<char> buf_iter;
	std::vector<eyeCandy> window;

	for (buf_iter i(eyes), e; i != e; ++i) {
		char c = *i;
		if (c == '\n') continue;
		if (window.size() >= NEURON_DEPTH) {
			window.erase(window.begin());
		}
		window.push_back(eyeCandy{ c, false });

		for (size_t n = 0; n < window.size(); ++n) {
			// if this is false, then we are only appending new nodes
			// and setting their frequency to 1
			bool processExisting = !window.at(n).proc;
			// if true, then we can append new nodes, and increment the
			// frequency of existing nodes, if found.

			std::string parentKey = EMPTY_KEY;
			for (size_t j = n; j < window.size(); ++j) {

			}
		}
	}
}