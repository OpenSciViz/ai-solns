#if !defined(__data_io_h__)
#define __data_io_h__  __FILE__ // "$Name$ $Id$"
#define __data_io__(arg) const char arg##data_io_h__Id[] = __data_io_h__;
static const char __version_data_io_h[] = "0.0.0";

// allow the use in c++ code ... ala /usr/include/regex.h
#ifdef __cplusplus
extern "C" {
#endif
 
#ifdef SWIG
%module cdata_io 
%{
#define SWIG_FILE_WITH_INIT
#include "data_io.h"
%}
#endif

#include "data_io.h"
//#include "logger.h"
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

extern char** alloc_tiles(const int dim);
extern size_t print_tiles(char** tiles, const int dim);
extern size_t read_tiles(char** tiles, const char* filename, const int dim);
extern size_t write_tiles(char** tiles, const char* filename, const int dim);
extern size_t set_tiles(char** tiles, int c, const int dim);

#ifdef __cplusplus
}
#endif // support c++ compilers

#endif // data_io.h -- prevents recursive/multiple include

