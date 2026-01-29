#include "DataHelper.h"
#include <cstddef>

const char availableChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#@$.()*&^%-_=+";
std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::size_t> dist(0, strlen(availableChars)-1);

/// <summary>
/// Generates a seriable key to be used as an identifier.
/// </summary>
/// <returns>8-Byte std::string</returns>
std::array<char, KEY_SIZE> generate10ByteKey() {
	std::array<char, KEY_SIZE> res{};

	for (size_t i = 0; i < KEY_SIZE; ++i) {
		res[i] = availableChars[dist(rng)];
	}

	return res;
}

std::streampos Brain::getFileSize(std::fstream& file) {
	auto current = file.tellg();
	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	file.seekg(current, std::ios::beg);
	return size;
}


bool Brain::loadBrain() {
	brainWorker = std::fstream("../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
	if (!brainWorker) {
		std::cout << "Failed to access Marilyn\'s smooth brain." << std::endl;
		return false;
	}

	size_t fileSize = getFileSize(brainWorker);
	if (fileSize > 0) {
		brainMap.reserve(fileSize / 25);
	}

	brainWorker.clear();
	brainWorker.seekg(0, std::ios_base::beg);

	// === brain === //
	// std::unordered_map<(parentKey+char), Node>
	// 25 Byte Blocks:
	// [[ParentKey(10)][char(1)][Key(10)][Frequency(4)]]
	BrainCell cell{};
	Node node{};
	std::array<char, KEY_SIZE + 1> compositeKey{};
	while (cell = readBrainCell(), !brainWorker.eof()) {
		std::memcpy(compositeKey.data(), cell.idHash.data(), KEY_SIZE);
		node.key = cell.key;
		node.frequency = cell.freq;
		brainMap[compositeKey] = node;

		// Print progress update
		std::cout << "\r"
			<< (double(brainWorker.tellg()) / double(fileSize)) * 100.0 << "%";
	}

	return true;
}

BrainCell Brain::readBrainCell() {
	BrainCell res{};
	brainWorker.clear();
	char idHashBuffer[KEY_SIZE + 1] = {};
	char keyBuffer[KEY_SIZE] = {};
	uint8_t freqBuffer = 0;

	// brain reading [parentKey][char][key][frequency]
	if (!brainWorker.read(idHashBuffer, KEY_SIZE + 1)) {
		return res;
	}

	if (!brainWorker.read(keyBuffer, KEY_SIZE)) {
		return res;
	}

	if (!brainWorker.read(reinterpret_cast<char*>(&freqBuffer), sizeof(freqBuffer))) {
		return res;
	}

	std::memcpy(res.idHash.data(), idHashBuffer, KEY_SIZE + 1);
	std::memcpy(res.key.data(), keyBuffer, KEY_SIZE);
	res.freq = freqBuffer;
	return res;
}

bool Brain::loadNeurons() {
	neuronWorker = std::fstream("../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);
	if (!neuronWorker) {
		std::cout << "Failed to access Marilyn\'s neuron network." << std::endl;
		return false;
	}

	size_t fileSize = getFileSize(neuronWorker);
	if (fileSize > 0) {
		neuronMap.reserve(fileSize / 15);
	}

	neuronWorker.clear();
	neuronWorker.seekg(0, std::ios_base::beg);

	// === neurons === //
	// std::unordered_map<parentKey, vector<char>>
	// 10 + (N*1 + 1) Byte Blocks:
	// [ParentKey(10)]['a', 'b', '\0']
	Neuron neuron{};
	std::array<char, KEY_SIZE> parentKey{};
	while (neuron = readNeuron(), !neuronWorker.eof()) {
		parentKey = neuron.idHash;
		neuronMap[parentKey] = neuron.children;

		// Print progress update
		std::cout << "\r"
			<< (double(neuronWorker.tellg()) / double(fileSize)) * 100.0 << "%";
	}

	return true;
}

Neuron Brain::readNeuron() {
	Neuron res{};
	neuronWorker.clear();
	char idHashBuffer[KEY_SIZE] = {};
	
	// neuron reading [parentKey][char...'\0']
	if (!neuronWorker.read(idHashBuffer, KEY_SIZE)) {
		return res;
	}

	std::memcpy(res.idHash.data(), idHashBuffer, KEY_SIZE);

	// read children until null terminator
	char childChar = '\0';
	while (neuronWorker.read(&childChar, CHAR_SIZE)) {
		if (childChar == '\0') break;
		res.children.push_back(childChar);
	}

	return res;
}

void Brain::saveDataToDisk() {
	// Delete old files and write fresh ones.
	brainWorker.close();
	neuronWorker.close();
	std::filesystem::remove("../SmoothBrain/Marilyn.brain");
	std::filesystem::remove("../SmoothBrain/Marilyn.neurons");
	{
		std::ofstream("../SmoothBrain/Marilyn.brain", std::ios::binary | std::ios::app);
		std::ofstream("../SmoothBrain/Marilyn.neurons", std::ios::binary | std::ios::app);
	}
	brainWorker.open("../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
 	neuronWorker.open("../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

	uint64_t inx = 0;
	uint64_t totalSize = neuronMap.size();
	for (std::pair<const std::array<char, 11>, Node> & rec : brainMap) {
		if (writeToBrain(rec) == false) {
			std::cout << "An error occured while writing to brain.." << std::endl;
			return;
		}
		std::cout << "\r"
			<< "Writing brain: "
			<< (double(++inx) / double(totalSize)) * 100 << "%";
	}
	brainWorker.flush();

	inx = 0;
	totalSize = neuronMap.size();
	for (std::pair<const std::array<char, 10>, std::vector<char>> & rec : neuronMap) {
		if (writeToNetwork(rec) == false) {
			std::cout << "An error occured while writing to network.." << std::endl;
			return;
		}
		std::cout << "\r"
			<< "Writing neurons: "
			<< (double(++inx) / double(totalSize)) * 100 << "%";
	}
	neuronWorker.flush();
}

bool Brain::writeToNetwork(std::pair<const std::array<char, 10>, std::vector<char>> &rec) {
	neuronWorker.clear();
	neuronWorker.seekp(0, std::ios_base::end);

	// writing [parentKey][char...'\0']
	char parentKeyBuffer[KEY_SIZE] = {};
	std::memcpy(parentKeyBuffer, rec.first.data(), KEY_SIZE);
	neuronWorker.write(parentKeyBuffer, KEY_SIZE);

	std::vector<char> &vec = rec.second;
	vec.push_back('\0');
	neuronWorker.write(vec.data(), vec.size());

	return true;
}

bool Brain::writeToBrain(std::pair<const std::array<char, 11>, Node> &rec) {
	brainWorker.clear();
	brainWorker.seekp(0, std::ios_base::end);
	
	// writing [parentKey][char][key][frequency]
	char idHashBuffer[KEY_SIZE + 1] = {};
	std::memcpy(idHashBuffer, rec.first.data(), KEY_SIZE+1);
	brainWorker.write(idHashBuffer, KEY_SIZE+1);

	char keyBuffer[KEY_SIZE] = {};
	std::memcpy(keyBuffer, rec.second.key.data(), KEY_SIZE);
	brainWorker.write(keyBuffer, KEY_SIZE);

	brainWorker.write(reinterpret_cast<const char *>(&rec.second.frequency), sizeof(rec.second.frequency));

	return true;
}

// 		double percent =
// 			(double(inx+1) / double(totalSize)) * 100.0;
// 		std::cout << "\r"
// 			<< inx << " / "
// 			<< totalSize << " Meows saved ("
// 			<< percent << "%)    "
// 			<< std::flush;
// 	}

/// <summary>
/// Writing a new memory to the neural network and smooth brain.
/// n should already contain the char to write as well as the parentKey.
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
// void Brain::writeNewMemory(Neuron& n) {
// 	neuronWorker.clear();
// 	memoryWorker.clear();
// 	neuronWorker.seekp(0, std::ios_base::end);
// 	memoryWorker.seekp(0, std::ios_base::end);

// 	char keyBuffer[KEY_SIZE] = {};
// 	keyCpy(keyBuffer, n.key);

// 	char parentKeyBuffer[KEY_SIZE] = {};
// 	keyCpy(parentKeyBuffer, n.parentKey);

// 	// neuron writing [key][parentKey]
// 	neuronWorker.write(keyBuffer, KEY_SIZE);
// 	neuronWorker.write(parentKeyBuffer, KEY_SIZE);
// 	neuronWorker.flush();

// 	// memory writing [key][char][frequency]
// 	memoryWorker.write(keyBuffer, KEY_SIZE);
// 	memoryWorker.write(reinterpret_cast<const char*>(&n.ch), CHAR_SIZE);
// 	memoryWorker.write(reinterpret_cast<const char*>(&n.frequency), sizeof(n.frequency));
// 	memoryWorker.flush();
// }

// void Brain::getMeow(std::string& userInput) {
// 	std::vector<char> characterStream;
// 	const size_t maxChars = NEURON_DEPTH - 1;
// 	const size_t start = userInput.size() > maxChars ? userInput.size() - maxChars : 0;
// 	const size_t maxCharsToPrint = 200;
// 	size_t printedChars = 0;


// 	characterStream.insert(
// 		characterStream.begin(),
// 		userInput.begin() + start,
// 		userInput.end()
// 	);

// 	// >Starting at the beginning of the chain, go down the line, child-by-child, until you get to the end of the line. 
// 	// >If you cannot make it to the end of the line, begin at the next node and repeat.
// 	// >When at the end of the line, find the next node (highest freq.)
// 	// >Append the node to the end of the window
// 	// >Attempt to find the next children in the chain
// 	// >Repeat above if previous step failed
// 	for (size_t start = 0; printedChars < maxCharsToPrint && start < characterStream.size(); start++) {
// 		std::array<char, KEY_SIZE> targetParentKey = EMPTY_KEY;
// 		int64_t lastIndex = 0;
// 		bool found = false;

// 		for (size_t end = start; end < characterStream.size(); end++) {
// 			char targetChar = characterStream[end];
// 			if ((lastIndex = findChild(targetParentKey, targetChar, lastIndex)) > -1) {
// 				found = true;				
// 			}
// 			else break;
// 		}

// 		while (found && printedChars < maxCharsToPrint) {
// 			// search for and attempt to print best child node
// 			if ((lastIndex = findBestChild(targetParentKey, lastIndex)) > -1) {
// 				// we found it >:)
// 				characterStream.push_back(memoryVec[lastIndex].ch);
// 				std::cout << memoryVec[lastIndex].ch << std::flush;
// 				printedChars++;
// 			}
// 			else {
// 				found = false;
// 			}
// 		}
// 	}
// }

// int64_t Brain::findBestChild(std::array<char, KEY_SIZE>& parentKey, int64_t startInx) {
// 	uint32_t bestFrequency = 0;
// 	int64_t bestIndex = -1;
// 	for (; startInx < neuronVec.size(); startInx++) {
// 		if (keyCompare(parentKey, neuronVec[startInx].parentKey) && memoryVec[startInx].frequency > bestFrequency) {
// 			parentKey = neuronVec[startInx].key;
// 			bestFrequency = memoryVec[startInx].frequency;
// 			bestIndex = startInx;
// 		}
// 	}
// 	return bestIndex;
// }

// Helper method to return the index of a child node. Returns 0 if not found. If found, a new parentKey is assigned
// int64_t Brain::findChild(std::array<char, KEY_SIZE>& parentKey, char target, int64_t startInx) {
// 	for (; startInx < neuronVec.size(); startInx++) {
// 		if (keyCompare(parentKey, neuronVec[startInx].parentKey) && memoryVec[startInx].ch == target) {
// 			parentKey = neuronVec[startInx].key;
// 			return startInx;
// 		}
// 	}
// 	return -1;
// }