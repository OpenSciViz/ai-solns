#include "stdio.h"

size_t read_tiles(const int dim, const char* filename, char** tiles) {
  FILE* fp = fopen(filename, "r+");
  size_t nr = fgets(fp, dim, tiles);
  return nr;  //sizeof(tiles)
}

size_t write_tiles(const char* filename, char** tiles) {
  FILE* fp = fopen(filename, "r+");
  size_t nw = fputs(fp, tiles);
  return nw; //sizeof(tiles);
}
