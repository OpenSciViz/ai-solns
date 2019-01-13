#if !defined(__data_io_c__)
#define __data_io_c__  __FILE__ // "$Name$ $Id$"
static const char __version_data_io_c[] = "0.0.0";

#include "data_io.h"
__data_io__(c);

//#include "logger.h"
//__logger_h__(c);

#include "dirent.h"
#include "errno.h"
#include "fcntl.h"
#include "limits.h"
#include "math.h"
#include "linux/nfs4.h"
#include "time.h"
#include "termio.h"
#include "signal.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "strings.h"
#include "sys/uio.h"
#include "sys/stat.h"
#include "unistd.h"

//static char _errmsg[NAME_MAX];
static char _errmsg[FILENAME_MAX];

char** alloc_tiles(const int dim) {
  // alloc 1 extra char for null termination of each string
  char** tiles = (char**) calloc(dim+1, sizeof(char*));;
  int y = 0;
  while( y < dim ) {
    tiles[y++] = (char*) calloc(dim+1, sizeof(char));
  }
  return tiles;
}

size_t print_tiles(char** tiles, const int dim) {
  size_t np = 0, y = 0;

  while( y < dim ) {
    puts(tiles[y]);
    y += 1; // C src should be readily ported to python , so no y++ 
  }
  return np; //sizeof(tiles);
}

size_t read_tiles(char** tiles, const char* filename, const int dim) {
  if( ! tiles ) tiles = alloc_tiles(dim);
  FILE* fp = fopen(filename, "r");
  if( ! fp ) return -1;

  char* s = ".";
  size_t nr = 0, y = 0;
  while( y < dim && s != NULL  ) {
    s = fgets(tiles[y], dim, fp);
    nr += strlen(s) ; y += 1; // C src should be readily ported to python , so no y++ 
  }
  fclose(fp);
  return nr;  //sizeof(tiles)
}

size_t write_tiles(char** tiles, const char* filename, const int dim) {
  FILE* fp = fopen(filename, "w");
  if( ! fp ) return -1;

  size_t nw = 0, y = 0;
  while( y < dim ) {
    nw += fputs(tiles[y], fp);
    y += 1; // C src should be readily ported to python , so no y++ 
  } 
  fclose(fp);
  return nw; //sizeof(tiles);
}

size_t set_tiles(char** tiles, int c, const int dim) {
  size_t ns = 0, y = 0;

  while( y < dim ) {
    memset(tiles[y], c, (dim-1)*sizeof(char));
    y += 1; // C src shoultd be readily ported to python , so no y++ 
    ns += dim;
  }
  print_tiles(tiles, dim);
  return ns; //sizeof(tiles);
}

int main(int argc, char** args) {
  const int dim = 14;
  char** tiles = alloc_tiles(dim);
  size_t ns = set_tiles(tiles, '.', dim);
  size_t nw = write_tiles(tiles, "tiles14x14.txt", dim);
  size_t nr = read_tiles(tiles, "tiles14x14.txt", dim);
  print_tiles(tiles, dim);
}

#endif // data_io.c -- in the event someone includes this c file, etc. 
