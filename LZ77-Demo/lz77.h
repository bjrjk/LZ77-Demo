#ifndef LZ77_H
#define LZ77_H

#include <vector>
using std::vector;

extern int LZ77_BUFFER_SIZE;
extern int LZ77_WINDOW_SIZE;
extern const char LZ77_ESCAPE_CHAR;
/*	When the low byte of window offset is LZ77_BUFFER_SIZE in compressed data, decompress 
	algorithm will recognize compressed data as uncompressed data incorrectly. So insert 
	a split char after the escape. */
extern const char LZ77_SPLIT_CHAR;

void lz77_compress(const vector<unsigned char>& origin, vector<unsigned char>& compression);
void lz77_decompress(const vector<unsigned char>& compression, vector<unsigned char>& origin);

#endif