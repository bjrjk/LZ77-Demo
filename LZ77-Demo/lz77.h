#ifndef LZ77_H
#define LZ77_H

#include <vector>
using std::vector;

extern int LZ77_BUFFER_SIZE;
extern int LZ77_WINDOW_SIZE;
extern const char LZ77_ESCAPE_CHAR;

void lz77_compress(const vector<unsigned char>& origin, vector<unsigned char>& compression);
void lz77_decompress(const vector<unsigned char>& compression, vector<unsigned char>& origin);

#endif