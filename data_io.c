#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

char** alloc_tiles(const int dim) {
  char** tiles = (char**) calloc(dim+1, sizeof(char*));;
  int y = 0;
  while( y < dim ) {
    tiles[y++] = (char*) calloc(dim+1, sizeof(char));
  }
  return tiles;
}

size_t read_tiles(const int dim, const char* filename, char** tiles) {
  if( tiles == NULL ) tiles = alloc_tiles(dim);
  FILE* fp = fopen(filename, "r+");
  size_t nr = 0, y = 0;
  char* s = ".";

  while( y < dim && s != NULL  ) {
    s = fgets(tiles[y], dim, fp);
    nr += strlen(s) ; y += 1; // C src should be readily ported to python , so no y++ 
  }
  return nr;  //sizeof(tiles)
}

size_t write_tiles(const int dim, const char* filename, char** tiles) {
  FILE* fp = fopen(filename, "r+");
  size_t nw = 0, y = 0;

  while( y < dim ) {
    nw += fputs(tiles[y], fp);
    y += 1; // C src should be readily ported to python , so no y++ 
  }
  return nw; //sizeof(tiles);
}

size_t set_tiles(const int dim, int c, char** tiles) {
  size_t ns = 0, y = 0;

  while( y < dim ) {
    memset(tiles[y], c, dim*sizeof(char));
    y += 1; // C src should be readily ported to python , so no y++ 
    ns += dim;
  }
  return ns; //sizeof(tiles);
}

size_t print_tiles(const int dim, char** tiles) {
  size_t np = 0, y = 0;

  while( y < dim ) {
    puts(tiles[y]);
    y += 1; // C src should be readily ported to python , so no y++ 
  }
  return np; //sizeof(tiles);
}

int main(int argc, char** args) {
  const int dim = 14;
  char** tiles = alloc_tiles(dim);
  size_t ns = set_tiles(dim, '.', tiles);
  size_t nw = write_tiles(dim, "tiles14x14.txt", tiles);
  size_t nr = read_tiles(dim, "tiles14x14.txt", tiles);
  print_tiles(dim, tiles);
}

