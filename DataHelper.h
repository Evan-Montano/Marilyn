#pragma once

#include "Marilyn.h"
#include <fstream>
#include <filesystem>
#include <iterator>
#include <random>
#include <vector>
#include <array>
#include <deque>
#include <unordered_map>
#include <algorithm>
#include <future>
#include <mutex>
#include <cmath>

constexpr size_t KEY_SIZE = 6;
constexpr size_t CHAR_SIZE = 1;
constexpr size_t NEURON_DEPTH = 14;
inline constexpr std::array<char, KEY_SIZE> EMPTY_KEY = 
	{ '0','0','0','0','0','0' };
inline constexpr std::array<char, 4> PUNCTUATION =
	{ '.', '!', '?' };

struct BrainCell {
	std::array<char, KEY_SIZE+1> idHash;
	std::array<char, KEY_SIZE> key;
	uint8_t freq;
};

struct Neuron {
	std::array<char, KEY_SIZE> idHash;
	std::vector<char> children;
};

struct Node {
	std::array<char, KEY_SIZE> key;
	uint8_t frequency;
};

struct Candidate {
	char c;
	double weight;
};

std::array<char, KEY_SIZE> generate6ByteKey();

// Custom hash function for std::array<char, N>
struct ArrayHasher {
	template <size_t N>
	std::size_t operator()(const std::array<char, N>& arr) const {
		std::size_t h = 0;
		for (auto e : arr) {
			// Combine the hash of the current character with the existing hash
			// This is a standard hash combination algorithm (from Boost)
			h ^= std::hash<char>{}(e) + 0x9e3779b9 + (h << 6) + (h >> 2);
		}
		return h;
	}
};

class Brain {
public:
	std::fstream inFile;
	std::mutex coutMutex;
	void beginChat();
	void processAttachedFile();
	bool loadBrain();
	bool loadNeurons();
private:
	std::fstream brainWorker;
	std::fstream neuronWorker;
	std::unordered_map<std::array<char, KEY_SIZE+1>, Node, ArrayHasher> brainMap;
	std::unordered_map<std::array<char, KEY_SIZE>, std::vector<char>, ArrayHasher> neuronMap;
	BrainCell readBrainCell();
	Neuron readNeuron();
	std::streampos getFileSize(std::fstream& file);
	void saveDataToDisk();
	bool writeToBrain(std::pair<const std::array<char, KEY_SIZE+1>, Node> &rec);
	bool writeToNetwork(std::pair<const std::array<char, KEY_SIZE>, std::vector<char>> &rec);
	void getMeow(std::string &userInput);
};

inline void moveCursorToLine(int line) {
	std::cout << "\033[" << line << ";1H";
}