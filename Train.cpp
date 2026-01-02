#include "Marilyn.h"
#include "Train.h"
#include "DataHelper.h"

Brain sig;

void initTrainModule() {
	sig.initTraining();
	sig.processAttachedFile();
}

bool Brain::initTraining() {
	std::cout << std::endl << "Paste the file path: ";
	std::string path;

	if (std::cin.peek() == '\n') {
		std::cin.ignore();
	}
	std::getline(std::cin, path);

	eyes = std::ifstream(path, std::ios::in);
	memoryWorker = std::fstream("../../../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
	neuronWorker = std::fstream("../../../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

	if (!eyes) {
		std::cout << "Failed to open file from path." << std::endl;
		return false;
	}

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

void Brain::processAttachedFile() {
	typedef std::istreambuf_iterator<char> buf_iter;
	std::vector<EyeCandy> window;

	for (buf_iter i(eyes), e; i != e; ++i) {
		char c = *i;
		if (c == '\n') continue;
		if (window.size() >= NEURON_DEPTH) {
			window.erase(window.begin());
		}
		window.push_back(EyeCandy{ c, false });

		for (size_t n = 0; n < window.size(); ++n) {
			// if this is false, then we are only appending new nodes
			// and setting their frequency to 1
			bool processExisting = !window.at(n).proc;
			// if true, then we can append new nodes, and increment the
			// frequency of existing nodes, if found.

			std::string parentKey = EMPTY_KEY;
			for (size_t j = n; j < window.size(); ++j) {
				// if the parent key is empty, we should first search
				// for a root node with the same char, and assign the parentKey
				// for subsequent searches.
				// Create new if doesn't exist.
				if (parentKey == EMPTY_KEY) {
					Neuron result = getNeuronByTargetAndParentKey(EMPTY_KEY, c, false, true);
					parentKey = result.key;
				}
				// ensure the current parentKey is valid 8-bytes
				else if (parentKey.size() == 8) {
					// we are searching for a child node
					Neuron result = getNeuronByTargetAndParentKey(parentKey, c, processExisting, true);
					parentKey = result.key;
				}
				else {
					// Clear the window and try again, something went wrong... :(
					window.clear();
				}
			}

			window.at(n).proc = true;
		}
	}
}