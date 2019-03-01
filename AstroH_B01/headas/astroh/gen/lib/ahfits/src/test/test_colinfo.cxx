/** \brief test of routines in ahfits to get column information
    \author Mike Witthoeft
    \date 2012-06-14
*/

#include "ahfits/ahfits.h"

#include <iostream>


int main() {

  char* hkfile=(char*)"ae400015010.hk";
  ahfits::AhFitsFilePtr infile;
  ahfits::ahFitsOpen(hkfile, "428_DHU_HK_D", &infile);
  if (!ahfits::ahFitsReadOK(infile)) {
    std::cout << "cannot open file" << std::endl;
    return 1;
  }

  std::string name="S_TIME";
  std::cout << "name: " << name << std::endl;
  std::cout << "colnum: " << ahfits::ahFitsColumnIndex(infile,name) << std::endl;
  std::cout << "typecode: " << ahfits::ahFitsColumnType(infile,name) << std::endl;
  std::cout << "units: " << ahfits::ahFitsColumnUnits(infile,name) << std::endl;
  std::cout << "repeat: " << ahfits::ahFitsColumnRepeat(infile,name) << std::endl;
  std::cout << "width: " << ahfits::ahFitsColumnWidth(infile,name) << std::endl;
  std::cout << std::endl;

  name="TI";
  std::cout << "name: " << name << std::endl;
  std::cout << "colnum: " << ahfits::ahFitsColumnIndex(infile,name) << std::endl;
  std::cout << "typecode: " << ahfits::ahFitsColumnType(infile,name) << std::endl;
  std::cout << "units: " << ahfits::ahFitsColumnUnits(infile,name) << std::endl;
  std::cout << "repeat: " << ahfits::ahFitsColumnRepeat(infile,name) << std::endl;
  std::cout << "width: " << ahfits::ahFitsColumnWidth(infile,name) << std::endl;
  std::cout << std::endl;

  ahfits::ahFitsMove(infile,"430_DHU_HK_A");

  name="TI";
  std::cout << "name: " << name << std::endl;
  std::cout << "colnum: " << ahfits::ahFitsColumnIndex(infile,name) << std::endl;
  std::cout << "typecode: " << ahfits::ahFitsColumnType(infile,name) << std::endl;
  std::cout << "units: " << ahfits::ahFitsColumnUnits(infile,name) << std::endl;
  std::cout << "repeat: " << ahfits::ahFitsColumnRepeat(infile,name) << std::endl;
  std::cout << "width: " << ahfits::ahFitsColumnWidth(infile,name) << std::endl;
  std::cout << std::endl;



  return 0;
}
