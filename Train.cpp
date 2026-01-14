#include "Marilyn.h"
#include "Train.h"
#include "DataHelper.h"

Brain noLumpsOrBumps;
void initTrainModule() {
	noLumpsOrBumps.initTraining();

	std::cout << "Beginning file processing. This *WILL take a long time.." << std::endl;
	noLumpsOrBumps.processAttachedFile();
}

bool Brain::initTraining() {
	std::cout << "Text (.txt) files only, please. ";
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

std::streampos getFileSize(std::ifstream& file) {
	auto current = file.tellg();
	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	file.seekg(current, std::ios::beg);
	return size;
}

void Brain::processAttachedFile() {
	const std::uint64_t totalBytes = static_cast<std::uint64_t>(getFileSize(eyes));
	std::uint64_t bytesProcessed = 0;
	typedef std::istreambuf_iterator<char> buf_iter;
	std::vector<EyeCandy> window;
	char parentKey[KEY_SIZE]{};

	std::cout << "Loading brain for processing..";
	noLumpsOrBumps.loadBrain();

	for (buf_iter i(eyes), e; i != e; ++i) {
		char c = *i;
		++bytesProcessed;

		if (c == '\n') continue;

		if (window.size() >= NEURON_DEPTH) {
			window.erase(window.begin());
		}
		window.push_back(EyeCandy{ c, false });

		for (size_t n = 0; n < window.size(); n++) {
			std::memcpy(parentKey, EMPTY_KEY, KEY_SIZE);
			uint64_t neuronInx = 0;

			for (size_t j = n; j < window.size(); j++) {
				if (keyCompare(parentKey, EMPTY_KEY)) {
					// check for a child node where the 
					// character is window.at(j).ch AND 
					// the parent key matches. Create one if it isn't found
					bool found = false;
					for (neuronInx; neuronInx < neuronVec.size(); ++neuronInx) {
						// position / memory_size = index in the memory vector
						uint64_t inx = neuronVec.at(neuronInx).position / MEMORY_SIZE;

						MemoryNode memNode = memoryVec.at(inx);
						if (keyCompare(parentKey, memNode.key) && memNode.ch == window.at(j).ch) {
							// match found
							found = true;
						}
					}
					if (found == false) {
						// create a new node with the parent key and char
						//char newKey[KEY_SIZE] = generate10ByteKey();
						//neuronVec.push_back(NeuronNode{})
					}
				}
				else {

				}
			}
		}

		// Update the progress message.
		double percent =
			(double(bytesProcessed) / double(totalBytes)) * 100.0;
		std::cout << "\rProcessing: "
			<< bytesProcessed << " / "
			<< totalBytes << " bytes ("
			<< std::fixed << std::setprecision(1)
			<< percent << "%)   "
			<< std::flush;

	}

	//for (buf_iter i(eyes), e; i != e; ++i) {
	// 
	//	for (size_t n = 0; n < window.size(); n++) {
	//		std::string parentKey = EMPTY_KEY;
	//		for (size_t j = n; j < window.size(); j++) {
	//			// if the parent key is empty, we should first search
	//			// for a root node with the same char, and assign the parentKey
	//			// for subsequent searches.
	//			// Create new if doesn't exist.
	//			if (parentKey == EMPTY_KEY) {
	//				Neuron result = getNeuronByTargetAndParentKey(EMPTY_KEY, window.at(j).ch, false, true);
	//				parentKey = result.key;
	//			}
	//			// ensure the current parentKey is valid 8-bytes
	//			else if (parentKey.size() == 8) {
	//				// we are searching for a child node
	//				// if a node hasn't been processed as a child, incrememnt it if found
	//				Neuron result = getNeuronByTargetAndParentKey(parentKey, window.at(j).ch, !window.at(j).proc, true);
	//				window.at(j).proc = true;
	//				parentKey = result.key;
	//			}
	//			else {
	//				window.clear();
	//			}
	//		}
	//	}
	//}
}
