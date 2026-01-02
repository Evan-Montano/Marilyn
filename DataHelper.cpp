#include "Marilyn.h"
#include "DataHelper.h"


/// <summary>
/// Generates a seriable key to be used as an identifier.
/// </summary>
/// <returns>8-Byte std::string</returns>
std::string generate8ByteKey() {
	const char availableChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#@$.()*&^%";
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