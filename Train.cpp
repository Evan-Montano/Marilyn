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

	std::cout << std::endl << "Beginning file processing. This *WILL take a long time.." << std::endl;
	brain.processAttachedFile();
}

void Brain::processAttachedFile() {
	const uint64_t totalBytes = static_cast<uint64_t>(getFileSize(inFile));
	typedef std::istreambuf_iterator<char> buf_iter;
	std::deque<char> window = {};
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
			// Set the characters which is the child of the current parent Key
			parentKeyHash[KEY_SIZE] = window[n];

			// Add or modify the child record in the brain
			if (std::memcmp(brainMap[parentKeyHash].key.data(), EMPTY_KEY.data(), KEY_SIZE) == 0 
					|| brainMap[parentKeyHash].frequency == 0) {
				// This is a new node
				brainMap[parentKeyHash].key = generate10ByteKey();
				brainMap[parentKeyHash].frequency = 1;
			}
			else {
				if (brainMap[parentKeyHash].frequency < 255) {
					brainMap[parentKeyHash].frequency++;
				}
			}

			// Ensure the neuronMap includes this char in the parentKey's vector
			std::array<char, KEY_SIZE> parentKey_only{};
			std::memcpy(parentKey_only.data(), parentKeyHash.data(), KEY_SIZE);
			std::vector<char>& vec = neuronMap[parentKey_only];
			if (std::find(vec.begin(), vec.end(), window[n]) == vec.end()) {
				vec.push_back(window[n]);
			}


			// save the current key into the parentKey for the next child
			std::memcpy(parentKeyHash.data(), brainMap[parentKeyHash].key.data(), KEY_SIZE);
		}

		std::cout << "\r"
			<< (double(inFile.tellg()) / double(totalBytes)) * 100.0 << "%";
	}

	std::cout << std::endl << "Saving to disk.." << std::endl;
	saveDataToDisk();

	std::cout << std::endl << "Completed." << std::endl;
}
