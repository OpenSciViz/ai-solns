// Spectrum object code. Definitions in Spectrum.h

#ifndef HAVE_pha
#include "pha.h"
#endif

#ifndef HAVE_grouping
#include "grouping.h"
#endif

#ifndef HAVE_SPio
#include "SPio.h"
#endif


// Class pha

// default constructor

pha::pha()
{
}


// Destructor

pha::~pha()
{
}


// reading from pha file. 

Integer pha::read(string filename)
{
  return(this->read(filename, 1, 1));
}

// reading from PHA file. this option allows multiple extensions in the same file

Integer pha::read(string filename, Integer PHAnumber)
{
  return(this->read(filename, PHAnumber, 1));
}

// reading from PHA file. For a type I file SpectrumNumber should be set to 1

Integer pha::read(string filename, Integer PHAnumber, Integer SpectrumNumber)
{

  const string hduName("SPECTRUM");
  string DefString;
  bool verbosity = FITS::verboseMode();


  // Read in the SPECTRUM extension number PHAnumber
  // and set up an object called spectrum with the contents

  const vector<string> hduKeys;
  const vector<string> primaryKey;

  auto_ptr<FITS> pInfile(0);

  try {
    pInfile.reset(new FITS(filename,Read,hduName,false,hduKeys,primaryKey,(int)PHAnumber));
  } catch(...) {
    return(NoSuchFile);
  }

  ExtHDU& spectrum = pInfile->extension(hduName);
  
  // read the standard keywords and store in the object

  DefString = "UNKNOWN";
  ChannelType = SPreadKey(spectrum, "CHANTYPE", SpectrumNumber, DefString);

  PHAVersion = SPreadKey(spectrum, "HDUVERS", SpectrumNumber, DefString);
  if ( PHAVersion == "UNKNOWN" ) {
    PHAVersion = SPreadKey(spectrum, "HDUVERS1", SpectrumNumber, DefString);
  }

  Telescope = SPreadKey(spectrum, "TELESCOP", SpectrumNumber, DefString);
  
  Instrument = SPreadKey(spectrum, "INSTRUME", SpectrumNumber, DefString);

  Detector = SPreadKey(spectrum, "DETNAM", SpectrumNumber, DefString);

  Filter = SPreadKey(spectrum, "FILTER", SpectrumNumber, DefString);

  Datamode = SPreadKey(spectrum, "DATAMODE", SpectrumNumber, DefString);

  DefString = "TOTAL";
  Spectrumtype = SPreadKey(spectrum, "HDUCLAS2", SpectrumNumber, DefString);

  DefString = "NONE";
  ResponseFile = SPreadKey(spectrum, "RESPFILE", SpectrumNumber, DefString);
    
  AncillaryFile = SPreadKey(spectrum, "ANCRFILE", SpectrumNumber, DefString);

  BackgroundFile = SPreadKey(spectrum, "BACKFILE", SpectrumNumber, DefString);

  CorrectionFile = SPreadKey(spectrum, "CORRFILE", SpectrumNumber, DefString);

  CorrectionScaling = SPreadKey(spectrum, "CORRSCAL", SpectrumNumber, (Real)1.0);

  Exposure = SPreadKey(spectrum, "EXPOSURE", SpectrumNumber, (Real)0.0);

  Poisserr = SPreadKey(spectrum, "POISSERR", SpectrumNumber, false);

  // Read the XFLT keywords

  bool done = false;
  int i = 0;
  while ( (i++)<=9998 && !done ) {
    ostringstream KeyStream;
    KeyStream << "XFLT" << setfill('0') << setw(4) << i;
    string KeyName(KeyStream.str());
    string KeyValue;
    DefString = "NONE";
    KeyValue = SPreadKey(spectrum, KeyName, SpectrumNumber, DefString);
    if (KeyValue != "NONE") {
      XSPECFilter.push_back(KeyValue);
    } else {
      done = true;
    }
  }

  // Check for TLMIN set for the CHANNEL column

  FITS::setVerboseMode(false);
  try {
    int ChannelIndex = spectrum.column("CHANNEL").index();
    ostringstream KeyStream;
    KeyStream << "TLMIN" << ChannelIndex;
    spectrum.readKey(KeyStream.str(),FirstChannel);
    FirstChannel = SPreadKey(spectrum, KeyStream.str(), SpectrumNumber, (Integer)1);
  } catch(Table::NoSuchColumn&) {
    FirstChannel = 1;
  } catch(HDU::NoSuchKeyword&) {
    FirstChannel = 1;
  }
  FITS::clearErrors();
  FITS::setVerboseMode(verbosity);

  // Get the number of detector channels which may differ from the actual number
  // of rows

  DetChans = SPreadKey(spectrum,"DETCHANS",SpectrumNumber,(Integer)0);

  // Read the CHANNEL column

  SPreadCol(spectrum,"CHANNEL",SpectrumNumber,Channel);

  // Read the data and set the datatype column appropriately

  SPreadCol(spectrum,"COUNTS",SpectrumNumber,Pha);
  if ( Pha.size() != 0 ) {
    Datatype = "COUNT";
  } else {
    SPreadCol(spectrum,"RATE",SpectrumNumber,Pha);
    Datatype = "RATE";
  }

  if ( Pha.size() == 0 ) return(NoData);

  // If no CHANNEL column was read and the Channel array can be constructed do so

  if ( Channel.size() == 0 ) {
    if ( Pha.size() == (size_t)DetChans ) {
      Channel.resize(Pha.size());
      for (size_t i=0; i<Pha.size(); i++) {
	Channel[i] = FirstChannel + i;
      }
    } else {
      return(NoChannelData);
    }
  }

  // Read the statistical error if poisserr is false

  if (!Poisserr) {
    SPreadCol(spectrum,"STAT_ERR",SpectrumNumber,StatError);
    if ( StatError.size() == 0 ) return(NoStatError);
  }
  
  // Read the systematic error if it is there otherwise set the array to 0

  FITS::setVerboseMode(false);
  try {
    SPreadCol(spectrum,"SYS_ERR",SpectrumNumber,SysError);
  } catch (...) {
  }
  FITS::clearErrors();
  FITS::setVerboseMode(verbosity);

  if ( SysError.size() == 0 ) {
    SysError.resize(1);
    SysError[0] = 0.0;
  }

  // Read the QUALITY

  SPreadCol(spectrum,"QUALITY",SpectrumNumber,Quality);

  // Read the GROUPING
  
  SPreadCol(spectrum,"GROUPING",SpectrumNumber,Group);

  // Read the AREASCAL

  SPreadCol(spectrum,"AREASCAL",SpectrumNumber,AreaScaling);

  // Read the BACKSCAL

  SPreadCol(spectrum,"BACKSCAL",SpectrumNumber,BackScaling);

  return(0);

}

// Deep copy

pha& pha::operator=(const pha& beta)
{
  // Copy the scalars

  FirstChannel = beta.FirstChannel;
  Exposure = beta.Exposure;
  CorrectionScaling = beta.CorrectionScaling;
  DetChans = beta.DetChans;
  Poisserr = beta.Poisserr;
  Datatype = beta.Datatype;
  Spectrumtype = beta.Spectrumtype;
  ResponseFile = beta.ResponseFile;
  AncillaryFile = beta.AncillaryFile;
  BackgroundFile = beta.BackgroundFile;
  CorrectionFile = beta.CorrectionFile;
  ChannelType = beta.ChannelType;
  PHAVersion = beta.PHAVersion;
  Telescope = beta.Telescope;  
  Instrument = beta.Instrument;
  Detector = beta.Detector;  
  Filter = beta.Filter;
  Datamode = beta.Datamode;

  // copy the XFLT keywords
  XSPECFilter.resize(beta.XSPECFilter.size());
  for (size_t i=0; i<beta.XSPECFilter.size(); i++) XSPECFilter[i] = beta.XSPECFilter[i];

  // now copy the arrays

  Channel.resize(beta.Channel.size());
  for (size_t i=0; i<beta.Channel.size(); i++) Channel[i] = beta.Channel[i];
  Pha.resize(beta.Pha.size());
  for (size_t i=0; i<beta.Pha.size(); i++) Pha[i] = beta.Pha[i];
  StatError.resize(beta.StatError.size());
  for (size_t i=0; i<beta.StatError.size(); i++) StatError[i] = beta.StatError[i];
  SysError.resize(beta.SysError.size());
  for (size_t i=0; i<beta.SysError.size(); i++) SysError[i] = beta.SysError[i];

  Quality.resize(beta.Quality.size());
  for (size_t i=0; i<beta.Quality.size(); i++) Quality[i] = beta.Quality[i];
  Group.resize(beta.Group.size());
  for (size_t i=0; i<beta.Group.size(); i++) Group[i] = beta.Group[i];

  AreaScaling.resize(beta.AreaScaling.size());
  for (size_t i=0; i<beta.AreaScaling.size(); i++) AreaScaling[i] = beta.AreaScaling[i];
  BackScaling.resize(beta.BackScaling.size());
  for (size_t i=0; i<beta.BackScaling.size(); i++) BackScaling[i] = beta.BackScaling[i];

  return *this;

}

// Return the number of channels in the arrays

Integer pha::NumberChannels()
{
  return Pha.size();
}

// Display information about the spectrum

string pha::disp()
{
  ostringstream outstr;

  outstr << "Spectrum information : " << endl;
  outstr << "   Number of channels   = " << NumberChannels()<< endl;
  outstr << "   Detchans             = " << DetChans << endl;
  outstr << "   Exposure             = " << Exposure<< endl;
  outstr << "   CorrectionScaling    = " << CorrectionScaling<< endl;
  if ( Poisserr ) {
    outstr << "   Poisserr             = " << "true"<< endl;
  } else {
    outstr << "   Poisserr             = " << "false"<< endl;
  }
  outstr << "   Datatype             = " << Datatype<< endl;
  outstr << "   Spectrumtype         = " << Spectrumtype<< endl;
  outstr << "   ResponseFile         = " << ResponseFile<< endl;
  outstr << "   AncillaryFile        = " << AncillaryFile<< endl;
  outstr << "   BackgroundFile       = " << BackgroundFile<< endl;
  outstr << "   CorrectionFile       = " << CorrectionFile<< endl;
  outstr << "   ChannelType          = " << ChannelType<< endl;
  outstr << "   PHAVersion           = " << PHAVersion<< endl;
  outstr << "   Telescope            = " << Telescope<< endl;
  outstr << "   Instrument           = " << Instrument<< endl;  
  outstr << "   Detector             = " << Detector<< endl;  
  outstr << "   Filter               = " << Filter<< endl;  
  outstr << "   Datamode             = " << Datamode<< endl;
  if ( AreaScaling.size() == 1 ) {
    outstr << "   AreaScaling          = " << AreaScaling[0]<< endl;
  }
  if ( BackScaling.size() == 1 ) {
    outstr << "   BackScaling          = " << BackScaling[0]<< endl;
  }
  if ( Quality.size() == 1 ) {
    outstr << "   Quality              = " << Quality[0]<< endl;
  }
  if ( Group.size() == 1 ) {
    outstr << "   Grouping             = " << Group[0]<< endl;
  }
  outstr << endl;

  if ( XSPECFilter.size() > 0 ) {
    outstr << "   XSPEC filter keywords : " << endl;
    for (size_t i=0; i<XSPECFilter.size(); i++) {
      outstr << "          " << XSPECFilter[i] << endl;
    }
  }

  if ( Channel.size() > 1 ) outstr << "  Channel array of size " << Channel.size() << endl;
  if ( Pha.size() > 1 ) outstr << "  Pha array of size " << Pha.size() << endl;
  if ( StatError.size() > 1 ) outstr << "  StatError array of size " << StatError.size() << endl;
  if ( SysError.size() > 1  ) outstr << "  SysError array of size " << SysError.size() << endl;
  if ( Quality.size() > 1   ) outstr << "  Quality array of size " << Quality.size() << endl;
  if ( Group.size() > 1  ) outstr << "  Grouping array of size " << Group.size() << endl;
  if ( AreaScaling.size() > 1 ) outstr << "  AreaScaling array of size " << AreaScaling.size() << endl;
  if ( BackScaling.size() > 1 ) outstr << "  BackScaling array of size " << BackScaling.size() << endl;
  outstr << endl;

  // for debugging
  //  for (size_t i=0; i<Pha.size(); i++) {
  //    outstr << i << "  ";
  //    if ( Channel.size() > 1 ) outstr << "  " << Channel[i];
  //    outstr << "  " << Pha[i];
  //    if ( StatError.size() > 1 ) outstr << "  " << StatError[i];
  //    if ( SysError.size() > 1  ) outstr << "  " << SysError[i];
  //    if ( Quality.size() > 1   ) outstr << "  " << Quality[i];
  //    if ( Group.size() > 1  ) outstr << "  " << Group[i];
  //    if ( AreaScaling.size() > 1 ) outstr << "  " << AreaScaling[i];
  //    if ( BackScaling.size() > 1 ) outstr << "  " << BackScaling[i];
  //    outstr << endl;
  //  }
  // end of debugging

  return outstr.str(); 

}

// Clear the information in the spectrum

void pha::clear()
{
  FirstChannel = 0;

  Pha.clear();
  StatError.clear();
  SysError.clear();
  Channel.clear();
  Quality.clear();
  Group.clear();
  AreaScaling.clear();
  BackScaling.clear();

  Exposure = 0.0;
  CorrectionScaling = 0.0;

  DetChans = 0;
  Poisserr = false;
  Datatype = " ";
  PHAVersion = " ";
  Spectrumtype = " ";
  ResponseFile = " ";
  AncillaryFile = " ";
  BackgroundFile = " ";
  CorrectionFile = " ";
  ChannelType = " ";
  Telescope = " ";
  Instrument = " ";
  Detector = " ";
  Filter = " ";
  Datamode = " ";

  XSPECFilter.clear();

  return;

}

// Check completeness and consistency of information in spectrum

string pha::check()
{
  ostringstream outstr;

  // Check for an exposure time > 0

  if ( Exposure <= 0.0 ) {
    outstr << "Exposure time is negative (" << Exposure << ")" << endl;
  }

  // Check for data

  if ( Pha.size() == 0 ) {
    outstr << "No data present" << endl;
  }

  // Check for consistency between the size of the Pha array and DetChans

  if ( Pha.size() > (size_t)DetChans ) {
    outstr << "Size of Pha array (" << Pha.size() << ") is greater than DetChans ("
	 << DetChans << ")" << endl;
  }

  // Check for consistency between the size of the Pha and StatError arrays

  if ( Pha.size() != StatError.size() && StatError.size() != 0 ) {
    outstr << "Size of Pha array (" << Pha.size() << ") differs from size of StatError array ("
	 << StatError.size() << ")" << endl;
  }

  // Check for consistency between the size of the Pha and SysError arrays

  if ( Pha.size() != SysError.size() && SysError.size() != 0 ) {
    outstr << "Size of Pha array (" << Pha.size() << ") differs from size of SysError array ("
	 << SysError.size() << ")" << endl;
  }

  // Check for consistency between the size of the Pha and Channel arrays

  if ( Pha.size() != Channel.size() && Channel.size() != 0 ) {
    outstr << "Size of Pha array (" << Channel.size() << ") differs from size of Channel array ("
	 << Channel.size() << ")" << endl;
  }

  // Check for consistency between the size of the Pha and Quality arrays

  if ( Pha.size() != Quality.size() && Quality.size() != 0 && Quality.size() != 1 ) {
    outstr << "Size of Pha array (" << Quality.size() << ") differs from size of Quality array ("
	 << Quality.size() << ")" << endl;
  }

  // Check for consistency between the size of the Pha and Group arrays

  if ( Pha.size() != Group.size() && Group.size() != 0 && Group.size() != 1 ) {
    outstr << "Size of Pha array (" << Group.size() << ") differs from size of Group array ("
	 << Group.size() << ")" << endl;
  }

  // Check for consistency between the size of the Pha and AreaScaling arrays

  if ( Pha.size() != AreaScaling.size() && AreaScaling.size() != 0 && AreaScaling.size() != 1 ) {
    outstr << "Size of Pha array (" << AreaScaling.size() << ") differs from size of AreaScaling array ("
	 << AreaScaling.size() << ")" << endl;
  }

  // Check for consistency between the size of the Pha and BackScaling arrays

  if ( Pha.size() != BackScaling.size() && BackScaling.size() != 0 && BackScaling.size() != 1 ) {
    outstr << "Size of Pha array (" << BackScaling.size() << ") differs from size of BackScaling array ("
	 << BackScaling.size() << ")" << endl;
  }

  // Check for consistency of Poisserr and presence of StatError

  if ( Poisserr && StatError.size() > 0 ) {
    outstr << "Poisserr is true but StatError present" << endl;
  }
  if ( !Poisserr && StatError.size() == 0 ) {
    outstr << "Poisserr is false but no StatError present" << endl;
  }

  // Check Datatype is either COUNT or RATE

  if ( Datatype != "COUNT" && Datatype != "RATE" ) {
    outstr << "Datatype (" << Datatype << ") must be COUNT or RATE" << endl;
  }

  // Check Spectrumtype is one of TOTAL, NET or BKG

  if ( Spectrumtype != "TOTAL" && Spectrumtype != "NET" && Spectrumtype != "BKG" ) {
    outstr << "Spectrumtype (" << Spectrumtype << ") must be TOTAL, NET or BKG" << endl;
  }

  return outstr.str();
}


// Write spectrum as type I file

Integer pha::write(string filename)
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
    return(CannotCreate);       
  }

  // set up the column descriptors for those attributes which need to be 
  // written as columns

  if ( SPneedCol(Channel) ) {
    ttype.push_back("CHANNEL");
    tform.push_back("J");
    tunit.push_back(" ");
  }

  if ( Datatype == "RATE" ) {
    ttype.push_back("RATE");
    tform.push_back("E");
    tunit.push_back("counts/s");
  } else {
    ttype.push_back("COUNTS");
    tform.push_back("J");
    tunit.push_back("counts");
  }
     
  if ( !Poisserr ) {
    ttype.push_back("STAT_ERR");
    tform.push_back("E");
    tunit.push_back(" ");
  }

  if ( SPneedCol(SysError) ) {
    ttype.push_back("SYS_ERR");
    tform.push_back("E");
    tunit.push_back(" ");
  }

  if ( SPneedCol(Quality) ) {
    ttype.push_back("QUALITY");
    tform.push_back("I");
    tunit.push_back(" ");
  }

  if ( SPneedCol(Group) ) {
    ttype.push_back("GROUPING");
    tform.push_back("I");
    tunit.push_back(" ");
  }

  if ( SPneedCol(AreaScaling) ) {
    ttype.push_back("AREASCAL");
    tform.push_back("E");
    tunit.push_back(" ");
  }

  if ( SPneedCol(BackScaling) ) {
    ttype.push_back("BACKSCAL");
    tform.push_back("E");
    tunit.push_back(" ");
  }

  // Create the new extension

  Table* pspectrum = pFits->addTable("SPECTRUM",Pha.size(),ttype,tform,tunit);
  Table& spectrum = *pspectrum;

  // Write the standard keywords
  
  SPwriteKey(spectrum, "HDUCLASS", (string)"OGIP", Blank);
    
  SPwriteKey(spectrum, "HDUCLAS1", (string)"SPECTRUM", Blank);

  SPwriteKey(spectrum, "HDUCLAS2", Spectrumtype, Blank);
    
  SPwriteKey(spectrum, "HDUCLAS3", Datatype, Blank);
    
  SPwriteKey(spectrum, "CHANTYPE", ChannelType, "Channel type");

  SPwriteKey(spectrum, "HDUVERS", PHAVersion, "OGIP version number");

  SPwriteKey(spectrum, "TELESCOP", Telescope, Blank);

  SPwriteKey(spectrum, "INSTRUME", Instrument, Blank);

  SPwriteKey(spectrum, "DETNAM", Detector, Blank);

  SPwriteKey(spectrum, "FILTER", Filter, Blank);

  SPwriteKey(spectrum, "DATAMODE", Datamode, Blank);

  SPwriteKey(spectrum, "DETCHANS", DetChans, "Number of channels in spectrum");

  SPwriteKey(spectrum, "TLMIN1", FirstChannel, "First channel number");

  SPwriteKey(spectrum, "EXPOSURE", Exposure, "Exposure time");

  SPwriteKey(spectrum, "CORRSCAL", CorrectionScaling, "Scaling for correction file");

  SPwriteKey(spectrum, "POISSERR", Poisserr, "Is error Poisson ?");

  SPwriteKey(spectrum, "RESPFILE", ResponseFile, Blank);

  SPwriteKey(spectrum, "ANCRFILE", AncillaryFile, Blank);

  SPwriteKey(spectrum, "BACKFILE", BackgroundFile, Blank);

  SPwriteKey(spectrum, "CORRFILE", CorrectionFile, Blank);

  for (size_t i=0; i<XSPECFilter.size(); i++) {
    ostringstream KeyStream;
    KeyStream << "XFLT" << setfill('0') << setw(4) << i+1;
    string KeyName(KeyStream.str());
    SPwriteKey(spectrum, KeyName, XSPECFilter[i], Blank);
  }

  // Write the arrays - if an array is of size 1 or all the same value 
  // it will be written as a keyword

  if ( Channel.size() > 1 ) {
    SPwriteCol(spectrum, "CHANNEL", Channel);
  }

  if ( Datatype == "RATE") {
    SPwriteCol(spectrum, "RATE", Pha);
  } else {
    SPwriteCol(spectrum, "COUNTS", Pha);
  }

  if (!Poisserr) SPwriteCol(spectrum, "STAT_ERR", StatError);

  SPwriteCol(spectrum, "SYS_ERR", SysError);

  SPwriteCol(spectrum, "QUALITY", Quality);

  SPwriteCol(spectrum, "GROUPING", Group);

  SPwriteCol(spectrum, "AREASCAL", AreaScaling);

  SPwriteCol(spectrum, "BACKSCAL", BackScaling);

  return(0);
}

// Write spectrum as type I file copying extra keywords and extensions from
// another file

Integer pha::write(string filename, string copyfilename)
{
  return(this->write(filename, copyfilename, 1));
}

// Write spectrum as type I file copying extra keywords and extensions from
// another file. The required spectrum is the HDUnumber instance of a spectrum
// extension in the file.

Integer pha::write(string filename, string copyfilename, Integer HDUnumber)
{
  Integer Status(0);

  Status = this->write(filename);

  if ( Status != 0 ) return(Status);

  Status = SPcopyKeys(copyfilename, filename, "SPECTRUM", HDUnumber);

  if ( Status != 0 ) return(Status);

  Status = SPcopyHDUs(copyfilename, filename);

  return(Status);
}

// Set grouping array from Grouping object

Integer pha::setGrouping(grouping& GroupInfo)
{

  // check for compatibility

  if ( NumberChannels() != GroupInfo.size() ) {
    return(InconsistentGrouping);
  }

  // loop through channels setting grouping array

  for (size_t i=0; i<(size_t)NumberChannels(); i++) {
    if ( GroupInfo.newBin(i) ) {
      Group[i] = 1;
    } else {
      Group[i] = 0;
    }
  }

  return(0);
}

// Rebin channels based on the Grouping object

Integer pha::rebinChannels(grouping& GroupInfo)
{
  // check for compatibility

  if ( NumberChannels() != GroupInfo.size() ) {
    return(InconsistentGrouping);
  }

  vector<Real> temp;
  Integer NumberOrigChannels(NumberChannels());

  // bin up the data

  GroupBin(Pha, SumMode, GroupInfo, temp);
  Pha.resize(temp.size());
  Pha = temp;
  temp.clear();

  // if necessary sum in quadrature the error

  if ( !Poisserr ) {

    GroupBin(StatError, SumQuadMode, GroupInfo, temp);
    StatError.resize(temp.size());
    StatError = temp;
    temp.clear();

  }

  // if necessary sum in quadrature the systematic error

  if ( SysError.size() > 0 ) {

    GroupBin(SysError, SumQuadMode, GroupInfo, temp);
    SysError.resize(temp.size());
    SysError = temp;
    temp.clear();

  }

  // reset Channel array - need to take into account that not all channels
  // may be given values in the original spectrum

  Integer FirstOrigChannel(Channel[0]);
  Channel.resize(NumberChannels());
  for (size_t i=0; i<(size_t)NumberChannels(); i++) {
    Channel[i] = FirstOrigChannel + i;
  }
  DetChans -= NumberOrigChannels - NumberChannels();

  // redo Quality array - if any element making up a bin has bad quality then
  // the whole bin does

  vector<Integer> newQual;
  for (size_t i=0; i<Quality.size(); i++) {
    if ( GroupInfo.newBin(i) ) {
      newQual.push_back(Quality[i]);
    } else {
      if ( Quality[i] != 0 ) newQual[newQual.size()-1] = Quality[i];
    }
  }
  Quality.resize(newQual.size());
  for (size_t i=0; i<Quality.size(); i++) Quality[i] = newQual[i];

  // it doesn't make sense to try to preserve the grouping so reset

  Group.resize(NumberChannels());
  for (size_t i=0; i<Group.size(); i++) Group[i] = 1;

  return(0);
}


//*******************************************************************************
// Some utility routines. Definitions in pha.h


// Return the type of a PHA extension

// Integer PHAtype(string filename, Integer PHAnumber)

Integer PHAtype(string filename, Integer PHAnumber)
{
  Integer Status(0);
  return(PHAtype(filename, PHAnumber, Status));
}

Integer PHAtype(string filename, Integer PHAnumber, Integer& Status)
{
  if ( Status != 0 ) return 0;

  const string hduName("SPECTRUM");
  string DefString;

  // Read in the SPECTRUM extension number PHAnumber
  // and set up an object called spectrum with the contents

  const vector<string> hduKeys;
  const vector<string> primaryKey;

  auto_ptr<FITS> pInfile(0);

  try {
    pInfile.reset(new FITS(filename,Read,hduName,false,hduKeys,primaryKey,(int)PHAnumber));
  } catch(...) {
    Status = NoSuchFile;
    return 0;
  }

  ExtHDU& spectrum = pInfile->extension(hduName);
  
  // Check for a HDUCLAS4 keyword of "TYPEII"

  DefString = "TYPEI";
  try {
    string hduclas4 = SPreadKey(spectrum, "HDUCLAS4", 1, DefString);
    if ( hduclas4 == "TYPEII" ) return 2;
  } catch (Table::NoSuchKeyword&) {
  }

  // Check for a type II extension by looking for the SPEC_NUM column

  try {
    Column& Col = spectrum.column("SPEC_NUM");
    if ( Col.rows() > 0 ) return 2;
  } catch (Table::NoSuchColumn&) {
  }

  // If there is no SPEC_NUM column then look for a RATE or COUNTS column
  // and check the TFORM

  try {
    Column& Col = spectrum.column("COUNTS");
    if ( Col.varLength() || Col.repeat() > 1 ) return 2;
  } catch (Table::NoSuchColumn&) {
    try {
      Column& Col = spectrum.column("RATE");
      if ( Col.varLength() || Col.repeat() > 1 ) return 2;
    } catch (Table::NoSuchColumn&) {
    }
  }
       
  // None of the checks for a type II extension panned out so it must be a type I

  return 1;
} 

// return 0 if COUNTS column exists and is integer or COUNTS column does not exist

bool IsPHAcounts(string filename, Integer PHAnumber)
{
  Integer Status(0);
  return(IsPHAcounts(filename, PHAnumber, Status));
}

bool IsPHAcounts(string filename, Integer PHAnumber, Integer& Status)
{

  if ( Status != 0 ) return 0;

  const string hduName("SPECTRUM");

  // Read in the SPECTRUM extension number PHAnumber
  // and set up an object called spectrum with the contents

  const vector<string> hduKeys;
  const vector<string> primaryKey;

  auto_ptr<FITS> pInfile(0);

  try{
    pInfile.reset(new FITS(filename,Read,hduName,false,hduKeys,primaryKey,(int)PHAnumber));
  } catch(...) {
    Status = NoSuchFile;
    return false;
  }

  ExtHDU& spectrum = pInfile->extension(hduName);

  try {
    Column& Col = spectrum.column("COUNTS");
    ValueType Type = Col.type();
    if ( Type == TSHORT || Type == TLONG ) return true;
  } catch (Table::NoSuchColumn&) {
  }

  return false;

}

// return the number of spectra in a type II PHA extension

Integer NumberofSpectra(string filename, Integer PHAnumber)
{
  Integer Status(0);
  return(NumberofSpectra(filename, PHAnumber, Status));
}

Integer NumberofSpectra(string filename, Integer PHAnumber, Integer& Status)
{

  if ( Status != 0 ) return 0;

  const string hduName("SPECTRUM");

  // Read in the SPECTRUM extension number PHAnumber
  // and set up an object called spectrum with the contents

  const vector<string> hduKeys;
  const vector<string> primaryKey;

  auto_ptr<FITS> pInfile(0);

  try {
    pInfile.reset(new FITS(filename,Read,hduName,false,hduKeys,primaryKey,(int)PHAnumber));
  } catch(...) {
    Status = NoSuchFile;
    return 0;
  }

  ExtHDU& spectrum = pInfile->extension(hduName);

  return (Integer)spectrum.rows();

}



