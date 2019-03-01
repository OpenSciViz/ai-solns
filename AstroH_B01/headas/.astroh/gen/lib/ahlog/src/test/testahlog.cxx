/** \file testahlog.cxx
    \brief Test program for ahlog library.
    \author Mike Witthoeft
    \date $Date: 2012/06/29 00:55:37 $
*/

#include <cstdlib>
#include <stdexcept>

#include "ape/ape_error.h"
#include "ape/ape_trad.h"

#include "ahlog/ahlog.h"
#include "ahlog/ahtrace.h"
#include "sys/time.h"


/// \ingroup mod_log
namespace ahlog {


  /** \addtogroup mod_ahlog
   *  @{
   */

  // ---------------------------------------------------------------------------

  void ut_sample_function() {

    AH_OUT << "This is an output message." << std::endl;

    AH_INFO(ahlog::LOW) << "This is LOW priority information." << std::endl;
    AH_INFO(ahlog::HIGH) << "This is HIGH priority information." << std::endl;

    AH_WARN(ahlog::LOW) << "This is a LOW priority warning." << std::endl;
    AH_WARN(ahlog::HIGH) << "This is a HIGH priority warning." << std::endl;

    AH_DEBUG << "This is a debug message." << std::endl;

    AH_THROW_RUNTIME("This is a thrown error");
//    AH_ERR << "This is an error message." << std::endl;

    return;
  }

  // ---------------------------------------------------------------------------

  void ut_sample_class() {

    class TestClass {
      private:
        int m_x;
        int m_y;

      public:

        void sample_method() {
          AH_OUT << "This is an output message." << std::endl;

          AH_INFO(ahlog::LOW) << "This is LOW priority information." << std::endl;
          AH_INFO(ahlog::HIGH) << "This is HIGH priority information." << std::endl;

          AH_WARN(ahlog::LOW) << "This is a LOW priority warning." << std::endl;
          AH_WARN(ahlog::HIGH) << "This is a HIGH priority warning." << std::endl;

          AH_DEBUG << "This is a debug message." << std::endl;

          AH_THROW_RUNTIME("This is a thrown error");
//          AH_ERR << "This is an error message." << std::endl;

          return;
        }

    };   // end TestClass

    TestClass object;
    object.sample_method();

  }  // end ut_sample_class

  // ---------------------------------------------------------------------------

  void ut_trace4() {
    AH_INFO(ahlog::HIGH) << "in traceback function4" << std::endl;
    AH_THROW_RUNTIME("throwing from function4");
    AH_INFO(ahlog::LOW) << "leaving function4 (should not see this)" 
                        << std::endl;
  }

  // ---------------------------------------------------------------------------

  void ut_trace3() {
    AH_INFO(ahlog::HIGH) << "in traceback function3" << std::endl;
    ut_trace4();
    AH_INFO(ahlog::LOW) << "leaving function3" << std::endl;
  }

  // ---------------------------------------------------------------------------

  void ut_trace2() {
    AH_INFO(ahlog::HIGH) << "in traceback function2" << std::endl;
    ut_trace3();
    AH_INFO(ahlog::LOW) << "leaving function2" << std::endl;
  }

  // ---------------------------------------------------------------------------

  void ut_trace1() {
    AH_INFO(ahlog::HIGH) << "in traceback function1" << std::endl;
    ut_trace2();
    AH_INFO(ahlog::LOW) << "leaving function1" << std::endl;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void ut_main(char* runtype, char* logfile, int chatter, bool debug) {

    // start logger
    ahlog::setup("testahlog",logfile,chatter,debug);

    // do work
    try {
      if (!strcmp(runtype,"function")) {
        ut_sample_function();
      } else if (!strcmp(runtype,"class")) {
        ut_sample_class();
      } else if (!strcmp(runtype,"stacktrace")) {
        ut_trace1();
      } else {
        AH_THROW_RUNTIME("invalid value for runtype");
      }
    } catch (const std::exception & x) {
      AH_ERR << x.what() << std::endl;
    }

    // close logger
    ahlog::shutdown();
  }

  /** @} */

}   // end namespace

// *****************************************************************************



int main(int argc, char* argv[]) {

  int status = ape_trad_init(argc, argv);
  if (eOK != status) {
    std::cout << "could not access parameter file" << std::endl;
    return 1;
  }

  // get parameter: chatter
  int chatter;
  status = ape_trad_query_int("chatter", &chatter);
  if (eOK != status) chatter=2;

  // get parameter: debug
  char t_debug=0;
  bool debug=false;
  status = ape_trad_query_bool("debug", &t_debug);
  if (eOK != status) {
    // Ignore error -- debug has default value.
  } else {
    debug = 0 != t_debug ? true : false;
  }

  // get parameter: logfile
  char* logfile;
  status = ape_trad_query_string("logfile",&logfile);

  // get parameter: runtype
  char* runtype;
  status = ape_trad_query_string("runtype", &runtype);

  ahlog::ut_main(runtype,logfile,chatter,debug);


  return 0;
}
