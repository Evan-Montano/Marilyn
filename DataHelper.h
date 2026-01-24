#include "Marilyn.h"
#include <fstream>
#include <filesystem>
#include <iterator>
#include <random>
#include <vector>
#include <cstring>
#include <array>
#include <deque>

#define NEURON_DEPTH 20
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
	std::array<char, KEY_SIZE> key{};
	char ch;
	uint32_t frequency;
	uint64_t position;
	std::array<char, KEY_SIZE> parentKey{};
};

struct MemoryNode {
	std::array<char, KEY_SIZE> key{};
	char ch;
	uint32_t frequency;
};

struct NeuronNode {
	std::array<char, KEY_SIZE> key{};
	uint64_t position;
	std::array<char, KEY_SIZE> parentKey{};
};

inline constexpr std::array<char, KEY_SIZE> EMPTY_KEY = 
	{ '0','0','0','0','0','0','0','0', '0', '0' };

std::array<char, KEY_SIZE> generate10ByteKey();
bool keyCompare(std::array<char, KEY_SIZE> c1, std::array<char, KEY_SIZE> c2);
void keyCpy(std::array<char, KEY_SIZE>& c1, std::array<char, KEY_SIZE> c2);
void keyCpy(std::array<char, KEY_SIZE>& c1, char c2[KEY_SIZE]);
void keyCpy(char c1[KEY_SIZE], std::array<char, KEY_SIZE> c2);

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
	void writeNewMemory(Neuron& n);
	void resetWorkerPos(std::fstream& worker);
	void saveTrainingDataToDisk();
	Neuron readMemory();
	void getMeow(std::string &userInput);
	int64_t findChild(std::array<char, KEY_SIZE>& parentKey, char target, int64_t startInx = 0);
	int64_t findBestChild(std::array<char, KEY_SIZE>& parentKey, int64_t startInx = 0);
	Neuron newNeuron();
};