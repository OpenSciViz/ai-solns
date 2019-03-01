/** \brief test of routines in ahfits to get header keyword values
    \author Mike Witthoeft
    \date 2012-06-13
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

if (ahFitsIsBintable(infile)) {
  std::cout << "HDU is a bintable" << std::endl;
} else {
  std::cout << "HDU is not a bintable" << std::endl;
}

if (ahFitsIsImage(infile)) {
  std::cout << "HDU is an image" << std::endl;
} else {
  std::cout << "HDU is not an image" << std::endl;
}

if (ahFitsIsASCII(infile)) {
  std::cout << "HDU is an ASCII table" << std::endl;
} else {
  std::cout << "HDU is not an ASCII table" << std::endl;
}
std::cout << std::endl;


std::string key;
std::string vstr;
double vdbl;
long long vint;
bool vbool;
bool okay;


// ----------------------------------------------------------------------------

key="TELESCOP";

std::cout << "Attempt to read a string as a string: " << std::endl;
okay=true;
try {
  vstr=ahfits::ahFitsGetKeyValStr(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vstr << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a string" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read a string as a double: " << std::endl;
okay=true;
try {
  vdbl=ahfits::ahFitsGetKeyValDbl(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vdbl << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a double" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read a string as a long long: " << std::endl;
okay=true;
try {
  vint=ahfits::ahFitsGetKeyValLLong(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vint << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a long long" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read a string as a boolean: " << std::endl;
okay=true;
try {
  vbool=ahfits::ahFitsGetKeyValBool(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vbool << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a boolean" << std::endl;
}
std::cout << std::endl;


// ----------------------------------------------------------------------------

key="NAXIS";

std::cout << "Attempt to read an integer as a string: " << std::endl;
okay=true;
try {
  vstr=ahfits::ahFitsGetKeyValStr(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vstr << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a string" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read an integer as a double: " << std::endl;
okay=true;
try {
  vdbl=ahfits::ahFitsGetKeyValDbl(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vdbl << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a double" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read an integer as a long long: " << std::endl;
okay=true;
try {
  vint=ahfits::ahFitsGetKeyValLLong(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vint << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a long long" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read an integer as a boolean: " << std::endl;
okay=true;
try {
  vbool=ahfits::ahFitsGetKeyValBool(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vbool << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a boolean" << std::endl;
}
std::cout << std::endl;

// ----------------------------------------------------------------------------

key="MEAN_EA3";

std::cout << "Attempt to read a double as a string: " << std::endl;
okay=true;
try {
  vstr=ahfits::ahFitsGetKeyValStr(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vstr << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a string" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read a double as a double: " << std::endl;
okay=true;
try {
  vdbl=ahfits::ahFitsGetKeyValDbl(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vdbl << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a double" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read a double as a long long: " << std::endl;
okay=true;
try {
  vint=ahfits::ahFitsGetKeyValLLong(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vint << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a long long" << std::endl;
}
std::cout << std::endl;

std::cout << "Attempt to read a double as a boolean: " << std::endl;
okay=true;
try {
  vbool=ahfits::ahFitsGetKeyValBool(infile,key);
} catch (...) { okay=false; }
if (okay) {
  std::cout << "SUCCESS: value = " << vbool << std::endl;
} else {
  std::cout << "FAILURE: value could not be read as a boolean" << std::endl;
}
std::cout << std::endl;

return 0;

}
