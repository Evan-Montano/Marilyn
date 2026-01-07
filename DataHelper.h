#include "Marilyn.h"
#include <fstream>
#include <iterator>
#include <random>
#include <vector>

/// <summary>
/// This is just a helper struct for processing text files
/// to make sure we aren't processing the same characters
/// many times over through the sliding window.
/// </summary>
struct EyeCandy {
	char ch;
	bool proc;
};

struct Neuron {
	std::string key;
	char ch;
	uint32_t frequency;
	uint64_t position;
	std::string parentKey;
};

const char EMPTY_KEY[9] =
	{ '0','0','0','0','0','0','0','0','\0' };
const int NEURON_DEPTH = 50;
const size_t KEY_SIZE = 8;
const size_t CHAR_SIZE = 1;

std::string generate8ByteKey();

class Brain {
public:
	bool initTraining();
	bool initChat();
	void processAttachedFile();
	void beginChat();
	std::string getHighestNeruon();
private:
	std::ifstream eyes;
	std::fstream memoryWorker;
	std::fstream neuronWorker;

	Neuron getNeuronByTargetAndParentKey(
		std::string parentKey,
		char targetChar,
		bool increment,
		bool createIfNotFound);
	
	void resetWorkerPos(std::fstream& worker);
	Neuron writeNewMemory(Neuron &n);
	Neuron readMemory();
	Neuron incrementMemory(Neuron& n);
	void getMeow(std::string &userInput);
	Neuron getNextHighestMeow(std::string parentKey);
};