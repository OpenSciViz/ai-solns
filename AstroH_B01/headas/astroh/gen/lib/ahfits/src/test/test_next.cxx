/** \brief test of routines in ahfits to get header keyword values
    \author Mike Witthoeft
    \date 2012-06-13
*/

#include "ahlog/ahlog.h"
#include "ahfits/ahfits.h"

#include <iostream>


int main(int argc, char ** argv) {

ahlog::setup(argv[0], std::string(argv[0])+".log", 3, false);

char* hkfile=(char*)"ae400015010.hk";
ahfits::AhFitsFilePtr infile;
ahfits::ahFitsOpen(hkfile, "", &infile);
if (!ahfits::ahFitsReadOK(infile)) {
  std::cout << "cannot open file" << std::endl;
  return 1;
}

std::string key="EXTNAME";
std::string vstr;
vstr=ahfits::ahFitsGetKeyValStr(infile,key);
std::cout << "first: " << vstr << std::endl;
do {
  vstr=ahfits::ahFitsGetKeyValStr(infile,key);
  std::cout << vstr << std::endl;
} while (ahfits::ahFitsNextHDU(infile));





return 0;

}
