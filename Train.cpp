#include "Marilyn.h"
#include "Train.h"
#include "DataHelper.h"

Brain brain;
void initTrainModule() {
	std::cout << "Text (.txt) files only, please. ";
	std::cout << std::endl << "Paste the file path: ";
	std::string path;

	if (std::cin.peek() == '\n') {
		std::cin.ignore();
	}
	std::getline(std::cin, path);
	brain.inFile = std::fstream(path, std::ios::in);
		if (!brain.inFile) {
		std::cout << "Failed to open file from path." << std::endl;
		return;
	}

	if (!brain.loadBrain() || !brain.loadNeurons()) return;

	std::cout << "Beginning file processing. This *WILL take a long time.." << std::endl;
	brain.processAttachedFile();
}

void Brain::processAttachedFile() {
	const uint64_t totalBytes = static_cast<uint64_t>(getFileSize(inFile));
	typedef std::istreambuf_iterator<char> buf_iter;
	std::deque<char> window;
	const size_t preloadSize = NEURON_DEPTH;
	std::array<char, KEY_SIZE+1> parentKeyHash{};

	buf_iter i(inFile), e;
	size_t count = 0;

	// Load the first NEURON_SIZE characters
	for (; i != e && window.size() < preloadSize; i++) {
		char c = *i;
		if (c == 'n') continue;
		window.push_back(c);
	}

	// loop to parse rest of training file
	for (; i != e; i++) {
		char c = *i;
		if (c == '\n') continue;
		if (window.size() > NEURON_DEPTH) window.pop_front();
		window.push_back(c);

		// Parse the window fully once, and build/increment the chain
		// with the root of window[0]
		std::memcpy(parentKeyHash.data(), EMPTY_KEY.data(), KEY_SIZE);
		for (size_t n = 0; n < window.size(); n++) {
			parentKeyHash[KEY_SIZE] = c;
			// brainMap[parentKeyHash];
			if (std::memcmp(brainMap[parentKeyHash].key.data(), EMPTY_KEY.data(), KEY_SIZE) == 0 
					|| brainMap[parentKeyHash].frequency == 0) {
				// This is a new node
				brainMap[parentKeyHash].key = generate10ByteKey();
				brainMap[parentKeyHash].frequency = 1;
			}
			else {
				brainMap[parentKeyHash].frequency++;
			}




		}
	}
}

// 	// loop to parse rest training file

// 		uint64_t neuronInx = 0;
// 		parentKey = EMPTY_KEY;
// 		// Parse the window fully once, and build/increment the chain
// 		
// 		for (size_t n = 0; n < window.size(); n++) {
// 			bool found = false;

// 			// check for a child node where the 
// 			// character is window.at(j).ch AND 
// 			// the parent key matches. Create one if it isn't found
// 			for (; neuronInx < neuronVec.size(); neuronInx++) {
// 				MemoryNode& memNode = memoryVec[neuronInx];
// 				NeuronNode& neurNode = neuronVec[neuronInx];
// 				if (keyCompare(parentKey, neurNode.parentKey) && memNode.ch == window[n].ch) {
// 					// match found
// 					// increment child nodes
// 					// set the parent key as the curr node key
// 					found = true;
// 					++memNode.frequency;
// 					parentKey = memNode.key;
// 					break;
// 				}
// 			}
// 			if (found == false) {
// 				// create a new node with the parent key and char
// 				std::array<char, KEY_SIZE> newKey = generate10ByteKey();
				
// 				NeuronNode newN {newKey, parentKey};
// 				MemoryNode newM {newKey, window.at(n).ch, 1};

// 				neuronVec.push_back(newN);
// 				memoryVec.push_back(newM);

// 				keyCpy(parentKey, newKey);
// 			}
// 		}

// 		// Update the progress message.
// 		double percent =
// 			(double(bytesProcessed) / double(totalBytes)) * 100.0;
// 		std::cout << "\rProcessing: "
// 			<< bytesProcessed << " / "
// 			<< totalBytes << " bytes ("
// 			<< std::fixed << std::setprecision(1)
// 			<< percent << "%)   "
// 			<< std::flush;

// 	}
	
// 	// Now we need to write the training data back to disk
// 	std::cout << std::endl << "Saving to disk.." << std::endl;
// 	saveTrainingDataToDisk();

// 	std::cout << "Completed." << std::endl;
// }
