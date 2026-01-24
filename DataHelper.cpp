#include "Marilyn.h"
#include "DataHelper.h"

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

	for (int i = 0; i < KEY_SIZE; ++i) {
		res[i] = availableChars[dist(rng)];
	}

	return res;
}

/// <summary>
/// Helper method to compare two 8 byte keys.
/// </summary>
/// <param name="c1"></param>
/// <param name="c2"></param>
/// <returns></returns>
bool keyCompare(std::array<char, KEY_SIZE> c1, std::array<char, KEY_SIZE> c2) {
	for (int i = 0; i < KEY_SIZE; ++i) {
		if (c1[i] != c2[i]) {
			return false;
		}
	}
	return true;
}

/// <summary>
/// Helper method to copy array values over
/// </summary>
/// <param name="c1"></param>
/// <param name="c2"></param>
void keyCpy(std::array<char, KEY_SIZE>& c1, std::array<char, KEY_SIZE> c2) {
	for (int i = 0; i < KEY_SIZE; ++i) {
		c1[i] = c2[i];
	}
}

void keyCpy(std::array<char, KEY_SIZE>& c1, char c2[KEY_SIZE]) {
	for (int i = 0; i < KEY_SIZE; ++i) {
		c1[i] = c2[i];
	}
}

void keyCpy(char c1[KEY_SIZE], std::array<char, KEY_SIZE> c2) {
	for (int i = 0; i < KEY_SIZE; ++i) {
		c1[i] = c2[i];
	}
}

/// <summary>
/// Resets the position of the passed
/// worker.
/// </summary>
/// <param name="worker"></param>
void Brain::resetWorkerPos(std::fstream& worker) {
	worker.clear(); // clear eofbit / failbit / badbit
	worker.seekg(0, std::ios_base::beg);
	worker.seekp(0, std::ios_base::beg);
}

Neuron Brain::newNeuron() {
	Neuron res{};
	res.key = EMPTY_KEY;
	res.parentKey = EMPTY_KEY;
	return res;
}

///// <summary>
///// If Neuron.key is empty, there was an error or we have reached EOF.
///// </summary>
///// <returns></returns>
Neuron Brain::readMemory() {
	Neuron res = newNeuron();
	neuronWorker.clear();
	memoryWorker.clear();

	char keyBuffer[KEY_SIZE];
	char confirmKeyBuffer[KEY_SIZE];
	char parentKeyBuf[KEY_SIZE];
	char charBuffer = '\0';
	uint32_t freqBuffer;
	uint64_t posBuffer;

	// neuron reading [key][position][parentKey]
	if (!neuronWorker.read(keyBuffer, KEY_SIZE)) {
		return res;
	}
	if (!neuronWorker.read(reinterpret_cast<char*>(&posBuffer), sizeof(posBuffer))) {
		return res;
	}
	if (!neuronWorker.read(parentKeyBuf, KEY_SIZE)) {
		return res;
	}

	// memory reading [key][char][frequency]
	memoryWorker.seekg(posBuffer, std::ios::beg);
	if (!memoryWorker.read(confirmKeyBuffer, KEY_SIZE) || std::memcmp(keyBuffer, confirmKeyBuffer, KEY_SIZE) != 0) {
		return res;
	}
	if (!memoryWorker.read(&charBuffer, CHAR_SIZE)) {
		return res;
	}
	if (!memoryWorker.read(reinterpret_cast<char*>(&freqBuffer), sizeof(freqBuffer))) {
		return res;
	}
	
	keyCpy(res.key, keyBuffer);
	res.ch = charBuffer;
	res.frequency = freqBuffer;
	res.position = posBuffer;
	keyCpy(res.parentKey, parentKeyBuf);

	return res;
}

void Brain::loadBrain() {
	resetWorkerPos(neuronWorker);
	MemoryNode memN{};
	NeuronNode neurN{};
	Neuron res{};

	while (true) {
		res = readMemory();
		// Break if the key is empty
		// Or if the character is the null terminator (default)
		if (keyCompare(res.key, EMPTY_KEY) == true
			|| res.ch == '\0') {
			break;
		}

		// Copy into neuron node
		keyCpy(neurN.key, res.key);
		neurN.position = res.position;
		keyCpy(neurN.parentKey, res.parentKey);

		// Copy into brain node
		keyCpy(memN.key, res.key);
		memN.ch = res.ch;
		memN.frequency = res.frequency;

		neuronVec.push_back(neurN);
		memoryVec.push_back(memN);
	}
}

void Brain::saveTrainingDataToDisk() {
	Neuron res;
	uint64_t totalSize = neuronVec.size();

	// we first delete the old files, then write new ones
	memoryWorker.close();
	neuronWorker.close();
	std::filesystem::remove("../SmoothBrain/Marilyn.brain");
	std::filesystem::remove("../SmoothBrain/Marilyn.neurons");

	{
		std::ofstream("../SmoothBrain/Marilyn.brain", std::ios::binary | std::ios::app);
		std::ofstream("../SmoothBrain/Marilyn.neurons", std::ios::binary | std::ios::app);
	}

	// reopen for read/write
	memoryWorker.open("../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
	neuronWorker.open("../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

	for (uint64_t inx = 0; inx < totalSize; inx++) {
		res.key = neuronVec[inx].key;
		res.parentKey = neuronVec[inx].parentKey;
		res.position = neuronVec[inx].position;
		res.frequency = memoryVec[inx].frequency;
		res.ch = memoryVec[inx].ch;

		writeNewMemory(res);

		double percent =
			(double(inx+1) / double(totalSize)) * 100.0;
		std::cout << "\r"
			<< inx << " / "
			<< totalSize << " Meows saved ("
			<< percent << "%)    "
			<< std::flush;
	}
}

/// <summary>
/// Writing a new memory to the neural network and smooth brain.
/// n should already contain the char to write as well as the parentKey.
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
void Brain::writeNewMemory(Neuron& n) {
	neuronWorker.clear();
	memoryWorker.clear();
	neuronWorker.seekp(0, std::ios_base::end);
	memoryWorker.seekp(0, std::ios_base::end);

	char keyBuffer[KEY_SIZE] = {};
	keyCpy(keyBuffer, n.key);

	char parentKeyBuffer[KEY_SIZE] = {};
	keyCpy(parentKeyBuffer, n.parentKey);

	// neuron writing [key][position][parentKey]
	neuronWorker.write(keyBuffer, KEY_SIZE);
	neuronWorker.write(reinterpret_cast<const char*>(&n.position), sizeof(n.position));
	neuronWorker.write(parentKeyBuffer, KEY_SIZE);
	neuronWorker.flush();

	// memory writing [key][char][frequency]
	memoryWorker.write(keyBuffer, KEY_SIZE);
	memoryWorker.write(reinterpret_cast<const char*>(&n.ch), CHAR_SIZE);
	memoryWorker.write(reinterpret_cast<const char*>(&n.frequency), sizeof(n.frequency));
	memoryWorker.flush();
}

void Brain::getMeow(std::string& userInput) {
	std::vector<char> characterStream;
	const size_t maxChars = NEURON_DEPTH - 1;
	const size_t start = userInput.size() > maxChars ? userInput.size() - maxChars : 0;
	const size_t maxCharsToPrint = 200;
	size_t printedChars = 0;


	characterStream.insert(
		characterStream.begin(),
		userInput.begin() + start,
		userInput.end()
	);

	// >Starting at the beginning of the chain, go down the line, child-by-child, until you get to the end of the line. 
	// >If you cannot make it to the end of the line, begin at the next node and repeat.
	// >When at the end of the line, find the next node (highest freq.)
	// >Append the node to the end of the window
	// >Attempt to find the next children in the chain
	// >Repeat above if previous step failed
	for (size_t start = 0; printedChars < maxCharsToPrint, start < characterStream.size(); start++) {
		std::array<char, KEY_SIZE> targetParentKey = EMPTY_KEY;
		int64_t lastIndex = 0;
		bool found = false;

		for (size_t end = start; end < characterStream.size(); end++) {
			char targetChar = characterStream[end];
			if ((lastIndex = findChild(targetParentKey, targetChar, lastIndex)) > -1) {
				found = true;				
			}
			else break;
		}

		while (found && printedChars < maxCharsToPrint) {
			// search for and attempt to print best child node
			if ((lastIndex = findBestChild(targetParentKey, lastIndex)) > -1) {
				// we found it >:)
				characterStream.push_back(memoryVec[lastIndex].ch);
				std::cout << memoryVec[lastIndex].ch << std::flush;
				printedChars++;
			}
			else {
				found = false;
			}
		}
	}
}

int64_t Brain::findBestChild(std::array<char, KEY_SIZE>& parentKey, int64_t startInx) {
	uint32_t bestFrequency = 0;
	int64_t bestIndex = -1;
	for (; startInx < neuronVec.size(); startInx++) {
		if (keyCompare(parentKey, neuronVec[startInx].parentKey) && memoryVec[startInx].frequency > bestFrequency) {
			parentKey = neuronVec[startInx].key;
			bestFrequency = memoryVec[startInx].frequency;
			bestIndex = startInx;
		}
	}
	return bestIndex;
}

// Helper method to return the index of a child node. Returns 0 if not found. If found, a new parentKey is assigned
int64_t Brain::findChild(std::array<char, KEY_SIZE>& parentKey, char target, int64_t startInx) {
	for (; startInx < neuronVec.size(); startInx++) {
		if (keyCompare(parentKey, neuronVec[startInx].parentKey) && memoryVec[startInx].ch == target) {
			parentKey = neuronVec[startInx].key;
			return startInx;
		}
	}
	return -1;
}