/** \file mcw_test.cxx
    \brief Test program for st_stream library.
    \author Mike Witthoeft
*/

#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "Stream.h"
#include "StreamFormatter.h"
#include "st_stream.h"

st_stream::StreamFormatter & formatter1() {
  static st_stream::StreamFormatter s_format("", "", 3);
  return s_format;
}


void sample_function() {

  // DEBUG message
  AH_DEBUG << "Entered function" << std::endl;

  int x=3;
  int y=4;

  // INFO message   (specifies chatter)
  AH_INFO3 << "X,Y = " << x << "," << y << std::endl;

  // sample WARNING  (uses default chatter)
  if (x > 2) {
    AH_WARN << "x is greater than 2! (x=" << x << ")" << std::endl;
  }

  // sample ERROR
  if (y > 0) {
    AH_ERR << "y is positive! " << std::endl;
  }

  int z=x+y;

  // send OUTPUT
  AH_OUT << "the answer is " << z << std::endl;

  // DEBUG message
  AH_DEBUG << "Leaving function" << std::endl;

  return;
}



void sample_class() {

  class TestClass {
    private:
      int m_x;
      int m_y;

    public:
      TestClass() { 
        m_x=0; 
        m_y=0; 
      }

      void set_values(int x,int y) {

        AH_DEBUG << "Entered function" << std::endl;

        // sample WARNING  (uses default chatter)
        if (x > 2) {
          AH_WARN << "x is greater than 2! (x=" << x << ")" << std::endl;
        }

        // sample ERROR
        if (y > 0) {
          AH_ERR << "y is positive! " << std::endl;
        }

        m_x=x;
        m_y=y;

        // INFO message   (specifies chatter)
        AH_INFO3 << "X,Y = " << x << "," << y << std::endl;

        AH_DEBUG << "Leaving function" << std::endl;
      }

      void calculate() {

        AH_DEBUG << "Entered function" << std::endl;

        int z=m_x+m_y;

        // send OUTPUT
        AH_OUT << "the answer is " << z << std::endl;

        AH_DEBUG << "Leaving function" << std::endl;
      }
  };   // end TestClass


  TestClass object;
  object.set_values(3,4);
  object.calculate();


}  // end sample_class




int main() {

  // Input: chatter and (conditionally) debug
  unsigned int max_chat = 3;
  bool debug_parallel=true;           // true: debug is not a chatter level
  bool pdebug = true;                // used when debug_parallel=true

  // inline or parallel implementation of debug
  bool debug=false;
  if (debug_parallel) {
    debug=pdebug;
  } else {
    if (max_chat >= 4) debug=true;
  }

  // Initialize standard streams with maximum chatter of max_chat.
  st_stream::InitStdStreams("mcw_test", max_chat, true, false);
  st_stream::SetDebugMode(debug);

  // define and open log file
  std::string logfile="test.log";
  std::ofstream std_os(logfile.c_str());
  if (std_os.bad()) {
    st_stream::sterr << "could not open log file " << logfile << std::endl;
    return 1;
  }

  // connect all streams to log file
  st_stream::stlog.connect(std_os);
  st_stream::sterr.connect(std_os);
  st_stream::stout.connect(std_os);


  // only connect to console if chatter > 0
  if (max_chat > 0) {
    st_stream::stlog.connect(std::clog);
    st_stream::sterr.connect(std::cerr);
    st_stream::stout.connect(std::cout);
  }



//  sample_function();
  sample_class();


  return 0;
}
