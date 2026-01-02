#include "Marilyn.h"
#include "DataHelper.h"


/// <summary>
/// Generates a seriable key to be used as an identifier.
/// </summary>
/// <returns>8-Byte std::string</returns>
std::string generate8ByteKey() {
	const char availableChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#@$.()*&^%-_=+";
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::size_t> dist(0, strlen(availableChars)-1);

	std::string res;
	res.reserve(8);
	for (int i = 0; i < 8; ++i) {
		res.push_back(availableChars[dist(rng)]);
	}

	return res;
}

/// <summary>
/// 
/// </summary>
/// <param name="parentKey"></param>
/// <param name="targetChar"></param>
/// <param name="increment"></param>
/// <param name="createIfNotFound"></param>
/// <returns>Neuron struct</returns>
Neuron Brain::getNeuronByTargetAndParentKey(
		std::string parentKey, 
		char targetChar, 
		bool increment, 
		bool createIfNotFound) {
	Neuron res { EMPTY_KEY, { 0 }, 0, 0, parentKey};
	resetWorkerPos(neuronWorker);
	//[key] [position] [parentKey]
	//[key] [char]     [frequency]
	// Until EOF of neuronWorker, read through each node,
	// stopping when parentKey is found in the parentKey position. 
	// Use the memoryWorker to read the node at the indicated position.
	// Stop when found or EOF, perform actions if requested.
	bool notFound = false;
	while (!notFound) {
		res = readMemory();
		notFound = res.key == EMPTY_KEY;

		if (!notFound && res.ch == targetChar && res.parentKey == parentKey) break;
	}

	if (notFound && createIfNotFound) {
		Neuron newNeuron{ EMPTY_KEY, targetChar, 0, 0, parentKey };
		res = writeNewMemory(newNeuron);
	}
	else if (increment) {

	}

	return res;
}

void Brain::resetWorkerPos(std::fstream &worker) {
	worker.clear(); // clear eofbit / failbit / badbit
	worker.seekg(0, std::ios_base::beg);
	worker.seekp(0, std::ios_base::beg);
}

/// <summary>
/// If Neuron.key is empty, there was an error or we have reached EOF.
/// </summary>
/// <returns></returns>
Neuron Brain::readMemory() {
	Neuron res{ EMPTY_KEY, '\0', 0, 0, EMPTY_KEY};
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

	res.key.assign(keyBuffer, KEY_SIZE);
	res.ch = charBuffer;
	res.frequency = freqBuffer;
	res.position = posBuffer;
	res.parentKey.assign(parentKeyBuf, KEY_SIZE);

	return res;
}

/// <summary>
/// Writing a new memory to the neural network and smooth brain.
/// n should already contain the char to write as well as the parentKey.
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
Neuron Brain::writeNewMemory(Neuron& n) {
	neuronWorker.clear();
	memoryWorker.clear();
	neuronWorker.seekp(0, std::ios_base::end);
	memoryWorker.seekp(0, std::ios_base::end);

	n.key = generate8ByteKey();
	n.position = static_cast<uint64_t>(memoryWorker.tellp());
	n.frequency = 1;

	char keyBuffer[KEY_SIZE] = {};
	std::memcpy(keyBuffer, n.key.data(), std::min(n.key.size(), KEY_SIZE));

	char parentKeyBuf[KEY_SIZE] = {};
	std::memcpy(parentKeyBuf, n.parentKey.data(), std::min(n.parentKey.size(), KEY_SIZE));

	// neuron writing [key][position][parentKey]
	neuronWorker.write(keyBuffer, KEY_SIZE);
	neuronWorker.write(reinterpret_cast<const char*>(&n.position), sizeof(n.position));
	neuronWorker.write(parentKeyBuf, KEY_SIZE);
	neuronWorker.flush();

	// memory writing [key][char][frequency]
	memoryWorker.write(keyBuffer, KEY_SIZE);
	memoryWorker.write(reinterpret_cast<const char*>(&n.ch), CHAR_SIZE);
	memoryWorker.write(reinterpret_cast<const char*>(&n.frequency), sizeof(n.frequency));
	memoryWorker.flush();

	return n;
}

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