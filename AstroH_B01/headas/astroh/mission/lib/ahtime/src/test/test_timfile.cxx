/** \file test_timfile.cxx
 *  \brief Test the reading of a TIM file.
 *  \author Mike Witthoeft
 *  \date $Date: 2012/07/16 20:09:39 $
 */

#include "ahtime/ahtimfile.h"
#include "ahlog/ahlog.h"

#include <iostream>


int main() {

  ahlog::setup("test_timfile","NONE",3,false);

  std::string filename="test.tim";
  std::string extension="TIME_PACKETS";
  ahtime::tim_data timdat;
  ahtime::readTIMData(filename,extension,timdat);
  std::cout << "Size: " << timdat.size << std::endl;
  std::cout << "first: " << timdat.ti[0] << ", " << timdat.s_time[0] << std::endl;
  int i=timdat.size-1;
  std::cout << "last:  " << timdat.ti[i] << ", " << timdat.s_time[i] << std::endl;
  std::cout << "S_TIME units: " << timdat.s_time_units << std::endl;
  std::cout << "TI units: " << timdat.ti_units << std::endl;


  ahtime::clearTIMData(timdat);
  std::cout << "Size: " << timdat.size << std::endl;


}

/* Revision Log
 $Log: test_timfile.cxx,v $
 Revision 1.2  2012/07/16 20:09:39  mwitthoe
 move TI and S_TIME units from TIM file into TIM struct; remove reading of TIM file from function in ahtimeassign (reading is now done outside HDU loop)

 Revision 1.1  2012/06/22 20:13:40  mwitthoe
 in ahtime, add functions to read data from TIM file; fix compiler warning in ahdelay.cxx


*/
