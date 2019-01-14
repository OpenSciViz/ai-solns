#if !defined(__flow_c__)
#define __flow_c__  __FILE__ // "$Name$ $Id$"
static const char __version_flow_c[] = "0.0.0";

#include "flow.h"
__flow__(c);

//#include "logger.h"
//__logger_h__(c);

int flow_down(char** tiles, const int x0, const int y0, const size_t dim) {
  // port pyhthon function to equiv C
  return -1;
}
 
int flow_right(char** tiles, const int x0, const int y0, const size_t dim) {
  // port python function to equiv C
  return -1;
}

int flow_left(char** tiles, const int x0, const int y0, const size_t dim) {
  // port python function to equiv C
  return -1;
}

int flow_direction(char** tiles, const int x0, const int y0, const size_t dim) {
  // port python function to equiv C
  return -1;
}

int main(int argc, char** args) {
  const int dim = 14;
  char** tiles = NULL; // alloc_tiles(dim);
  int f = flow_direction(tiles, 0, 0, dim);
}

#endif // flow.c -- in the event someone includes this c file, etc. 
