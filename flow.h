#if !defined(__flow_h__)
#define __flow_h__  __FILE__ // "$Name$ $Id$"
#define __flow__(arg) const char arg##flow_h__Id[] = __flow_h__;
static const char __version_flow_h[] = "0.0.0";

// allow the use in c++ code ... ala /usr/include
#ifdef __cplusplus
extern "C" {
#endif
 
#ifdef SWIG
%module cflow 
%{
#define SWIG_FILE_WITH_INIT
#include "flow.h"
%}
#endif

#include "flow.h"
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

extern int flow_down(char** tiles, const int x0, const int y0, const size_t dim);
extern int flow_right(char** tiles, const int x0, const int y0, const size_t dim);
extern int flow_left(char** tiles, const int x0, const int y0, const size_t dim);
extern int flow_direction(char** tiles, const int x0, const int y0, const size_t dim);

#ifdef __cplusplus
}
#endif // support c++ compilers

#endif // flow.h -- prevents recursive/multiple include

