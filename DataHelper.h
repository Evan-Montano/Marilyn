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
struct eyeCandy {
	char ch;
	bool proc;
};

const char EMPTY_KEY[8] = "0000000";
const int NEURON_DEPTH = 20;

std::string generate8ByteKey();

class BrainSignal {
public:
	bool initTraining();
	void processAttachedFile();
private:
	std::ifstream eyes;
	std::ofstream memoryWriter;
	std::ofstream neuronWriter;
};