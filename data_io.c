#include "stdio.h"

size_t read_tiles(char** tiles, char* filename) {
  File *fp = fopen(filename, "r+");
  size_t nr = fgets(fp, tiles);
  return nr;  //sizeof(tiles)
}

size_t write_tiles(chaar** tiles, char* filename) {
  File *fp = fopen(filename, "r+");
  isize_t nw = fputs(fp, tiles);
  return sizeof(tiles);
}
