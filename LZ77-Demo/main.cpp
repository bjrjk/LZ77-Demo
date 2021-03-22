#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include "lz77.h"
using namespace std;
using namespace std::chrono;
void writeBin(string fileName, const vector<unsigned char>& data) {
	ofstream f(fileName, ios::binary);
	if (!f) return;
	f.write(reinterpret_cast<const char*>(data.data()), data.size());
	f.close();
}
void readBin(string fileName, vector<unsigned char>& data) {
	ifstream f(fileName, ios::binary);
	char c;
	if (!f)return;
	do {
		f.read(&c, 1);
		if(!f.eof())data.push_back(c);
	} while (!f.eof());
	f.close();
}
bool cmpBin(const vector<unsigned char>& data1, const vector<unsigned char>& data2) {
	if (data1.size() != data2.size())return false;
	for (int i = 0; i < data1.size(); i++) {
		if (data1[i] != data2[i])return false;
	}
	return true;
}
int main() {
	high_resolution_clock::time_point ts, te;
	duration<double, std::ratio<1, 1000000>> duration_ms;
	vector<unsigned char> data;
	vector<unsigned char> compression;
	vector<unsigned char> origin;
	readBin("origin.dat", data);
	ts = high_resolution_clock::now();
	lz77_compress(data, compression);
	te = high_resolution_clock::now();
	duration_ms = te - ts;
	cout << "Compress Time: " << duration_ms.count() << "us\n";
	writeBin("compressed.dat",compression);
	ts = high_resolution_clock::now();
	lz77_decompress(compression, origin);
	te = high_resolution_clock::now();
	writeBin("decompressed.dat", origin);
	duration_ms = te - ts;
	cout << "Decompress Time: " << duration_ms.count() << "us\n";
	bool result = cmpBin(data, origin);
	if (result)cout << "Success!";
	else cout << "Failed!";
}