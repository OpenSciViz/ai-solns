// definitions for heasp library

// all the includes

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>
#include <stdexcept>
#include <ctime>
#include <valarray>
#include <vector>

#include <CCfits/CCfits>

// use the std and CCfits namespaces

using namespace std;
using namespace CCfits;

// define Integer and Real

typedef int Integer;
typedef float Real;


#define HAVE_HEASP 1

// set up error statuses

enum{OK, NoSuchFile, NoData, NoChannelData, NoStatError, CannotCreate,
     NoEnergLo, NoEnergHi, NoSpecresp, NoEboundsExt, NoEmin, NoEmax,
     NoMatrixExt, NoNgrp, NoFchan, NoNchan, NoMatrix, CannotCreateMatrixExt,
     CannotCreateEboundsExt, InconsistentGrouping};
