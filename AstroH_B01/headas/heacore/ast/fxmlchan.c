/*
*+
*  Name:
*     fxmlchan.c

*  Purpose:
*     Define a FORTRAN 77 interface to the AST XmlChan class.

*  Type of Module:
*     C source file.

*  Description:
*     This file defines FORTRAN 77-callable C functions which provide
*     a public FORTRAN 77 interface to the XmlChan class.

*  Routines Defined:
*     AST_XMLCHAN
*     AST_ISAXMLCHAN

*  Copyright:
*     Copyright (C) 1997-2006 Council for the Central Laboratory of the
*     Research Councils

*  Licence:
*     This program is free software; you can redistribute it and/or
*     modify it under the terms of the GNU General Public Licence as
*     published by the Free Software Foundation; either version 2 of
*     the Licence, or (at your option) any later version.
*     
*     This program is distributed in the hope that it will be
*     useful,but WITHOUT ANY WARRANTY; without even the implied
*     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*     PURPOSE. See the GNU General Public Licence for more details.
*     
*     You should have received a copy of the GNU General Public Licence
*     along with this program; if not, write to the Free Software
*     Foundation, Inc., 59 Temple Place,Suite 330, Boston, MA
*     02111-1307, USA

*  Authors:
*     DSB: David S. Berry (Starlink)

*  History:
*     21-OCT-2003 (DSB):
*        Original version.
*/

/* Define the astFORTRAN77 macro which prevents error messages from
   AST C functions from reporting the file and line number where the
   error occurred (since these would refer to this file, they would
   not be useful). */
#define astFORTRAN77

/* Header files. */
/* ============= */
#include "f77.h"                 /* FORTRAN <-> C interface macros (SUN/209) */
#include "c2f77.h"               /* F77 <-> C support functions/macros */
#include "error.h"               /* Error reporting facilities */
#include "memory.h"              /* Memory handling facilities */
#include "channel.h"             /* Provides wrapper functions */
#include "xmlchan.h"             /* C interface to the XmlChan class */

#include <stddef.h>

/* Prototypes for external functions. */
/* ================================== */
/* This is the null function defined by the FORTRAN interface in fobject.c. */
F77_SUBROUTINE(ast_null)( void );

/* FORTRAN interface functions. */
/* ============================ */
/* These functions implement the remainder of the FORTRAN interface. */
F77_INTEGER_FUNCTION(ast_xmlchan)( void (* SOURCE)(),
                                   void (* SINK)(),
                                   CHARACTER(OPTIONS),
                                   INTEGER(STATUS)
                                   TRAIL(OPTIONS) ) {
   GENPTR_CHARACTER(OPTIONS)
   F77_INTEGER_TYPE(RESULT);
   char *options;
   const char *(* source)( void );
   int i;
   void (* sink)( const char * );

   astAt( "AST_XMLCHAN", NULL, 0 );
   astWatchSTATUS(

/* Set the source and sink function pointers to NULL if a pointer to
   the null routine AST_NULL has been supplied. */
      source = (const char *(*)( void )) SOURCE;
      if ( source == (const char *(*)( void )) F77_EXTERNAL_NAME(ast_null) ) {
         source = NULL;
      }
      sink = (void (*)( const char * )) SINK;
      if ( sink == (void (*)( const char * )) F77_EXTERNAL_NAME(ast_null) ) {
         sink = NULL;
      }
      options = astString( OPTIONS, OPTIONS_length );

/* Change ',' to '\n' (see AST_SET in fobject.c for why). */
      if ( astOK ) {
         for ( i = 0; options[ i ]; i++ ) {
            if ( options[ i ] == ',' ) options[ i ] = '\n';
         }
      }
      RESULT = astP2I( astXmlChanFor( source, astSourceWrap, sink, astSinkWrap,
                                      "%s", options ) );
      astFree( options );
   )
   return RESULT;
}

F77_LOGICAL_FUNCTION(ast_isaxmlchan)( INTEGER(THIS),
                                      INTEGER(STATUS) ) {
   GENPTR_INTEGER(THIS)
   F77_LOGICAL_TYPE(RESULT);

   astAt( "AST_ISAXMLCHAN", NULL, 0 );
   astWatchSTATUS(
      RESULT = astIsAXmlChan( astI2P( *THIS ) ) ? F77_TRUE : F77_FALSE;
   )
   return RESULT;
}




