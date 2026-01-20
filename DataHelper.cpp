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

	std::array<char, KEY_SIZE> res;

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
bool keyCompare(char* c1, char* c2) {
	for (int i = 0; i < KEY_SIZE; ++i) {
		if (c1[i] != c2[i]) {
			return false;
		}
	}
	return true;
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
	std::memcpy(res.key, EMPTY_KEY, KEY_SIZE);
	std::memcpy(res.parentKey, EMPTY_KEY, KEY_SIZE);
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
	
	std::memcpy(res.key, keyBuffer, KEY_SIZE);
	res.ch = charBuffer;
	res.frequency = freqBuffer;
	res.position = posBuffer;
	std::memcpy(res.parentKey, parentKeyBuf, KEY_SIZE);

	return res;
}

void Brain::loadBrain() {
	resetWorkerPos(neuronWorker);
	MemoryNode memN{};
	NeuronNode neurN{};
	Neuron res;

	while (true) {
		res = readMemory();
		// Break if the key is empty
		// Or if the character is the null terminator (default)
		if (std::memcmp(res.key, EMPTY_KEY, KEY_SIZE) == 0
			|| res.ch == '\0') {
			break;
		}

		// Copy into neuron node
		std::memcpy(neurN.key, res.key, KEY_SIZE);
		neurN.position = res.position;
		std::memcpy(neurN.parentKey, res.parentKey, KEY_SIZE);

		// Copy into brain node
		std::memcpy(memN.key, res.key, KEY_SIZE);
		memN.ch = res.ch;
		memN.frequency = res.frequency;

		neuronVec.push_back(neurN);
		memoryVec.push_back(memN);
	}
}

void Brain::saveTrainingDataToDisk() {
	resetWorkerPos(neuronWorker);
	Neuron res;

	for (NeuronNode neuron: neuronVec) {
		std::string key(neuron.key, KEY_SIZE);
		std::string parentKey(neuron.parentKey, KEY_SIZE);
		std::cout << key
				  << neuron.position
				  << parentKey
				  << std::endl;
	}
}

///// <summary>
///// Increments the frequency of the current memory.
///// We should already have the exact position of the memory.
///// </summary>
///// <param name="n"></param>
///// <returns></returns>
//Neuron Brain::incrementMemory(Neuron& n) {
//	if (std::memcmp(n.key.data(), EMPTY_KEY, KEY_SIZE) != 0) {
//		memoryWorker.clear();
//
//		const std::streampos freqPos =
//			n.position + KEY_SIZE + CHAR_SIZE;
//
//		uint32_t tmpFreq;
//
//		// Read
//		memoryWorker.seekg(freqPos, std::ios::beg);
//		if (!memoryWorker.read(reinterpret_cast<char*>(&tmpFreq), sizeof(tmpFreq)))
//			return n;
//
//		if (tmpFreq != n.frequency)
//			return n;
//
//		// Increment
//		++tmpFreq;
//
//		// Write back to the SAME location
//		memoryWorker.seekp(freqPos, std::ios::beg);
//		if (!memoryWorker.write(reinterpret_cast<const char*>(&tmpFreq), sizeof(tmpFreq)))
//			return n;
//
//		memoryWorker.flush();
//		n.frequency = tmpFreq;
//	}
//	return n;
//}

/// <summary>
/// Writing a new memory to the neural network and smooth brain.
/// n should already contain the char to write as well as the parentKey.
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
//Neuron Brain::writeNewMemory(Neuron& n) {
//	neuronWorker.clear();
//	memoryWorker.clear();
//	neuronWorker.seekp(0, std::ios_base::end);
//	memoryWorker.seekp(0, std::ios_base::end);
//
//	n.key = generate8ByteKey();
//	n.position = static_cast<uint64_t>(memoryWorker.tellp());
//	n.frequency = 1;
//
//	char keyBuffer[KEY_SIZE] = {};
//	std::memcpy(keyBuffer, n.key.data(), std::min(n.key.size(), KEY_SIZE));
//
//	char parentKeyBuf[KEY_SIZE] = {};
//	std::memcpy(parentKeyBuf, n.parentKey.data(), std::min(n.parentKey.size(), KEY_SIZE));
//
//	// neuron writing [key][position][parentKey]
//	neuronWorker.write(keyBuffer, KEY_SIZE);
//	neuronWorker.write(reinterpret_cast<const char*>(&n.position), sizeof(n.position));
//	neuronWorker.write(parentKeyBuf, KEY_SIZE);
//	neuronWorker.flush();
//
//	// memory writing [key][char][frequency]
//	memoryWorker.write(keyBuffer, KEY_SIZE);
//	memoryWorker.write(reinterpret_cast<const char*>(&n.ch), CHAR_SIZE);
//	memoryWorker.write(reinterpret_cast<const char*>(&n.frequency), sizeof(n.frequency));
//	memoryWorker.flush();
//
//	return n;
//}
//
//void Brain::getMeow(std::string &userInput) {
//	std::cout << ">>";
//
//	std::vector<char> characterStream;
//	const size_t maxChars = NEURON_DEPTH - 1;
//	const size_t start =
//		userInput.size() > maxChars ? userInput.size() - maxChars : 0;
//
//	characterStream.insert(
//		characterStream.begin(),
//		userInput.begin() + start,
//		userInput.end()
//	);
//
//	//Neuron bestMeow{ EMPTY_KEY, '\0', 0, 0, EMPTY_KEY };
//	for (size_t i = 0; i < characterStream.size()-1; ++i) {
//		resetWorkerPos(neuronWorker);
//		// getting root node
//		Neuron nextChar = getNeuronByTargetAndParentKey(
//			EMPTY_KEY, characterStream[i], false, false);
//
//		for (size_t j = i + 1; j <= characterStream.size(); ++j) {
//			// if we are at the end of the character stream, search for the next character
//			if (j >= characterStream.size()) {
//				nextChar = getNextHighestMeow(nextChar.key);
//				if (nextChar.ch == '\0') break;
//				
//				// print the next meow
//				std::cout << nextChar.ch;
//
//				if (characterStream.size() >= NEURON_DEPTH) {
//					characterStream.erase(characterStream.begin());
//				}
//				characterStream.push_back(nextChar.ch);
//			}
//			else {
//				nextChar = getNeuronByTargetAndParentKey(
//					nextChar.key, characterStream[j], false, false);
//				if (nextChar.ch == '\0' && nextChar.key == EMPTY_KEY) break;
//			}
//		}
//	}
//}
//
//Neuron Brain::getNextHighestMeow(std::string parentKey) {
//	Neuron res { EMPTY_KEY, '\0', 0, 0, parentKey };
//	std::vector<Neuron> matches;
//	resetWorkerPos(neuronWorker);
//
//	bool eof = false;
//	while (!eof) {
//		Neuron tmp = readMemory();
//		eof = ((tmp.key.size() != KEY_SIZE) || (tmp.ch == '\0'));
//		if (!eof && tmp.parentKey == parentKey) {
//			matches.push_back(tmp);
//		}
//	}
//
//	// Get the highest frequency
//	for (Neuron n : matches) {
//		if (n.frequency > res.frequency) {
//			res = n;
//		}
//	}
//
//	return res;
//}
//
//std::string Brain::getHighestNeruon() {
//	Neuron res{ EMPTY_KEY, '\0', 0, 0, EMPTY_KEY };
//	resetWorkerPos(neuronWorker);
//	bool eof = false;
//	while (!eof) {
//		Neuron tmp = readMemory();
//		eof = ((tmp.key.size() != KEY_SIZE) || (tmp.ch == '\0'));
//		if (!eof && tmp.frequency >= res.frequency) {
//			res = tmp;
//		}
//	}
//
//	std::string ret =
//		"Key: " + res.key +
//		"    char: " + std::string(1, res.ch) +
//		"    frequency: " + std::to_string(res.frequency);
//
//	return ret;
//}

//NOTES:
// char buffer[8];
// neuronWorker.read(reinterpret_cast<char*>(buffer), KEY_SIZE);

// char t = 'a';
// memoryWorker.write(reinterpret_cast<const char*>(t), CHAR_SIZE);
// 
//std::fstream has pointers for the in and out streams, which are not synced up automatically.
//
//Seeking:
//seekg(n) (get)
//seekp(n) (put)
//
//ios_base::beg	beginning of the stream
//ios_base::cur	current position in the stream
//ios_base::end	end of the stream