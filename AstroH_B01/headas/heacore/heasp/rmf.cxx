// ResponseMatrix object code. Definitions in ResponseMatrix.h

#ifndef HAVE_rmf
#include "rmf.h"
#endif

#ifndef HAVE_arf
#include "arf.h"
#endif

#ifndef HAVE_grouping
#include "grouping.h"
#endif

#ifndef HAVE_SPio
#include "SPio.h"
#endif

// Class rmf

// default constructor

rmf::rmf()
{
}


// Destructor

rmf::~rmf()
{
}

// reading Matrix and Channel bounds extensions from RMF file. 

Integer rmf::read(string filename)
{
  return(this->read(filename, 1));
}

// reading Matrix and Channel bounds extensions from RMF file. This option allows multiple 
// extensions in the same file

Integer rmf::read(string filename, Integer RMFnumber)
{
  Integer Status(0);
  
  // read the MATRIX extension

  Status = this->readMatrix(filename, RMFnumber);
  if ( Status != 0 ) return(Status);

  // try to read the EBOUNDS extension. If this fails try to read with RMFnumber = 1
  // under assumption that there are multiple MATRIX extensions and only one EBOUNDS
  // extension

  Status = this->readChannelBounds(filename, RMFnumber);
  if ( Status != 0 ) {
    Status = this->readChannelBounds(filename, 1);
  }

  return(Status);
}

// reading channel bounds extension from RMF file. 

Integer rmf::readChannelBounds(string filename)
{
  return(this->readChannelBounds(filename, 1));
}

// reading channel bounds extension from PHA file. this option allows multiple extensions in the same file

Integer rmf::readChannelBounds(string filename, Integer EBDnumber)
{
  string hduName("EBOUNDS");
  string DefString;

  // Read in the Channel bounds extension number EBDnumber
  // and set up an object called ebd with the contents

  const vector<string> hduKeys;
  const vector<string> primaryKey;

  auto_ptr<FITS> pInfile(0);

  try {
    pInfile.reset(new FITS(filename,Read,hduName,false,hduKeys,primaryKey,(int)EBDnumber));
  } catch(...) {
    return(NoSuchFile);
  }

  ExtHDU& ebd = pInfile->extension(hduName);
  
  // read the standard keywords and store in the object

  DefString = "UNKNOWN";
  ChannelType = SPreadKey(ebd, "CHANTYPE", DefString);

  EBDVersion = SPreadKey(ebd, "HDUVERS", DefString);
  if ( EBDVersion == "UNKNOWN" ) {
    EBDVersion = SPreadKey(ebd, "HDUVERS2", DefString);
    if ( EBDVersion == "UNKNOWN" ) {
      EBDVersion = SPreadKey(ebd, "EBDVERSN", DefString);
      if ( EBDVersion == "UNKNOWN" ) EBDVersion = "1.1.0";
    }
  }

  EBDExtensionName = SPreadKey(ebd, "EXTNAME", DefString);
  
  Telescope = SPreadKey(ebd, "TELESCOP", DefString);
  
  Instrument = SPreadKey(ebd, "INSTRUME", DefString);

  Detector = SPreadKey(ebd, "DETNAM", DefString);

  Filter = SPreadKey(ebd, "FILTER", DefString);

  // Read the E_MIN and E_MAX columns

  SPreadCol(ebd,"E_MIN",ChannelLowEnergy);
  SPreadCol(ebd,"E_MAX",ChannelHighEnergy);
  if ( ChannelLowEnergy.size() == 0 ) return(NoEmin);
  if ( ChannelHighEnergy.size() == 0 ) return(NoEmax);

  SPreadColUnits(ebd, "E_MIN", EnergyUnits);

  return(0);
}



// reading Matrix extension from RMF file. 

Integer rmf::readMatrix(string filename)
{
  return(this->readMatrix(filename, 1));
}

// reading Matrix extension from PHA file. this option allows multiple extensions in the same file

Integer rmf::readMatrix(string filename, Integer RMFnumber)
{
  string hduName("MATRIX");
  string DefString;
  bool verbosity = FITS::verboseMode();

  // Read in the MATRIX extension number RMFnumber
  // and set up an object called rmf with the contents

  const vector<string> hduKeys;
  const vector<string> primaryKey;

  auto_ptr<FITS> pInfile(0);

  try {
    pInfile.reset(new FITS(filename,Read,hduName,false,hduKeys,primaryKey,(int)RMFnumber));
  } catch (...) {
    hduName = "SPECRESP MATRIX";
    FITS::clearErrors();
    try {
      pInfile.reset(new FITS(filename,Read,hduName,false,hduKeys,primaryKey,(int)RMFnumber));
    } catch(...) {
      return(NoSuchFile);
    }
  }

  ExtHDU& rmf = pInfile->extension(hduName);
  
  // read the standard keywords and store in the object

  DefString = "UNKNOWN";
  ChannelType = SPreadKey(rmf, "CHANTYPE", DefString);

  RMFVersion = SPreadKey(rmf, "HDUVERS", DefString);
  if ( RMFVersion == "UNKNOWN" ) {
    RMFVersion = SPreadKey(rmf, "HDUVERS2", DefString);
    if ( RMFVersion == "UNKNOWN" ) {
      RMFVersion = SPreadKey(rmf, "RMFVERSN", DefString);
      if ( RMFVersion == "UNKNOWN" ) RMFVersion = "1.1.0";
    }
  }

  RMFExtensionName = SPreadKey(rmf, "EXTNAME", DefString);
  
  Telescope = SPreadKey(rmf, "TELESCOP", DefString);
  
  Instrument = SPreadKey(rmf, "INSTRUME", DefString);

  Detector = SPreadKey(rmf, "DETNAM", DefString);

  Filter = SPreadKey(rmf, "FILTER", DefString);

  RMFType = SPreadKey(rmf, "HDUCLAS3", DefString);

  AreaScaling = SPreadKey(rmf, "EFFAREA", (Real)1.0);

  ResponseThreshold = SPreadKey(rmf, "LO_THRES", (Real)1.0);

  Integer Nrows;
  Nrows = SPreadKey(rmf, "NAXIS2", (Integer)0);

  Integer NtotGroups;
  NtotGroups = SPreadKey(rmf, "NUMGRP", (Integer)0);
  
  Integer NtotElts;
  NtotElts = SPreadKey(rmf, "NUMELT", (Integer)0);
 
  // Read the ENERG_LO and ENERG_HI columns

  SPreadCol(rmf,"ENERG_LO",LowEnergy);
  SPreadCol(rmf,"ENERG_HI",HighEnergy);
  if ( LowEnergy.size() == 0 ) return(NoEnergLo);
  if ( HighEnergy.size() == 0 ) return(NoEnergHi);

  SPreadColUnits(rmf, "ENERG_LO", EnergyUnits);

  // Get the number of groups for each energy bin

  SPreadCol(rmf,"N_GRP",NumberGroups);
  if ( NumberGroups.size() == 0 ) return(NoNgrp);

  // Set up the FirstGroup array - note that it counts from 0

  FirstGroup.resize(Nrows);
  Integer igrp = 0;
  for (size_t i=0; i<(size_t)Nrows; i++) {
    FirstGroup[i] = igrp;
    igrp += NumberGroups[i];
  }
  
  // If the NUMGRP keyword was not read then sum up this column to calculate it

  if ( NtotGroups == 0 ) {
    for (size_t i=0; i<(size_t)Nrows; i++) NtotGroups += NumberGroups[i];
  }

  // Read the first channel and number of channels for each group - grab the whole data 
  // into temporary arrays then place in object. Also set the FirstElement array.

  vector<vector<Integer> > fchan, nchan;

  SPreadVectorCol(rmf, "F_CHAN", fchan);
  SPreadVectorCol(rmf, "N_CHAN", nchan);
  if ( fchan.size() == 0 ) return(NoFchan);
  if ( nchan.size() == 0 ) return(NoNchan);


  FirstChannelGroup.resize(NtotGroups);
  NumberChannelsGroup.resize(NtotGroups);
  FirstElement.resize(NtotGroups);
  size_t ipt = 0;
  size_t ielt = 0;
  for (size_t i=0; i<(size_t)Nrows; i++) {
    for (size_t j=0; j<(size_t)fchan[i].size(); j++) {
      if ( nchan[i][j] > 0 ) {
	FirstChannelGroup[ipt] = fchan[i][j];
	NumberChannelsGroup[ipt] = nchan[i][j];
	FirstElement[ipt] = ielt;
	ielt += NumberChannelsGroup[ipt];
	ipt++;
      }
    }
  }
  
  // Check for a TLMIN for the F_CHAN column

  try {
    int ChannelIndex = rmf.column("F_CHAN").index();
    ostringstream KeyStream;
    KeyStream << "TLMIN" << ChannelIndex;
    rmf.readKey(KeyStream.str(),FirstChannel);
    FirstChannel = SPreadKey(rmf, KeyStream.str(), (Integer)1);
  } catch(Table::NoSuchColumn&) {
    FirstChannel = 1;
  } catch(HDU::NoSuchKeyword&) {
    FirstChannel = 1;
  }

  // If the NUMELT keyword was not read then sum up the N_CHAN column to calculate it

  if ( NtotElts == 0 ) {
    for (size_t i=0; i<(size_t)NtotGroups; i++) NtotElts += NumberChannelsGroup[i];
  }

  // Read the matrix column into a temporary array then set the Matrix array

  vector<vector<Real> > elements;
  SPreadVectorCol(rmf, "MATRIX", elements);
  if ( elements.size() == 0 ) return(NoMatrix);
  
  Matrix.resize(NtotElts);
  ipt = 0;
  for (size_t i=0; i<(size_t)Nrows; i++) {
    for (size_t j=0; j<elements[i].size(); j++) {
      Matrix[ipt++] = elements[i][j];
    }
  }

  SPreadColUnits(rmf, "MATRIX", RMFUnits);

  // Read the optional order information

  vector<vector<Integer> > order;
  try {
    FITS::setVerboseMode(false);
    SPreadVectorCol(rmf, "ORDER", order);
  } catch (...) {
  }

  FITS::clearErrors();
  FITS::setVerboseMode(verbosity);

  if ( order.size() > 0 ) {
    OrderGroup.resize(NtotGroups);
    ipt = 0;
    for (size_t i=0; i<(size_t)Nrows; i++) {
      for (size_t j=0; j<(size_t)NumberGroups[i]; j++) {
	OrderGroup[ipt++] = order[j][i];
      }
    }
  }

  return(0);
}

// update the FirstGroup and FirstElement arrays from NumberGroups and
// NumberChannelsGroup, respectively.

void rmf::update()
{
  // initialize
  FirstGroup.resize(NumberGroups.size());
  FirstElement.resize(NumberChannelsGroup.size());

  FirstGroup[0] = 0;
  for (size_t i=1; i<NumberGroups.size(); i++) {
    FirstGroup[i] = FirstGroup[i-1] + NumberGroups[i-1];
  }

  FirstElement[0];
  for (size_t i=1; i<NumberChannelsGroup.size(); i++) {
    FirstElement[i] = FirstElement[i-1] + NumberChannelsGroup[i-1];
  }

  return;
}

// initialize from an arf object. Copies members in common between arfs and rmfs

void rmf::initialize(const arf& a)
{
  EnergyUnits = a.EnergyUnits;
  Telescope   = a.Telescope;
  Instrument  = a.Instrument;
  Detector    = a.Detector;
  Filter      = a.Filter;
  
  LowEnergy.resize(a.LowEnergy.size());
  for (size_t i=0; i<LowEnergy.size(); i++) LowEnergy[i] = a.LowEnergy[i];
  HighEnergy.resize(a.HighEnergy.size());
  for (size_t i=0; i<HighEnergy.size(); i++) HighEnergy[i] = a.HighEnergy[i];

  return;
}

// Deep copy

rmf& rmf::operator= (const rmf& beta)
{

  FirstChannel = beta.FirstChannel;
  AreaScaling = beta.AreaScaling;
  ResponseThreshold = beta.ResponseThreshold;
  ChannelType = beta.ChannelType;
  RMFVersion = beta.RMFVersion;
  EBDVersion = beta.EBDVersion;
  Telescope = beta.Telescope;
  Instrument = beta.Instrument;
  Detector = beta.Detector;
  Filter = beta.Filter;
  RMFType = beta.RMFType;
  RMFExtensionName = beta.RMFExtensionName;
  EBDExtensionName = beta.EBDExtensionName;
  EnergyUnits = beta.EnergyUnits;
  RMFUnits = beta.RMFUnits;

  NumberGroups.resize(beta.NumberGroups.size());
  for (size_t i=0; i<NumberGroups.size(); i++) NumberGroups[i] = beta.NumberGroups[i];
  FirstGroup.resize(beta.FirstGroup.size());
  for (size_t i=0; i<FirstGroup.size(); i++) FirstGroup[i] = beta.FirstGroup[i];

  FirstChannelGroup.resize(beta.FirstChannelGroup.size());
  for (size_t i=0; i<FirstChannelGroup.size(); i++) FirstChannelGroup[i] = beta.FirstChannelGroup[i];
  NumberChannelsGroup.resize(beta.NumberChannelsGroup.size());
  for (size_t i=0; i<NumberChannelsGroup.size(); i++) NumberChannelsGroup[i] = beta.NumberChannelsGroup[i];
  FirstElement.resize(beta.FirstElement.size());
  for (size_t i=0; i<FirstElement.size(); i++) FirstElement[i] = beta.FirstElement[i];
  OrderGroup.resize(beta.OrderGroup.size());
  for (size_t i=0; i<OrderGroup.size(); i++) OrderGroup[i] = beta.OrderGroup[i];
  

  LowEnergy.resize(beta.LowEnergy.size());
  for (size_t i=0; i<LowEnergy.size(); i++) LowEnergy[i] = beta.LowEnergy[i];
  HighEnergy.resize(beta.HighEnergy.size());
  for (size_t i=0; i<HighEnergy.size(); i++) HighEnergy[i] = beta.HighEnergy[i];
  

  Matrix.resize(beta.Matrix.size());
  for (size_t i=0; i<Matrix.size(); i++) Matrix[i] = beta.Matrix[i];
  

  ChannelLowEnergy.resize(beta.ChannelLowEnergy.size());
  for (size_t i=0; i<ChannelLowEnergy.size(); i++) ChannelLowEnergy[i] = beta.ChannelLowEnergy[i];
  ChannelHighEnergy.resize(beta.ChannelHighEnergy.size());
  for (size_t i=0; i<ChannelHighEnergy.size(); i++) ChannelHighEnergy[i] = beta.ChannelHighEnergy[i];
  
  return *this;
}

// Return information

Integer rmf::NumberChannels()               // Number of spectrum channels 
{
  return ChannelLowEnergy.size();
}

Integer rmf::NumberEnergyBins()             // Number of response energies 
{
  return LowEnergy.size();
}

Integer rmf::NumberTotalGroups()            // Total number of response groups 
{
  return FirstChannelGroup.size();
}

Integer rmf::NumberTotalElements()          // Total number of response elements 
{
  return Matrix.size();
}

Real rmf::ElementValue(Integer ChannelNumber, Integer EnergyBin)    // Return the value for a particular channel
{                                           // and energy

  if ( ChannelNumber < FirstChannel || ChannelNumber >= FirstChannel+NumberChannels() || 
       EnergyBin < 0 || EnergyBin >= NumberEnergyBins() ) return 0.0;

  // loop round groups for this energy bin

  for(size_t i=FirstGroup[EnergyBin];i<(size_t)(FirstGroup[EnergyBin]+NumberGroups[EnergyBin]);i++) {

    if( ChannelNumber >= FirstChannelGroup[i] && 
        ChannelNumber < FirstChannelGroup[i]+NumberChannelsGroup[i]) {

      return(Matrix[FirstElement[i]+ChannelNumber-FirstChannelGroup[i]]);

    }

  }

  return 0.0;

}

// Return vector of matrix values for a particular energy  

vector<Real> rmf::RowValues(Integer EnergyBin)
{
  vector<Real> values(NumberChannels());

  for (size_t i=0; i<values.size(); i++) values[i] = 0.0;

  // Loop round response groups for this energy

  for (size_t i=0; i<(size_t)NumberGroups[EnergyBin]; i++) {

    size_t igroup = i + FirstGroup[EnergyBin];
    size_t ivec = FirstChannelGroup[igroup];
    size_t ielt = FirstElement[igroup];

    // loop round elements in this group - adding them to the output array

    for (size_t j=0; j<(size_t)NumberChannelsGroup[igroup]; j++) values[ivec+j] += Matrix[ielt+j];

  }

  return values;
}

// Return vector of randomly generated channel numbers for a particular energy  

vector<Integer> rmf::RandomChannels(const Real energy, const Integer NumberPhotons)
{
  vector<Integer> channel(NumberPhotons);

  // initialize the output array to -1s in the event that either the input energy is
  // outside the response range or that the response does not sum to unity and events
  // can fall off the end of the channels.

  for (size_t i=0; i<(size_t)NumberPhotons; i++) channel[i] = -1;

  size_t lower = 0;
  size_t upper = LowEnergy.size() - 1;

  // trap the case of the energy being outside the response range

  if ( energy < LowEnergy[lower] || energy > HighEnergy[upper] ) return channel;

  // find the energy bin associated with the input energy - assumes the energies are in increasing order

  size_t middle, energybin;
  while ( upper - lower > 1 ) {
    middle = (upper + lower)/2;
    if ( energy < HighEnergy[middle] ) {
      upper = middle;
    } else {
      lower = middle;
    }
  }
  if ( energy > HighEnergy[lower] ) {
    energybin = upper;
  } else {
    energybin = lower;
  }

  // generate an array of size channel each element of which is the integrated response up to and
  // including that channel

  vector<Real> sumresponse(ChannelHighEnergy.size());
  for (size_t j=0; j<ChannelHighEnergy.size(); j++) sumresponse[j] = 0.0;

  for (size_t i=0; i<(size_t)NumberGroups[energybin]; i++) {
    size_t igrp = i + FirstGroup[energybin];
    for (size_t j=0; j<(size_t)NumberChannelsGroup[igrp]; j++) {
      size_t ichan = j + FirstChannelGroup[igrp] - FirstChannel;
      sumresponse[ichan] = Matrix[j+FirstElement[igrp]];
    }
  }
  for (size_t i=1; i<sumresponse.size(); i++) sumresponse[i] += sumresponse[i-1];

  // generate random numbers between 0 and 1

  vector<Real> RandomNumber(NumberPhotons);
  for (size_t i=0; i<(size_t)NumberPhotons; i++) RandomNumber[i] = (Real) HDmtDrand();

  // loop round the photons

  for (size_t i=0; i<(size_t)NumberPhotons; i++) {

    // find the array element containing this random number. note that we do
    // not assume that the total response sums to 1 - if the random number
    // exceeds the total response then we assume that the event fell off the
    // end of the channel array and return a -1

    lower = 0;
    upper = ChannelHighEnergy.size() - 1;
    if ( RandomNumber[i] <= sumresponse[upper] ) {
      while ( upper - lower > 1 ) {
	middle = (upper + lower)/2;
	if ( RandomNumber[i] < sumresponse[middle] ) {
	  upper = middle;
	} else {
	  lower = middle;
	}
      }
      if ( RandomNumber[i] > sumresponse[lower] ) {
	channel[i] = upper;
      } else {
	channel[i] = lower;
      }

      // correct the channel number for the first channel number in use in the response matrix

      channel[i] += FirstChannel;

    }

  }

  return channel;
}

// Display information about the RMF - return as a string

string rmf::disp()
{
  ostringstream outstr;

  outstr << "Response information : " << endl;

  outstr << "   FirstChannel        = " << FirstChannel << endl;
  outstr << "   AreaScaling         = " << AreaScaling << endl;
  outstr << "   ResponseThreshold   = " << ResponseThreshold << endl;
  outstr << "   ChannelType         = " << ChannelType << endl;
  outstr << "   RMFVersion          = " << RMFVersion << endl;
  outstr << "   EBDVersion          = " << EBDVersion << endl;
  outstr << "   Telescope           = " << Telescope << endl;
  outstr << "   Instrument          = " << Instrument << endl;
  outstr << "   Detector            = " << Detector << endl;
  outstr << "   Filter              = " << Filter << endl;
  outstr << "   RMFType             = " << RMFType << endl;
  outstr << "   RMFExtensionName    = " << RMFExtensionName << endl;
  outstr << "   EBDExtensionName    = " << EBDExtensionName << endl;

  outstr << "   EnergyUnits         = " << EnergyUnits << endl;
  outstr << "   RMFUnits            = " << RMFUnits << endl;

  outstr << "   NumberChannels      = " << NumberChannels() << endl;
  outstr << "   NumberEnergyBins    = " << NumberEnergyBins() << endl;
  outstr << "   NumberTotalGroups   = " << NumberTotalGroups() << endl;
  outstr << "   NumberTotalElements = " << NumberTotalElements() << endl;


  if ( NumberGroups.size() > 1 ) outstr << "   NumberGroups array of size " << NumberGroups.size() << endl;
  if ( FirstGroup.size() > 1 ) outstr << "   FirstGroup array of size " << FirstGroup.size() << endl;

  if ( FirstChannelGroup.size() > 1 ) outstr << "   FirstChannelGroup array of size " << FirstChannelGroup.size() << endl;
  if ( NumberChannelsGroup.size() > 1 ) outstr << "   NumberChannelsGroup array of size " << NumberChannelsGroup.size() << endl;
  if ( FirstElement.size() > 1 ) outstr << "   FirstElement array of size " << FirstElement.size() << endl;

  if ( OrderGroup.size() > 1 ) outstr << "   OrderGroup array of size " << OrderGroup.size() << endl;

  if ( LowEnergy.size() > 1 ) outstr << "   LowEnergy array of size " << LowEnergy.size() << endl;
  if ( HighEnergy.size() > 1 ) outstr << "   HighEnergy array of size " << HighEnergy.size() << endl;

  if ( Matrix.size() > 1 ) outstr << "   Matrix array of size " << Matrix.size() << endl;

  if ( ChannelLowEnergy.size() > 1 ) outstr << "   ChannelLowEnergy array of size " << ChannelLowEnergy.size() << endl;
  if ( ChannelHighEnergy.size() > 1 ) outstr << "   ChannelHighEnergy array of size " << ChannelHighEnergy.size() << endl;

  // debug
  //
  //  for (size_t i=0; i<NumberGroups.size(); i++) {
  //    outstr << i << "  " << NumberGroups[i];
  //    for (size_t j=0; j<NumberGroups[i]; j++) {
  //      outstr << "  " << FirstChannelGroup[j+FirstGroup[i]] << "  " << NumberChannelsGroup[j+FirstGroup[i]];
  //    }
  //    outstr << endl;
  //  }

  return outstr.str();
}

// Clear information from the response

void rmf::clear()
{
  FirstChannel = 0;

  NumberGroups.clear();
  FirstGroup.clear();
  FirstChannelGroup.clear();
  NumberChannelsGroup.clear();
  FirstElement.clear();
  OrderGroup.clear();

  LowEnergy.clear();
  HighEnergy.clear();
  Matrix.clear();
  ChannelLowEnergy.clear();
  ChannelHighEnergy.clear();

  AreaScaling = 0.0;
  ResponseThreshold = 0.0;

  EnergyUnits = " ";
  RMFUnits = " ";

  ChannelType = " ";
  RMFVersion = " ";
  EBDVersion = " ";
  Telescope = " ";
  Instrument = " ";
  Detector = " ";
  Filter = " ";
  RMFType = " ";
  RMFExtensionName = " ";
  EBDExtensionName = " ";

  return;
}

// Check completeness and consistency of information in the rmf
  // if there is a problem then return diagnostic in string

string rmf::check()
{
  ostringstream outstr;

  // check for presence of any data

  if ( Matrix.size() == 0 ) {
    outstr << "Matrix has no data" << endl;
  }

  // check size consistency between arrays - channels

  if ( ChannelLowEnergy.size() != ChannelHighEnergy.size() ) {
    outstr << "ChannelLowEnergy size (" << ChannelLowEnergy.size() 
	 << ") differs from ChannelHighEnergy size (" << ChannelHighEnergy.size() 
	 << ")" << endl;
  }

  // energies

  if ( LowEnergy.size() != HighEnergy.size() ) {
    outstr << "LowEnergy size (" << LowEnergy.size() 
	 << ") differs from HighEnergy size (" << HighEnergy.size() 
	 << ")" << endl;
  }

  if ( LowEnergy.size() != NumberGroups.size() ) {
    outstr << "LowEnergy size (" << LowEnergy.size() 
	 << ") differs from NumberGroups size (" << NumberGroups.size() 
	 << ")" << endl;
  }

  if ( LowEnergy.size() != FirstGroup.size() ) {
    outstr << "LowEnergy size (" << LowEnergy.size() 
	 << ") differs from FirstGroup size (" << FirstGroup.size() 
	 << ")" << endl;
  }

  // groups

  if ( FirstChannelGroup.size() != NumberChannelsGroup.size() ) {
    outstr << "FirstChannelGroup size (" << FirstChannelGroup.size() 
	 << ") differs from NumberChannelsGroup size (" << NumberChannelsGroup.size() 
	 << ")" << endl;
  }

  if ( FirstChannelGroup.size() != FirstElement.size() ) {
    outstr << "FirstChannelGroup size (" << FirstChannelGroup.size() 
	 << ") differs from FirstElement size (" << FirstElement.size() 
	 << ")" << endl;
  }

  if ( FirstChannelGroup.size() != OrderGroup.size() && OrderGroup.size() > 0 ) {
    outstr << "FirstChannelGroup size (" << FirstChannelGroup.size() 
	 << ") differs from OrderGroup size (" << OrderGroup.size() 
	 << ")" << endl;
  }

  // check that arrays have sensible values

  for (size_t i=0; i<NumberGroups.size(); i++) {
    if ( NumberGroups[i] < 0 ) {
      outstr << "NumberGroups has invalid value (" << NumberGroups[i] 
	   << ") for element " << i << endl;
    }
    if ( FirstGroup[i] < 0 || FirstGroup[i] >= NumberGroups[i] ) {
      outstr << "FirstGroup has invalid value (" << FirstGroup[i] 
	   << ") for element " << i << endl;
    }
  }

  for (size_t i=0; i<FirstChannelGroup.size(); i++) {
    if ( FirstChannelGroup[i] < FirstChannel || FirstChannelGroup[i] >= (Integer)ChannelLowEnergy.size() ) {
      outstr << "FirstChannelGroup has invalid value (" << FirstChannelGroup[i] 
	   << ") for element " << i << endl;
    }
    if ( NumberChannelsGroup[i] < 0 || NumberChannelsGroup[i] >= (Integer)ChannelLowEnergy.size() ) {
      outstr << "NumberChannelsGroup has invalid value (" << NumberChannelsGroup[i] 
	   << ") for element " << i << endl;
    }
    if ( FirstElement[i] < 0 ) {
      outstr << "FirstElement has invalid value (" << FirstElement[i] 
	   << ") for element " << i << endl;
    }
  }      

  return outstr.str();
}

// Normalize the rmf so it sums to 1.0 for each energy bin

void rmf::normalize()
{

  // Loop over energies

  for (size_t ie=0; ie<(size_t)NumberEnergyBins(); ie++) {

    // sum up the response in this energy

    Real sumresp = 0.0;

    for (size_t i=0; i<(size_t)NumberGroups[ie]; i++) {
       size_t igrp = i + FirstGroup[ie];
       for (size_t j=0; j<(size_t)NumberChannelsGroup[igrp]; j++) {
         sumresp += Matrix[j+FirstElement[igrp]];
       }
    }

    // divide through by the summed response

    for (size_t i=0; i<(size_t)NumberGroups[ie]; i++) {
       size_t igrp = i + FirstGroup[ie];
       for (size_t j=0; j<(size_t)NumberChannelsGroup[igrp]; j++) {
         Matrix[j+FirstElement[igrp]] /= sumresp;
       }
    }

  }

  return;
}

void rmf::compress(const Real threshold)
{

  // Set up temporary vectors to store the output RMF

  vector<Integer> oFirstChGrp, oNumberChsGrp, oOrderGrp;
  vector<Real> oMatrix;

  // Temporary array for the response for a given energy

  vector<Real> Response(ChannelLowEnergy.size());

  // Loop over energies

  for (size_t i=0; i<(size_t)LowEnergy.size(); i++) {

    // expand response matrix into a channel array for this energy

    for (size_t j=0; j<Response.size(); j++) Response[j] = 0.0;

    for ( size_t j=FirstGroup[i]; j<(size_t)(FirstGroup[i]+NumberGroups[i]); j++ ) {
      for ( size_t k=FirstChannelGroup[j]; k<(size_t)(FirstChannelGroup[k]+NumberChannelsGroup[k]); k++ ) {
	Response[k] = Matrix[FirstElement[j]+k];
      }
    }

    // update the compressed response arrays for this energy bin

    compressLine(Response, FirstChannel, threshold, NumberGroups[i], 
		 oFirstChGrp, oNumberChsGrp, oMatrix);

    // end loop over energies

  }

  // copy new response arrays into current response

  FirstChannelGroup.resize(oFirstChGrp.size());
  NumberChannelsGroup.resize(oNumberChsGrp.size());
  Matrix.resize(oMatrix.size());

  for (size_t i=0; i<oFirstChGrp.size(); i++) {
    FirstChannelGroup[i] = oFirstChGrp[i];
  }
  for (size_t i=0; i<oNumberChsGrp.size(); i++) {
    NumberChannelsGroup[i] = oNumberChsGrp[i];
  }
  for (size_t i=0; i<oMatrix.size(); i++) {
    Matrix[i] = oMatrix[i];
  }

  // update the FirstGroup and FirstElement arrays

  this->update();

  // set RMF threshold to the new value

  ResponseThreshold = threshold;

  return;
}

// Compress in channel space

Integer rmf::rebinChannels(grouping& GroupInfo)
{

  // check for consistency between grouping and number of channels

  if ( GroupInfo.size() != NumberChannels() ) {
    return(InconsistentGrouping);
  }

  // Set up temporary vectors to store the output rmf

  vector<Integer> oFirstChGrp, oNumberChsGrp, oOrderGrp;
  vector<Real> oMatrix;

  // Temporary array for the response for a given energy

  vector<Real> Response(NumberChannels());

  // Loop over energies

  for (size_t i=0; i<(size_t)LowEnergy.size(); i++) {

    // expand response matrix into a channel array for this energy

    for (size_t j=0; j<Response.size(); j++) Response[j] = 0.0;

    for ( size_t j=FirstGroup[i]; j<(size_t)(FirstGroup[i]+NumberGroups[i]); j++ ) {
      for ( size_t k=FirstChannelGroup[j]; k<(size_t)(FirstChannelGroup[k]+NumberChannelsGroup[k]); k++ ) {
	Response[k] = Matrix[FirstElement[j]+k];
      }
    }

    // bin up the response for the energy

    vector<Real> binResponse;
    GroupBin(Response, SumMode, GroupInfo, binResponse);

    // update the compressed response arrays for this energy bin

    compressLine(binResponse, FirstChannel, ResponseThreshold, NumberGroups[i], 
		 oFirstChGrp, oNumberChsGrp, oMatrix);

    // end loop over energies

  }

  // copy new response arrays into current response

  FirstChannelGroup.resize(oFirstChGrp.size());
  NumberChannelsGroup.resize(oNumberChsGrp.size());
  Matrix.resize(oMatrix.size());

  for (size_t i=0; i<oFirstChGrp.size(); i++) {
    FirstChannelGroup[i] = oFirstChGrp[i];
  }
  for (size_t i=0; i<oNumberChsGrp.size(); i++) {
    NumberChannelsGroup[i] = oNumberChsGrp[i];
  }
  for (size_t i=0; i<oMatrix.size(); i++) {
    Matrix[i] = oMatrix[i];
  }

  // reset the FirstGroup and FirstElement arrays

  this->update();

  // now rebin the channel boundary arrays

  vector<Real> temp;
  GroupBin(ChannelLowEnergy, FirstEltMode, GroupInfo, temp);
  ChannelLowEnergy.resize(temp.size());
  ChannelLowEnergy = temp;
  GroupBin(ChannelHighEnergy, LastEltMode, GroupInfo, temp);
  ChannelHighEnergy.resize(temp.size());
  ChannelHighEnergy = temp;

  return(0);
}

// Compress in energy space

Integer rmf::rebinEnergies(grouping& GroupInfo)
{

  // check for consistency between grouping and number of channels

  if ( GroupInfo.size() != NumberEnergyBins() ) {
    return(InconsistentGrouping);
  }

  // Set up temporary vectors to store the output rmf

  vector<Integer> oNumberGroups, oFirstChGrp, oNumberChsGrp, oOrderGrp;
  vector<Real> oMatrix;

  // Temporary array for the response for a given energy

  vector<Real> Response(NumberChannels());

  // Loop over energies

  for (size_t i=0; i<(size_t)NumberEnergyBins(); i++) {

    // reset the Response array if this is the start of a new energy bin

    if ( GroupInfo.newBin(i) ) {
      for (size_t j=0; j<Response.size(); j++) Response[j] = 0.0;
    }

    // expand response matrix into a channel array for this energy and accumulate

    for ( size_t j=FirstGroup[i]; j<(size_t)(FirstGroup[i]+NumberGroups[i]); j++ ) {
      for ( size_t k=FirstChannelGroup[j]; k<(size_t)(FirstChannelGroup[k]+NumberChannelsGroup[k]); k++ ) {
	Response[k] += Matrix[FirstElement[j]+k];
      }
    }

    // if this is the last energy bin or the next one starts a new grouped bin then
    // update the compressed response arrays

    if ( i==(size_t)(NumberEnergyBins()-1) || GroupInfo.newBin(i+1) ) {

      Integer nGroups;
      compressLine(Response, FirstChannel, ResponseThreshold, nGroups, 
		   oFirstChGrp, oNumberChsGrp, oMatrix);
      oNumberGroups.push_back(nGroups);

    }

    // end loop over energy bins

  }

  // copy new response arrays into current response

  NumberGroups.resize(oNumberGroups.size());
  FirstChannelGroup.resize(oFirstChGrp.size());
  NumberChannelsGroup.resize(oNumberChsGrp.size());
  Matrix.resize(oMatrix.size());

  for (size_t i=0; i<oNumberGroups.size(); i++) {
    NumberGroups[i] = oNumberGroups[i];
  }
  for (size_t i=0; i<oFirstChGrp.size(); i++) {
    FirstChannelGroup[i] = oFirstChGrp[i];
  }
  for (size_t i=0; i<oNumberChsGrp.size(); i++) {
    NumberChannelsGroup[i] = oNumberChsGrp[i];
  }
  for (size_t i=0; i<oMatrix.size(); i++) {
    Matrix[i] = oMatrix[i];
  }

  // reset the FirstGroup and FirstElement arrays

  this->update();

  // now rebin the energy boundary arrays

  vector<Real> temp;
  GroupBin(LowEnergy, FirstEltMode, GroupInfo, temp);
  LowEnergy.resize(temp.size());
  LowEnergy = temp;
  GroupBin(HighEnergy, LastEltMode, GroupInfo, temp);
  HighEnergy.resize(temp.size());
  HighEnergy = temp;

  return(0);
}

// Write response matrix and channel bounds extensions. If file already exists then append.

Integer rmf::write(string filename)
{
  Integer Status(0);
  Status = this->writeChannelBounds(filename);
  if ( Status != 0 ) return(Status);
  Status = this->writeMatrix(filename);
  return(Status);
}

// Write response matrix and channel bounds extensions. If file already exists then append. Copy keywords and extra extensions from another file.

Integer rmf::write(string filename, string copyfilename)
{
  return(this->write(filename, copyfilename, 1));
}

// Write response matrix and channel bounds extensions. If file already exists then 
// append. Copy keywords and extra extensions from another file. Uses HDUnumber 
// instances of matrix and channel bounds extensions in both files.

Integer rmf::write(string filename, string copyfilename, Integer HDUnumber)
{
  Integer Status(0);

  // this routines write the Matrix and ChannelBounds extensions while copying
  // extra keywords in these extensions from copyfilename. They do not copy extra
  // extensions

  Status = this->writeMatrix(filename, copyfilename, HDUnumber);
  if ( Status !=0 ) return(Status);
  Status = this->writeChannelBounds(filename, copyfilename, HDUnumber);
  if ( Status !=0 ) return(Status);

  // now copy the extra extensions

  Status = SPcopyHDUs(copyfilename, filename);

  return(Status);
}

// Write response matrix extension. If file already exists appends.

Integer rmf::writeMatrix(string filename)
{
  string Blank = " ";

  vector<string> ttype;
  vector<string> tform;
  vector<string> tunit;

  // Create a new FITS file instance  

  std::auto_ptr<FITS> pFits(0);
      
  try {                
    pFits.reset( new FITS(filename,Write) );
  } catch (FITS::CantCreate) {
    return(CannotCreateMatrixExt);
  }

  // calculate the maximum number of groups and elements per row

  Integer Nrows = NumberGroups.size();
  
  Integer MaxGroups=0;
  for (size_t i=0; i<NumberGroups.size(); i++) {
    if ( NumberGroups[i] > MaxGroups ) MaxGroups = NumberGroups[i];
  }

  Integer MaxElts=0;
  for (size_t i=0; i<(size_t)Nrows; i++) {
    Integer NumElts=0;
    for (size_t j=0; j<(size_t)NumberGroups[i]; j++) {
      NumElts += NumberChannelsGroup[j+FirstGroup[i]];
    }
    if ( NumElts > MaxElts ) MaxElts = NumElts;
  }

  // set up the fchan and nchan vector arrays

  vector<vector<Integer> > fchan(Nrows), nchan(Nrows);
  for (size_t i=0; i<(size_t)Nrows; i++) {
    fchan[i].resize(MaxGroups);
    nchan[i].resize(MaxGroups);
    for (size_t j=0; j<(size_t)MaxGroups; j++) {
      fchan[i][j] = 0;
      nchan[i][j] = 0;
    }
    for (size_t j=0; j<(size_t)NumberGroups[i]; j++) {
      fchan[i][j] = FirstChannelGroup[j+FirstGroup[i]];
      nchan[i][j] = NumberChannelsGroup[j+FirstGroup[i]];
    }
  }

  // set up the array of matrix elements

  vector<vector<Real> > elements(Nrows);
  for (size_t i=0; i<(size_t)Nrows; i++) {
    Integer NumElts=0;
    for (size_t j=0; j<(size_t)NumberGroups[i]; j++) {
      NumElts += NumberChannelsGroup[j+FirstGroup[i]];
    }
    elements[i].resize(NumElts);
    for (size_t j=0; j<(size_t)NumElts; j++) {
      elements[i][j] = Matrix[j+FirstElement[FirstGroup[i]]];
    }
  }

  // if required set up the order vector array

  vector<vector<Integer> > order(Nrows);
  if ( OrderGroup.size() > 0 ) {

    for (size_t i=0; i<(size_t)Nrows; i++) {
      order[i].resize(MaxGroups);
      for (size_t j=0; j<(size_t)MaxGroups; j++) {
	order[i][j] = 0;
      }
      for (size_t j=0; j<(size_t)NumberGroups[i]; j++) {
	order[i][j] = OrderGroup[j+FirstGroup[i]];
      }
    }

  }

  // set up the column descriptors for those attributes which need to be 
  // written as columns

  ttype.push_back("ENERG_LO");
  tform.push_back("E");
  tunit.push_back(EnergyUnits);

  ttype.push_back("ENERG_HI");
  tform.push_back("E");
  tunit.push_back(EnergyUnits);

  ttype.push_back("N_GRP");
  tform.push_back("I");
  tunit.push_back(" ");

  stringstream RepeatStream;
  RepeatStream << MaxGroups;
  string Repeat(RepeatStream.str());

  ttype.push_back("F_CHAN");
  tform.push_back(Repeat+"J");
  tunit.push_back(" ");

  ttype.push_back("N_CHAN");
  tform.push_back(Repeat+"J");
  tunit.push_back(" ");
     
  if ( SPneedCol(OrderGroup) ) {
    ttype.push_back("ORDER");
    tform.push_back(Repeat+"J");
    tunit.push_back(" ");
  }

  RepeatStream.str("");
  RepeatStream << MaxElts;
  Repeat = RepeatStream.str();

  ttype.push_back("MATRIX");
  tform.push_back("PE("+Repeat+")");
  tunit.push_back(RMFUnits);

  // Create the new extension

  Table* prmf = pFits->addTable("MATRIX",Nrows,ttype,tform,tunit);
  Table& rmf = *prmf;

  // Write the standard keywords
  
  SPwriteKey(rmf, "HDUCLASS", (string)"OGIP", Blank);
    
  SPwriteKey(rmf, "HDUCLAS1", (string)"RESPONSE", Blank);

  SPwriteKey(rmf, "HDUCLAS2", (string)"RSP_MATRIX", Blank);
    
  SPwriteKey(rmf, "HDUCLAS3", RMFType, Blank);
    
  SPwriteKey(rmf, "CHANTYPE", ChannelType, "Channel type");

  SPwriteKey(rmf, "HDUVERS", RMFVersion, "OGIP version number");

  SPwriteKey(rmf, "TELESCOP", Telescope, Blank);

  SPwriteKey(rmf, "INSTRUME", Instrument, Blank);

  SPwriteKey(rmf, "DETNAM", Detector, Blank);

  SPwriteKey(rmf, "FILTER", Filter, Blank);

  SPwriteKey(rmf, "EFFAREA", AreaScaling, Blank);

  SPwriteKey(rmf, "LO_THRES", ResponseThreshold, Blank);

  SPwriteKey(rmf, "DETCHANS", NumberChannels(), "Number of channels in rmf");

  SPwriteKey(rmf, "NUMGRP", NumberTotalGroups(), "Total number of response groups");

  SPwriteKey(rmf, "NUMELT", NumberTotalElements(), "Total number of response elements");

  SPwriteKey(rmf, "TLMIN4", FirstChannel, "First channel number");

  // Write the arrays - if an array is of size 1 or all the same value 
  // it will be written as a keyword

  SPwriteCol(rmf, "ENERG_LO", LowEnergy, true);
  SPwriteCol(rmf, "ENERG_HI", HighEnergy, true);

  SPwriteCol(rmf, "N_GRP", NumberGroups, true);


  SPwriteVectorCol(rmf, "F_CHAN", fchan, true);
  SPwriteVectorCol(rmf, "N_CHAN", nchan, true);


  SPwriteVectorCol(rmf, "MATRIX", elements, true);

  if ( OrderGroup.size() > 0 ) {
    SPwriteVectorCol(rmf, "ORDER", order);
  }
  
  return(0);
}

// Write response matrix extension. If file already exists appends. Copy extra
// keywords from the matrix extension in copyfilename. Note that unlike this
// method for some other classes extra extensions are not copied.

Integer rmf::writeMatrix(string filename, string copyfilename)
{
  return(this->writeMatrix(filename, copyfilename, 1));
}

// Write response matrix extension. If file already exists appends. Copy extra
// keywords from the HDUnumber instance of matrix extension in copyfilename. Note 
// that unlike this method for some other classes extra extensions are not copied.

Integer rmf::writeMatrix(string filename, string copyfilename, Integer HDUnumber)
{
  Integer Status(0);

  Status = this->writeMatrix(filename);

  if ( Status != 0 ) return(Status);

  Status = SPcopyKeys(copyfilename, filename, "MATRIX", HDUnumber);

  return(Status);
}


// Write channel bounds extension. If file already exists appends.

Integer rmf::writeChannelBounds(string filename)
{

  string Blank = " ";

  vector<string> ttype;
  vector<string> tform;
  vector<string> tunit;

  // Create a new FITS file instance  

  std::auto_ptr<FITS> pFits(0);
      
  try {                
    pFits.reset( new FITS(filename,Write) );
  } catch (FITS::CantCreate) {
    return(CannotCreateEboundsExt);
  }

  // set up the column descriptors for those attributes which need to be 
  // written as columns

  ttype.push_back("CHANNEL");
  tform.push_back("I");
  tunit.push_back(" ");

  ttype.push_back("E_MIN");
  tform.push_back("E");
  tunit.push_back(EnergyUnits);

  ttype.push_back("E_MAX");
  tform.push_back("E");
  tunit.push_back(EnergyUnits);

  // Create the new extension

  Table* pebd = pFits->addTable("EBOUNDS",ChannelLowEnergy.size(),ttype,tform,tunit);
  Table& ebd = *pebd;

  // Write the standard keywords
  
  SPwriteKey(ebd, "HDUCLASS", (string)"OGIP", Blank);
    
  SPwriteKey(ebd, "HDUCLAS1", (string)"RESPONSE", Blank);

  SPwriteKey(ebd, "HDUCLAS2", (string)"EBOUNDS", Blank);
    
  SPwriteKey(ebd, "CHANTYPE", ChannelType, "Channel type");

  SPwriteKey(ebd, "HDUVERS", EBDVersion, "OGIP version number");

  SPwriteKey(ebd, "TELESCOP", Telescope, Blank);

  SPwriteKey(ebd, "INSTRUME", Instrument, Blank);

  SPwriteKey(ebd, "DETNAM", Detector, Blank);

  SPwriteKey(ebd, "FILTER", Filter, Blank);

  SPwriteKey(ebd, "DETCHANS", NumberChannels(), "Number of channels in ebd");

  // Generate and write the COLUMN array

  vector<Real> Channel(NumberChannels());
  for (size_t i=0; i<(size_t)NumberChannels(); i++) Channel[i] = i + FirstChannel;
  SPwriteCol(ebd, "CHANNEL", Channel);

  // Write the E_MIN and E_MAX arrays - if an array is of size 1 or all the 
  // same value it will be written as a keyword

  SPwriteCol(ebd, "E_MIN", ChannelLowEnergy);
  SPwriteCol(ebd, "E_MAX", ChannelHighEnergy);

  return(0);
}

// Write channel bounds extension. If file already exists appends. Copy extra
// keywords from the channel bounds in copyfilename. Note that unlike this
// method for some other classes extra extensions are not copied.

Integer rmf::writeChannelBounds(string filename, string copyfilename)
{
  return(this->writeChannelBounds(filename, copyfilename, 1));
}

// Write channel bounds matrix extension. If file already exists appends. Copy extra
// keywords from the HDUnumber instance of channel bounds extension in copyfilename. Note 
// that unlike this method for some other classes extra extensions are not copied.

Integer rmf::writeChannelBounds(string filename, string copyfilename, Integer HDUnumber)
{
  Integer Status(0);

  Status = this->writeChannelBounds(filename);
  if ( Status != 0 ) return(Status);

  Status = SPcopyKeys(copyfilename, filename, "EBOUNDS", HDUnumber);

  return(Status);
}

// Merge arf and rmf

rmf& rmf::operator*=(const arf& a)
{
  // check that the arf and rmf are compatible
  // if not just return the current rmf

  if ( !checkCompatibility(a) ) return *this;

  // loop round energy bins multiplying appropriate elements of the rmf by
  // the effective area for this energy from the ARF

  for ( size_t i=0; i < (size_t)LowEnergy.size(); i++ ) {

    Real effarea = a.EffArea[i];

    // loop over response groups for this energy

    for ( size_t j=FirstGroup[i]; j<(size_t)(FirstGroup[i]+NumberGroups[i]); j++ ) {

      // loop over matrix elements for this response group

      for ( size_t k=FirstElement[j]; k<(size_t)(FirstElement[j]+NumberChannelsGroup[j]); k++ ) {

	Matrix[k] *= effarea;

      }

    }

  }     

  return *this;
}

rmf& rmf::operator+=(const rmf& r)
{

 // check that the two rmfs are compatible
 // if not just return the current rmf

  if ( !checkCompatibility(r) ) return *this;

  // temporary arrays for the response for each energy

  vector<Real> Response1(ChannelLowEnergy.size());
  vector<Real> Response2(ChannelLowEnergy.size());

  // temporary arrays to store the output rmf

  vector<Integer> oFirstChGrp, oNumberChsGrp, oOrderGrp;
  vector<Real> oMatrix;

  // loop round energy bins summing appropriate elements of the rmf

  for ( size_t i=0; i < (size_t)LowEnergy.size(); i++ ) {

    // expand both response matrices into a channel array for this energy

    for (size_t j=0; j<Response1.size(); j++) {
      Response1[j] = 0.0;
      Response2[j] = 0.0;
    }

    for ( size_t j=FirstGroup[i]; j<(size_t)(FirstGroup[i]+NumberGroups[i]); j++ ) {
      for ( size_t k=FirstChannelGroup[j]; k<(size_t)(FirstChannelGroup[k]+NumberChannelsGroup[k]); k++ ) {
	Response1[k] = Matrix[FirstElement[j]+k];
      }
    }

    for ( size_t j=r.FirstGroup[i]; j<(size_t)(r.FirstGroup[i]+r.NumberGroups[i]); j++ ) {
      for ( size_t k=r.FirstChannelGroup[j]; k<(size_t)(r.FirstChannelGroup[k]+r.NumberChannelsGroup[k]); k++ ) {
	Response2[k] = r.Matrix[r.FirstElement[j]+k];
      }
    }

    // sum the two responses for this energy bin

    for (size_t j=0; j<Response1.size(); j++) {
      Response1[j] += Response2[j];
    }

    // update the compressed response arrays for this energy bin

    compressLine(Response1, FirstChannel, ResponseThreshold, NumberGroups[i], 
		 oFirstChGrp, oNumberChsGrp, oMatrix);

    // end loop over energies

  }

  // copy new response arrays into current response

  FirstChannelGroup.resize(oFirstChGrp.size());
  NumberChannelsGroup.resize(oNumberChsGrp.size());
  Matrix.resize(oMatrix.size());

  for (size_t i=0; i<oFirstChGrp.size(); i++) {
    FirstChannelGroup[i] = oFirstChGrp[i];
  }
  for (size_t i=0; i<oNumberChsGrp.size(); i++) {
    NumberChannelsGroup[i] = oNumberChsGrp[i];
  }
  for (size_t i=0; i<oMatrix.size(); i++) {
    Matrix[i] = oMatrix[i];
  }

  // reset the FirstGroup and FirstElement arrays

  this->update();

  return *this;
}


bool rmf::checkCompatibility(const rmf& r)
{

  // check that the two rmf energy binnings are compatible

  if ( LowEnergy.size() != r.LowEnergy.size() ) return false;
  for ( size_t i=0; i < (size_t)LowEnergy.size(); i++ ) {
    if ( LowEnergy[i] != r.LowEnergy[i] ) return false;
    if ( HighEnergy[i] != r.HighEnergy[i] ) return false;
  }

 // check that the two rmf channel binnings are compatible

  if ( ChannelLowEnergy.size() != r.ChannelLowEnergy.size() ) return false;
  for ( size_t i=0; i < (size_t)ChannelLowEnergy.size(); i++ ) {
    if ( ChannelLowEnergy[i] != r.ChannelLowEnergy[i] ) return false;
    if ( ChannelHighEnergy[i] != r.ChannelHighEnergy[i] ) return false;
  }

  return true;
}

bool rmf::checkCompatibility(const arf& a)
{

  if ( LowEnergy.size() != a.LowEnergy.size() ) return false;
  for ( size_t i=0; i < (size_t)LowEnergy.size(); i++ ) {
    if ( LowEnergy[i] != a.LowEnergy[i] ) return false;
    if ( HighEnergy[i] != a.HighEnergy[i] ) return false;
  }

  return true;

}


// utility routines that are not methods for the rmf object

rmf operator* (const rmf& r, const arf& a){
  rmf rr(r);
  return rr *= a;
}

rmf operator* (const arf& a, const rmf& r){
  rmf rr(r);
  return rr *= a;
}

rmf operator+ (const rmf& a, const rmf& b){
  rmf r(a);
  return r += b;
}

void compressLine(const vector<Real> Response, const Integer FirstChannel, const Real threshold, 
		  Integer& NumberGroups, vector<Integer>& oFirstChGrp, vector<Integer>& oNumberChsGrp, 
		  vector<Real>& oMatrix)
{
  NumberGroups = 0;
  bool inGroup(false);

  for ( size_t j=0; j<Response.size(); j++ ) {

    if ( Response[j] >= threshold ) {

      // if not in a response group then start a new one

      if ( !inGroup ) {

	NumberGroups++;

	oFirstChGrp.push_back(j+FirstChannel);
	oNumberChsGrp.push_back(1);
	oMatrix.push_back(Response[j]);

	inGroup = true;

	// otherwise add next response to this group

      } else {

	oNumberChsGrp[oNumberChsGrp.size()-1]++;
	oMatrix.push_back(Response[j]);

      }

      // if response below threshold then end group if it is open

    } else {

      if ( inGroup ) inGroup = false;

    }

    // end loop over channels

  }

  return;

}
