/* wcsconfig_f77.h.  Generated from wcsconfig_f77.h.in by configure.  */
/*============================================================================
*
*   wcsconfig_f77.h is generated from wcsconfig_f77.h.in by 'configure'.  It
*   contains C preprocessor definitions for building the WCSLIB 4.4 Fortran
*   wrappers.
*
*   Author: Mark Calabretta, Australia Telescope National Facility
*   http://www.atnf.csiro.au/~mcalabre/index.html
*   $Id: wcsconfig_f77.h.in,v 1.3 2009/09/14 20:25:13 irby Exp $
*===========================================================================*/

/* Macro for mangling Fortran subroutine names that do not contain
 * underscores.  Typically a name like "WCSINI" (case-insensitive) will become
 * something like "wcsini_" (case-sensitive).  The Fortran wrappers, which are
 * written in C, are preprocessed into names that match the latter.  The macro
 * takes two arguments which specify the name in lower and upper case. */
/* #undef F77_FUNC */
