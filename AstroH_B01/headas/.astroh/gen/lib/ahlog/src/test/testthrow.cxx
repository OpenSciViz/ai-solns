#include <iostream>
#include <stdexcept>
#include "ahlog/ahlog.h"


void function1();
void function2();
void function3();
void function4();


int main() {

  std::string name="testthrow";
  std::string logfile="!DEFAULT";
  int chatter=2;
  bool debug=true;
  ahlog::setup(name,logfile,chatter,debug);


  try {
    function1();
  } catch (const std::runtime_error & x) {
    std::cout << "caught runtime error" << std::endl;
    AH_ERR << x.what() << std::endl;
  } catch (const std::logic_error & x) {
    std::cout << "caught logic error" << std::endl;
    AH_ERR << x.what() << std::endl;
  }

  ahlog::shutdown();

  return 0;
}

void function1() {
  std::cout << "in function1" << std::endl;
  function2();
  std::cout << "leaving function1" << std::endl;
}

void function2() {
  std::cout << "in function2" << std::endl;
  function3();
  std::cout << "leaving function2" << std::endl;
}

void function3() {
  std::cout << "in function3" << std::endl;
  function4();
  std::cout << "leaving function3" << std::endl;
}

void function4() {
  std::cout << "in function4" << std::endl;
//  AH_THROW_RUNTIME("throwing from function4");
  AH_THROW_LOGIC("throwing from function4");
  std::cout << "leaving function4" << std::endl;
}
