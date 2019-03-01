/** \file testmath.cxx
    \brief Test program for ahmath library.
    \author Mike Witthoeft
    \date $Date: 2012/06/21 19:03:03 $
*/

#include "ahmath/ahmath.h"
#include "ahlog/ahlog.h"

#include <iostream>


int main(int argc, char* argv[]) {

  // ----- test interpolation -------
  
  // fill arrays
  ahmath::vecdbl xarr,yarr;
  xarr.push_back(0.0);
  yarr.push_back(1.0);

  xarr.push_back(1.0);
  yarr.push_back(2.0);

  xarr.push_back(2.0);
  yarr.push_back(3.0);

  xarr.push_back(3.0);
  yarr.push_back(4.0);

  xarr.push_back(4.0);
  yarr.push_back(5.0);

  xarr.push_back(5.0);
  yarr.push_back(6.0);

  xarr.push_back(6.0);
  yarr.push_back(7.0);

  xarr.push_back(7.0);
  yarr.push_back(8.0);

  xarr.push_back(8.0);
  yarr.push_back(9.0);

  xarr.push_back(9.0);
  yarr.push_back(10.0);

  // nearest-point
  std::cout << "nearest point interpolation" << std::endl;
  double x,y;
  for (int i=0; i <10; i++) {
    x=2.+0.1*i;
    y=ahmath::interpolate_single(x,xarr,yarr,ahmath::NEAREST);
    std::cout << "x,y: " << x << ", " << y << std::endl;
  }
  std::cout << std::endl;

  // 2-point
  std::cout << "linear interpolation" << std::endl;
  for (int i=0; i <10; i++) {
    x=2.+0.1*i;
    y=ahmath::interpolate_single(x,xarr,yarr,ahmath::TWOPOINT);
    std::cout << "x,y: " << x << ", " << y << std::endl;
  }
  std::cout << std::endl;





  return 0;
}
