#include "math.h"
#include "stdio.h"
#include "stdlib.h"

size_t read_tiles(const int dim, const char* filename, char** tiles) {
  if( tiles == NULL ) tiles = (char**) calloc(dim, dim+1);
  FILE* fp = fopen(filename, "r+");
  size_t nr = 0, y = 0;
  char* s = '.';

  while( y < dim && s != NULL  ) {
    s = fgets(tiles[y], dim, fp);
    nr += strlen(s) ; y += 1; // C src should be readily ported to python , so no y++ 
  }
  return nr;  //sizeof(tiles)
}

size_t write_tiles(const ind dim, const char* filename, char** tiles) {
  FILE* fp = fopen(filename, "r+");
  size_t nw = 0, y = 0;

  while( y < dim ) {
    nw += fputs(tiles[y], fp);
    y += 1; // C src should be readily ported to python , so no y++ 
  }
  return nw; //sizeof(tiles);
}

int main(int argc, char** args) {
  const int dim = 14;
  tiles = (char**) calloc(dim, dim+1)
