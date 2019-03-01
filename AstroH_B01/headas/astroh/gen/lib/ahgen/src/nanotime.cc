/*
 * #define _POSIX_C_SOURCE
 * #include <values.h>
 */
#include <time.h>
#include <math.h>

#if defined(__cplusplus)
namespace ahgen {
#endif
  /*
   * typedef struct { time_t tv_sec; long tv_nsec; } NanoTime; 
   * i.e. { seconds ; nanoseconds } 
   */
#if defined(__cplusplus)
  typedef struct timespec NanoTime; 
#else
  /* typedef struct timespec { time_t tv_sec; long tv_nsec; } NanoTime; */
  typedef struct { time_t tv_sec; long tv_nsec; } NanoTime; 
#endif

  NanoTime nanoTimeOf(double time) {
    const long imax = 999999999; /* nano seconds of timespec cannot be more than 999999999 */
    const double dmax = 1.0 * imax;
    const double prec = 1.0 / dmax; 
    const double huge_maxfloat = 3.40282347e+38F; /* since linux and cygwin headers differ... */
    NanoTime nanotime; nanotime.tv_sec = 0; nanotime.tv_nsec = 0;

    /*
     * allow only positive definite time? so if param is < precision -- set to 0.0 or dmax?
     */
    if( time < prec ) {
      /* time = huge_maxfloat; */ /* HUGE or  MAXFLOAT or MAXDOUBLE */
      return nanotime;
    }

    nanotime.tv_sec = (time_t)( time ); /* truncate */

    time -= nanotime.tv_sec;

    /*
     * nano seconds of timespec cannot be more than 999999999
     */
    nanotime.tv_nsec = (long)( time * dmax );

    return nanotime;
  }

#if defined(__cplusplus)
} // namespace ahgen
#endif

