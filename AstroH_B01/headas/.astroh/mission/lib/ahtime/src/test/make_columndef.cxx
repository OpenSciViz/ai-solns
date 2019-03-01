/** \file make_columndef.cxx
 *  \brief Make mock-up of columndef CALDB FITS file.
 *  \author Mike Witthoeft
 *  \date $Date: 2012/06/19 21:31:36 $
 */

#include <iostream>

#include "ahtime/ahtime.h"
#include "ahtime/ahcolumndef.h"
#include "ahlog/ahlog.h"

int main() {

const char* tplfile="columndef.tpl";
const char* filename="columndef.fits";

ahtime::write_coldef_template(tplfile);
ahtime::write_coldef_fits(filename,tplfile);

ahtime::coldef_names tmp;
ahtime::get_coldef_columns(filename,"SXS-Antico",tmp);

std::cout << "COLUM1: " << tmp.colum1 << std::endl;
std::cout << "COLUM2: " << tmp.colum2 << std::endl;
std::cout << "LTIME1EVT: " << tmp.ltime1evt << std::endl;
std::cout << "LTIME2EVT: " << tmp.ltime2evt << std::endl;
std::cout << "LTIME1HK: " << tmp.ltime1hk << std::endl;
std::cout << "LTIME2HK: " << tmp.ltime2hk << std::endl;

}

/* Revision Log
 $Log: make_columndef.cxx,v $
 Revision 1.1  2012/06/19 21:31:36  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.1  2012/06/07 11:11:40  mwitthoe
 ahtime: add ahcolumndef, ahdelay, and ahtimeassign with test programs and HK FITS file for testing

*/
