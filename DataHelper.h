#include "Marilyn.h"
#include <fstream>
#include <filesystem>
#include <iterator>
#include <random>
#include <vector>
#include <cstring>
#include <array>
#include <deque>
#include <unordered_map>

constexpr size_t KEY_SIZE = 10;
constexpr size_t CHAR_SIZE = 1;
inline constexpr std::array<char, KEY_SIZE> EMPTY_KEY = 
	{ '0','0','0','0','0','0','0','0', '0', '0' };

struct BrainCell {
	std::array<char, KEY_SIZE+1> idHash;
	std::array<char, KEY_SIZE> key;
	uint32_t freq;
};

struct Neuron {
	std::array<char, KEY_SIZE> idHash;
	std::vector<char> children;
};

struct Node {
	std::array<char, KEY_SIZE> key;
	uint32_t frequency;
};

std::array<char, KEY_SIZE> generate10ByteKey();

class Brain {
public:
	// bool initTraining();
	// bool initChat();
	// void processAttachedFile();
	// void beginChat();
	//std::string getHighestNeruon();
	// void loadBrain();
	// bool init();
	bool loadBrain();
	bool loadNeurons();
	std::ifstream inFile;
private:
	std::fstream brainWorker;
	std::fstream neuronWorker;
	std::unordered_map<std::array<char, KEY_SIZE+1>, Node> brainMap;
	std::unordered_map<std::array<char, KEY_SIZE>, std::vector<char>> neuronMap;
	BrainCell readBrainCell();
	Neuron readNeuron();
	std::streampos getFileSize(std::fstream& file);
	// void writeNewMemory(Neuron& n);
	// void resetWorkerPos(std::fstream& worker);
	// void saveTrainingDataToDisk();
	// void getMeow(std::string &userInput);
	// int64_t findChild(std::array<char, KEY_SIZE>& parentKey, char target, int64_t startInx = 0);
	// int64_t findBestChild(std::array<char, KEY_SIZE>& parentKey, int64_t startInx = 0);
	// Neuron newNeuron();
};