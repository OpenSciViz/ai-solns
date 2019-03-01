/** \file make_delay.cxx
 *  \brief Make mock-up of delay CALDB FITS file.
 *  \author Mike Witthoeft
 *  \date $Date: 2012/06/19 21:31:36 $
 */

#include <iostream>

#include "ahtime/ahtime.h"
#include "ahtime/ahdelay.h"
#include "ahlog/ahlog.h"

int main() {

  ahlog::setup("make_delay","NONE",3,false);

  const char* tplfile="delay.tpl";
  const char* filename="delay.fits";

  int nrows=2;

  ahtime::write_delay_template(tplfile,nrows);
  ahtime::create_delay_fits(filename,tplfile);


  std::string inst;
  std::vector<int> times;
  std::vector<double> delays;
  times.push_back(100);
//  times.push_back(150);

  inst="SXS PSP 1";
  delays.clear();
  delays.push_back(10.5);
//  delays.push_back(11.0);
  ahtime::write_delay_times(filename,inst,times,delays);

  times.clear();
  times.push_back(100);
  times.push_back(150);

  inst="SXS PSP 2";
  delays.clear();
  delays.push_back(10.5);
  delays.push_back(11.3);
  ahtime::write_delay_times(filename,inst,times,delays);

  inst="HXI1";
  delays.clear();
  delays.push_back(20.5);
  delays.push_back(21.5);
  ahtime::write_delay_times(filename,inst,times,delays);

  inst="HXI2";
  delays.clear();
  delays.push_back(20.5);
  delays.push_back(20.4);
  ahtime::write_delay_times(filename,inst,times,delays);

  inst="SGD1";
  delays.clear();
  delays.push_back(15.0);
  delays.push_back(16.0);
  ahtime::write_delay_times(filename,inst,times,delays);

  inst="SGD2";
  delays.clear();
  delays.push_back(15.0);
  delays.push_back(15.2);
  ahtime::write_delay_times(filename,inst,times,delays);

  inst="SXI";
  delays.clear();
  delays.push_back(1.0);
  delays.push_back(1.1);
  ahtime::write_delay_times(filename,inst,times,delays);


  inst="SXS PSP 1";
  int time=120;
  double out;
  out=ahtime::get_delay(inst,time);
  std::cout << "Delay: " << out << std::endl;


}

/* Revision Log
 $Log: make_delay.cxx,v $
 Revision 1.1  2012/06/19 21:31:36  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.1  2012/06/07 11:11:40  mwitthoe
 ahtime: add ahcolumndef, ahdelay, and ahtimeassign with test programs and HK FITS file for testing

*/
