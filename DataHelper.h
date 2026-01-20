#include "Marilyn.h"
#include <fstream>
#include <iterator>
#include <random>
#include <vector>
#include <cstring>
#include <array>

#define NEURON_DEPTH 50
#define KEY_SIZE 10
#define CHAR_SIZE 1
#define MEMORY_SIZE 15

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
	char key[KEY_SIZE];
	char ch;
	uint32_t frequency;
	uint64_t position;
	char parentKey[KEY_SIZE];
};

struct MemoryNode {
	char key[KEY_SIZE];
	char ch;
	uint32_t frequency;
};

struct NeuronNode {
	char key[KEY_SIZE];
	uint64_t position;
	char parentKey[KEY_SIZE];
};

static char EMPTY_KEY[KEY_SIZE] =
	{ '0','0','0','0','0','0','0','0', '0', '0' };

std::array<char, KEY_SIZE> generate10ByteKey();
bool keyCompare(char* c1, char* c2);

class Brain {
public:
	bool initTraining();
	bool initChat();
	void processAttachedFile();
	void beginChat();
	//std::string getHighestNeruon();
	void loadBrain();
private:
	std::ifstream eyes;
	std::fstream memoryWorker;
	std::fstream neuronWorker;
	std::vector<MemoryNode> memoryVec;
	std::vector<NeuronNode> neuronVec;
	
	void resetWorkerPos(std::fstream& worker);
	void saveTrainingDataToDisk();
	//Neuron writeNewMemory(Neuron &n);
	Neuron readMemory();
	//void getMeow(std::string &userInput);
	//Neuron getNextHighestMeow(std::string parentKey);
	Neuron newNeuron();
};