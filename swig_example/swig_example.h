/* File : swig_example.h */
 
#ifdef SWIG
%module swig_example
%{
#include "swig_example.h"
%}
#endif

#include <time.h>
 
extern int fact(int n);
extern int my_mod(int x, int y);
extern char *get_time();

