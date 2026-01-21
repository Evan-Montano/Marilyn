#include "Marilyn.h"
#include "DataHelper.h"

/// <summary>
/// Generates a seriable key to be used as an identifier.
/// </summary>
/// <returns>8-Byte std::string</returns>
std::array<char, KEY_SIZE> generate10ByteKey() {
	const char availableChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#@$.()*&^%-_=+";
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::size_t> dist(0, strlen(availableChars)-1);

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
	resetWorkerPos(neuronWorker);
	Neuron res;
	uint64_t totalSize = neuronVec.size();

	// we first delete the old files, then write new ones
	memoryWorker.close();
	neuronWorker.close();
	std::filesystem::remove("../../../SmoothBrain/Marilyn.brain");
	std::filesystem::remove("../../../SmoothBrain/Marilyn.neurons");

	{
		std::ofstream("../../../SmoothBrain/Marilyn.brain", std::ios::binary | std::ios::app);
		std::ofstream("../../../SmoothBrain/Marilyn.neurons", std::ios::binary | std::ios::app);
	}

	// reopen for read/write
	memoryWorker.open("../../../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
	neuronWorker.open("../../../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

	for (uint64_t inx = 0; inx < totalSize; inx++) {
		res.key = neuronVec[inx].key;
		res.parentKey = neuronVec[inx].parentKey;
		res.position = neuronVec[inx].position;
		res.frequency = memoryVec[inx].frequency;
		res.ch = memoryVec[inx].ch;

		writeNewMemory(res);

		double percent =
			(double(inx) / double(totalSize)) * 100.0;
		std::cout << "\r"
			<< inx << " / "
			<< totalSize << " bytes ("
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
	std::cout << ">>";
	std::vector<char> characterStream;
	const size_t maxChars = NEURON_DEPTH - 1;
	const size_t start = userInput.size() > maxChars ? userInput.size() - maxChars : 0;

	characterStream.insert(
		characterStream.begin(),
		userInput.begin() + start,
		userInput.end()
	);

	// parsing through the inputted character stream
	for (size_t i = 0; i < characterStream.size() - 1; i++) {
		// reset variables
		uint64_t neuronPos = 0;
		std::array<char, KEY_SIZE> parentKey = EMPTY_KEY;

		// inner loop of input stream for finding largest chain
		for (size_t j = i; j < characterStream.size(); j++) {
			char targetC = characterStream[j];

			// === Node Search === //
			for (; neuronPos < neuronVec.size(); neuronPos++) {
				if (neuronVec[neuronPos].parentKey == parentKey
						&& memoryVec[neuronPos].ch == targetC) {
					// Root found
					parentKey = neuronVec[neuronPos].key;
					break;
				}
			}// === End Node Search === //

			// if we made it to the end of the chain,
			// search for the next character.
			if (j >= characterStream.size()-1) {
				uint32_t bestFrequency = 0;
				uint64_t bestCharInx = neuronPos;

				for (; neuronPos < neuronVec.size(); neuronPos++) {
					if (neuronVec[neuronPos].parentKey == parentKey
						&& memoryVec[neuronPos].frequency > bestFrequency) {
						bestFrequency = memoryVec[neuronPos].frequency;
						bestCharInx = neuronPos;
					}
				}
				if (bestFrequency == 0) break;

				// At this point we can assume that we found the next character to print
				std::cout << memoryVec[bestCharInx].ch;

				if (characterStream.size() >= NEURON_DEPTH) {
					characterStream.erase(characterStream.begin());
				}
				characterStream.push_back(memoryVec[bestCharInx].ch);
			}

		}
	}
}