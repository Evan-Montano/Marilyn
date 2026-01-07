#include "Marilyn.h"
#include "Train.h"
#include "DataHelper.h"

Brain noLumpsOrBumps;
void initTrainModule() {
	noLumpsOrBumps.initTraining();

	std::cout << "Beginning file processing. This *WILL take a long time.." << std::endl;
	noLumpsOrBumps.processAttachedFile();
}

bool Brain::initTraining() {
	std::cout << "Text (.txt) files only, please. ";
	std::cout << std::endl << "Paste the file path: ";
	std::string path;

	if (std::cin.peek() == '\n') {
		std::cin.ignore();
	}
	std::getline(std::cin, path);

	eyes = std::ifstream(path, std::ios::in);
	memoryWorker = std::fstream("../../../SmoothBrain/Marilyn.brain", std::ios::in | std::ios::out | std::ios::binary);
	neuronWorker = std::fstream("../../../SmoothBrain/Marilyn.neurons", std::ios::in | std::ios::out | std::ios::binary);

	if (!eyes) {
		std::cout << "Failed to open file from path." << std::endl;
		return false;
	}

	if (!memoryWorker) {
		std::cout << "Failed to access Marilyn\'s smooth brain." << std::endl;
		return false;
	}

	if (!neuronWorker) {
		std::cout << "Failed to access Marilyn\'s neuron index." << std::endl;
		return false;
	}

	return true;
}

std::streampos getFileSize(std::ifstream& file) {
	auto current = file.tellg();
	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	file.seekg(current, std::ios::beg);
	return size;
}

void Brain::processAttachedFile() {
	typedef std::istreambuf_iterator<char> buf_iter;
	std::vector<EyeCandy> window;

	const std::streampos totalBytes = getFileSize(eyes);
	std::uint64_t bytesProcessed = 0;

	const std::streampos reportEvery = totalBytes / 100; // 1% granularity
	std::streampos nextReport = reportEvery;

	for (buf_iter i(eyes), e; i != e; ++i) {
		char c = *i;
		++bytesProcessed;

		if (bytesProcessed >= nextReport) {
			double percent =
				(double(bytesProcessed) / double(totalBytes)) * 100.0;

			std::cout << "\rProcessing: "
				<< std::fixed << std::setprecision(1)
				<< percent << "%" << std::flush;

			nextReport += reportEvery;
		}

		if (c == '\n') continue;

		if (window.size() >= NEURON_DEPTH) {
			window.erase(window.begin());
		}
		window.push_back(EyeCandy{ c, false });

		for (size_t n = 0; n < window.size(); n++) {
			std::string parentKey = EMPTY_KEY;
			for (size_t j = n; j < window.size(); j++) {
				// if the parent key is empty, we should first search
				// for a root node with the same char, and assign the parentKey
				// for subsequent searches.
				// Create new if doesn't exist.
				if (parentKey == EMPTY_KEY) {
					Neuron result = getNeuronByTargetAndParentKey(EMPTY_KEY, window.at(j).ch, false, true);
					parentKey = result.key;
				}
				// ensure the current parentKey is valid 8-bytes
				else if (parentKey.size() == 8) {
					// we are searching for a child node
					// if a node hasn't been processed as a child, incrememnt it if found
					Neuron result = getNeuronByTargetAndParentKey(parentKey, window.at(j).ch, !window.at(j).proc, true);
					window.at(j).proc = true;
					parentKey = result.key;
				}
				else {
					window.clear();
				}
			}
		}
	}

	std::cout << "\rProcessing: 100.0%\n";
}
