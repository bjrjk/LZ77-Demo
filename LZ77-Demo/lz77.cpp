#include <algorithm>
#include <vector>
#include "lz77.h"
using std::vector;
using std::min;
using std::max;
int LZ77_BUFFER_SIZE = 128;
int LZ77_WINDOW_SIZE = 4096;
const char LZ77_ESCAPE_CHAR = '\\';
const char LZ77_SPLIT_CHAR = '\0';
static inline int putEscapedChar(vector<unsigned char>& arr, unsigned char c, bool ctrl = false) {
	if (ctrl) {
		arr.push_back(LZ77_ESCAPE_CHAR);
		return 1;
	}
	else if (c == LZ77_ESCAPE_CHAR) {
		arr.push_back(LZ77_ESCAPE_CHAR);
		arr.push_back(LZ77_ESCAPE_CHAR);
		return 2;
	}
	else {
		arr.push_back(c);
		return 1;
	}
}
static inline unsigned char readEscapedChar(const vector<unsigned char>& arr, size_t& i,
	bool& ctrl, bool modifyIndex = true) {
	if (arr[i] == LZ77_ESCAPE_CHAR && (i + 1 >= arr.size() || arr[i + 1] != LZ77_ESCAPE_CHAR)) {
		if (modifyIndex)i++;
		ctrl = true;
		return LZ77_ESCAPE_CHAR;
	}
	else if (arr[i] == LZ77_ESCAPE_CHAR && (i + 1 < arr.size() && arr[i + 1] == LZ77_ESCAPE_CHAR)) {
		if (modifyIndex)i += 2;
		ctrl = false;
		return LZ77_ESCAPE_CHAR;
	}
	else {
		char c = arr[i];
		if (modifyIndex)i++;
		ctrl = false;
		return c;
	}
}
static inline void getLongestMatchingPhrase(const vector<unsigned char>& data,
	const int windowStart, const int bufferStart, size_t& phraseStart, size_t& phraseLength,
	bool& firstCExists, unsigned char& firstC) {
	// Can be optimized using KMP, current is brute-force
	phraseStart = bufferStart;
	phraseLength = 0;
	firstCExists = false;
	for (int winIter = max(windowStart, 0);
		winIter < min(windowStart + LZ77_WINDOW_SIZE, (int)data.size());
		winIter++) {
		for (int bufIter = bufferStart; bufIter < min(bufferStart + LZ77_BUFFER_SIZE, (int)data.size()); bufIter++) {
			size_t tmpPhrasePos = bufIter - bufferStart;
			if (winIter + (int)tmpPhrasePos >= min(windowStart + LZ77_WINDOW_SIZE, (int)data.size()))break;
			if (data[winIter + tmpPhrasePos] != data[bufIter])break;
			size_t tmpPhraseLen = tmpPhrasePos + 1;
			if (tmpPhraseLen > phraseLength) {
				phraseStart = winIter - windowStart;
				phraseLength = tmpPhraseLen;
				firstCExists = bufIter + 1 < (int)data.size();
				if (firstCExists) firstC = data[bufIter + 1];
			}
		}
	}
}
static inline vector<unsigned char> sizet2bytes(size_t num) { //Little Endian
	vector<unsigned char> result;
	unsigned char* ptr = reinterpret_cast<unsigned char*>(&num);
	for (size_t i = 0; i < sizeof(size_t); i++) {
		result.push_back(*ptr);
		ptr++;
	}
	return result;
}
static inline size_t bytes2sizet(const vector<unsigned char>& data, size_t startIndex) {
	size_t result;
	unsigned char* ptr = reinterpret_cast<unsigned char*>(&result);
	for (size_t i = 0; i < sizeof(size_t); i++) {
		*ptr = data[startIndex + i];
		ptr++;
	}
	return result;
}
static void writeCtrlData(vector<unsigned char>& data,
	size_t phraseStart, size_t phraseLength, unsigned char firstC, bool firstCExists = true) {
	vector<unsigned char> phraseStartBytes = sizet2bytes(phraseStart);
	vector<unsigned char> phraseLengthBytes = sizet2bytes(phraseLength);
	putEscapedChar(data, LZ77_ESCAPE_CHAR, true);
	putEscapedChar(data, LZ77_SPLIT_CHAR);
	data.insert(data.end(), phraseStartBytes.begin(), phraseStartBytes.end());
	data.insert(data.end(), phraseLengthBytes.begin(), phraseLengthBytes.end());
	if (firstCExists)putEscapedChar(data, firstC);
	else {
		putEscapedChar(data, LZ77_ESCAPE_CHAR + 1);
		putEscapedChar(data, LZ77_ESCAPE_CHAR + 1);
	}
	putEscapedChar(data, LZ77_SPLIT_CHAR);
	putEscapedChar(data, LZ77_ESCAPE_CHAR, true);
}
static bool readCtrlData(const vector<unsigned char>& data, size_t& i,
	size_t& phraseStart, size_t& phraseLength, unsigned char& firstC, bool& firstCExists) {
	bool ctrl;
	readEscapedChar(data, i, ctrl, false);
	if (!ctrl) return false;
	readEscapedChar(data, i, ctrl);
	readEscapedChar(data, i, ctrl);
	phraseStart = bytes2sizet(data, i);
	i += sizeof(size_t);
	phraseLength = bytes2sizet(data, i);
	i += sizeof(size_t);
	firstCExists = !(i + 1 < data.size() && data[i] == LZ77_ESCAPE_CHAR + 1 && data[i + 1] == LZ77_ESCAPE_CHAR + 1);
	if (firstCExists) {
		firstC = readEscapedChar(data, i, ctrl);
	}
	else i += 2;
	i += 2;
	return true;
}
void lz77_compress(const vector<unsigned char>& origin, vector<unsigned char>& compression) {
	int windowStart = -LZ77_WINDOW_SIZE, bufferStart = 0;
	size_t phraseStart, phraseLength;
	while (windowStart < (int)origin.size() && bufferStart < (int)origin.size()) {
		bool firstCExists;
		unsigned char firstC;
		getLongestMatchingPhrase(origin, windowStart, bufferStart,
			phraseStart, phraseLength, firstCExists, firstC);
		if (!firstCExists)firstC = 0;
		if (phraseLength == 0) putEscapedChar(compression, origin[phraseStart]);
		else writeCtrlData(compression, phraseStart, phraseLength, firstC, firstCExists);
		windowStart += phraseLength + 1;
		bufferStart += phraseLength + 1;
	}
}
void lz77_decompress(const vector<unsigned char>& compression, vector<unsigned char>& origin) {
	int windowStart = -LZ77_WINDOW_SIZE;
	size_t compressIndex = 0;
	size_t phraseStart, phraseLength;
	unsigned char c;
	bool ctrl, firstCExists;
	while (compressIndex < compression.size()) {
		readEscapedChar(compression, compressIndex, ctrl, false);
		if (ctrl) {
			readCtrlData(compression, compressIndex, phraseStart, phraseLength, c, firstCExists);
			origin.insert(origin.end(),
				origin.begin() + (windowStart + phraseStart),
				origin.begin() + (windowStart + phraseStart + phraseLength));
			windowStart += phraseLength + firstCExists;
		}
		else {
			firstCExists = true;
			c = readEscapedChar(compression, compressIndex, ctrl);
			windowStart++;
		}
		if (firstCExists)origin.push_back(c);
	}
}