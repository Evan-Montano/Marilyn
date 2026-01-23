#include "Marilyn.h"
#include "Train.h"
#include "DataHelper.h"

Brain noLumpsOrBumps;
void initTrainModule() {
	if (noLumpsOrBumps.initTraining() == false) return;

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
	memoryWorker = std::fstream("../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
	neuronWorker = std::fstream("../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

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
	std::array<char, KEY_SIZE> parentKey{};

	std::cout << "Loading brain for processing..";
	noLumpsOrBumps.loadBrain();

	// loop to parse training file
	for (buf_iter i(eyes), e; i != e; ++i) {
		char c = *i;
		++bytesProcessed;

		if (c == '\n') continue;

		if (window.size() >= NEURON_DEPTH) {
			window.erase(window.begin());
		}
		window.push_back(EyeCandy{ c, false });

		// outer loop for the root node
		for (size_t n = 0; n < window.size(); n++) {
			//std::memcpy(parentKey, EMPTY_KEY, KEY_SIZE);
			parentKey = EMPTY_KEY;
			uint64_t neuronInx = 0;

			// inner loop for the root & subsequent chars
			for (size_t j = n; j < window.size(); j++) {
				// check for a child node where the 
				// character is window.at(j).ch AND 
				// the parent key matches. Create one if it isn't found
				bool found = false;
				for (neuronInx; neuronInx < neuronVec.size(); neuronInx++) {
					// position / memory_size = index in the memory vector
					uint64_t inx = (neuronVec.at(neuronInx).position) / MEMORY_SIZE;

					MemoryNode memNode = memoryVec.at(inx);
					NeuronNode neurNode = neuronVec.at(inx);
					if (keyCompare(parentKey, neurNode.parentKey) && memNode.ch == window.at(j).ch) {
						// match found
						// increment child nodes
						// set the parent key as the curr node key
						found = true;
						if (keyCompare(parentKey, EMPTY_KEY) == false) {
							++memNode.frequency;
						}
						parentKey = memNode.key;
						break;
					}
				}
				if (found == false) {
					// create a new node with the parent key and char
					std::array<char, KEY_SIZE> newKey = generate10ByteKey();
					
					NeuronNode newN {newKey, (neuronInx*MEMORY_SIZE), parentKey};
					MemoryNode newM {newKey, window.at(j).ch, 1};
 
					neuronVec.push_back(newN);
					memoryVec.push_back(newM);
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
	
	// Now we need to write the training data back to disk
	std::cout << std::endl << "Saving to disk.." << std::endl;
	saveTrainingDataToDisk();

	std::cout << "Completed.";
}
