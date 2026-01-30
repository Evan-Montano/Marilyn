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
	brainWorker = std::fstream("../../../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
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
	int lastPercent = -1;
	while (cell = readBrainCell(), !brainWorker.eof()) {
		std::memcpy(compositeKey.data(), cell.idHash.data(), KEY_SIZE+1);
		node.key = cell.key;
		node.frequency = cell.freq;
		brainMap[compositeKey] = node;

		// Print progress update
		int percent = int((double(brainWorker.tellg()) / double(fileSize)) * 100.0);
		if (percent != lastPercent) { // only print when percentage increases
			std::cout << "\rLoading brain: " << percent << "%";
			lastPercent = percent;
		}
		
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
	neuronWorker = std::fstream("../../../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);
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
	int lastPercent = -1;
	while (neuron = readNeuron(), !neuronWorker.eof()) {
		parentKey = neuron.idHash;
		neuronMap[parentKey] = neuron.children;

		// Print progress update
		int percent = int((double(neuronWorker.tellg()) / double(fileSize)) * 100.0);
		if (percent != lastPercent) { // only print when percentage increases
			std::cout << "\rLoading neurons: " << percent << "%";
			lastPercent = percent;
		}
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
	std::filesystem::remove("../../../SmoothBrain/Marilyn.brain");
	std::filesystem::remove("../SmoothBrain/Marilyn.neurons");
	{
		std::ofstream("../../../SmoothBrain/Marilyn.brain", std::ios::binary | std::ios::app);
		std::ofstream("../../../SmoothBrain/Marilyn.neurons", std::ios::binary | std::ios::app);
	}
	brainWorker.open("../../../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
 	neuronWorker.open("../../../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

	uint64_t inx = 0;
	uint64_t totalSize = brainMap.size();
	int lastPercent = -1;
	for (std::pair<const std::array<char, 11>, Node> & rec : brainMap) {
		if (writeToBrain(rec) == false) {
			std::cout << "An error occured while writing to brain.." << std::endl;
			return;
		}

		++inx;
		int percent = int((double(inx) / double(totalSize)) * 100.0);
		if (percent != lastPercent) { // only print when percentage increases
			std::cout << "\rWriting brain: " << percent << "%";
			lastPercent = percent;
		}
	}
	brainWorker.flush();

	inx = 0;
	totalSize = neuronMap.size();
	lastPercent = -1;
	for (std::pair<const std::array<char, 10>, std::vector<char>> & rec : neuronMap) {
		if (writeToNetwork(rec) == false) {
			std::cout << "An error occured while writing to network.." << std::endl;
			return;
		}
		
		++inx;
		int percent = int((double(inx) / double(totalSize)) * 100.0);
		if (percent != lastPercent) {
			std::cout << "\rWriting neurons: " << percent << "%";
			lastPercent = percent;
		}
	}
	neuronWorker.flush();
	std::cout << std::endl << std::flush;
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

void Brain::getMeow(std::string& userInput) {
	std::vector<char> characterStream;
	const size_t maxChars = NEURON_DEPTH - 1;
	const size_t start = userInput.size() > maxChars ? userInput.size() - maxChars : 0;
	const size_t maxCharsToPrint = 200;
	const size_t maxLapses = 100;
	const size_t maxPunctuation = 3;
	size_t printedChars = 0;

	characterStream.insert(
		characterStream.begin(),
		userInput.begin() + start,
		userInput.end()
	);

	std::array<char, KEY_SIZE> parentKey = {};
	std::array<char, KEY_SIZE+1> compositeKey = {};
	std::vector<char> existingChars = {};

	// std::unordered_map<(parentKey, char), Node> brainMap
	// std::unordered_map<parentKey, vector<char>> neuronMap
	while (printedChars <= maxCharsToPrint) {
		parentKey = EMPTY_KEY;
		size_t lapses = 0;
		size_t punct = 0;
		for (size_t i = 0; i <= characterStream.size() && printedChars <= maxCharsToPrint; i++) {
			// Check neuronMap vector for existing characterStream[i]
			// If node exists, get the key from brainMap
			if (i < characterStream.size()) {
				existingChars = neuronMap[parentKey];
				if (std::find(existingChars.begin(), existingChars.end(), characterStream[i]) != existingChars.end()) {
					std::memcpy(compositeKey.data(), parentKey.data(), KEY_SIZE);
					compositeKey[KEY_SIZE] = characterStream[i];
					parentKey = brainMap[compositeKey].key;
				}
				else {
					// reset to root node
					parentKey = EMPTY_KEY;
				}
			}
			else {
				// Get next char for printing
				existingChars = neuronMap[parentKey];
				uint8_t highestFreq = 0;
				char charToPrint = '\0';
				std::memcpy(compositeKey.data(), parentKey.data(), KEY_SIZE);
				for (char c : existingChars) {
					compositeKey[KEY_SIZE] = c;
					if (brainMap[compositeKey].frequency >= highestFreq) {
						charToPrint = c;
						highestFreq = brainMap[compositeKey].frequency;
					}
				}
				if ((charToPrint == '\n' || charToPrint == '\r') && printedChars > 0) {
					std::cout << std::endl << "Broke from return line";
					return;
				}
				if (charToPrint != '\0') {
					compositeKey[KEY_SIZE] = charToPrint;
					parentKey = brainMap[compositeKey].key;

					// print char to console
					std::cout << charToPrint << std::flush;
					printedChars++;

					if (charToPrint == ' ' && std::find(PUNCTUATION.begin(), PUNCTUATION.end(), characterStream.size() - 1) != PUNCTUATION.end()) {
						++punct;
						if (punct >= maxPunctuation) {
							std::cout << std::endl << "Broke from punctuation";
							return;
						}
					}

					characterStream.push_back(charToPrint);

					//if (std::find(PUNCTUATION.begin(), PUNCTUATION.end(), charToPrint) != PUNCTUATION.end()) ++punct;
					/*if (punct >= maxPunctuation) {
						std::cout << std::endl << "Broke from punctuation";
						return;
					}*/
				}
				else if (lapses <= maxLapses) {
					parentKey = EMPTY_KEY;
					++printedChars;
					if (i - maxChars >= 0) i -= maxChars;
					else i = 0;
					++lapses;
				}
			}

		}
	}
	std::cout << std::endl << "Broke from max chars";
}