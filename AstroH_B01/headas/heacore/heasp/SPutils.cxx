// Some useful general utility routines

#ifndef HAVE_SPutils
#include "SPutils.h"
#endif

// Read the units associated with a column

void SPreadColUnits(ExtHDU& ext, string ColName, string& Units)
{
  // get the column index

  try {
    Column& Col = ext.column(ColName);
    int index = Col.index();

    // construct the keyword name

    stringstream nameStream;
    nameStream << "TUNIT" << index;

    string DefString(" ");
    Units = SPreadKey(ext, nameStream.str(), DefString);

  } catch(Table::NoSuchColumn&){
    Units = " ";
  }

  return;
}

// Write the units associated with a column

void SPwriteColUnits(Table& table, string ColName, string Units)
{
  // get the column index

  try {
    Column& Col = table.column(ColName);
    int index = Col.index();

    // construct the keyword name

    stringstream nameStream;
    nameStream << "TUNIT" << index;
    SPwriteKey(table, nameStream.str(), Units, " ");

  } catch(Table::NoSuchColumn&){
  }

  return;
}

// Return a FITS string specification for the longest string in the input
// vector<string>

string SPstringTform(const vector<string>& Data)
{

  size_t length = Data[0].size();
  for (size_t i=1; i<Data.size(); i++) {
    if ( Data[i].size() > length ) length = Data[i].size();
  }

  stringstream tmpStream;
  tmpStream << length;

  return tmpStream.str();
}

// copy from infile to outfile all HDUs which are not manipulated by this library 

Integer SPcopyHDUs(string infile, string outfile)
{

  size_t NumIgnore(5);
  string IgnoreNames[5] = {"SPECTRUM", "SPECRESP", "EBOUNDS", 
			   "MATRIX", "SPECRESP MATRIX"};

  auto_ptr<FITS> pInfile(0);
  auto_ptr<FITS> pOutfile(0);

  // open the input file

  try {
    pInfile.reset(new FITS(infile));
  } catch(...) {
    return(NoSuchFile);
  }

  FITS InFITS(infile);

  // open the output file. If it doesn't exist then create it by copying the
  // primary header from the input file

  try {
    pOutfile.reset(new FITS(outfile, Write));
  } catch(...) {
    try {
      pOutfile.reset(new FITS(outfile, infile));
    } catch(...) {
      return(CannotCreate);
    }
  }

  // Loop through the extensions in the input file

  bool done(false);
  Integer extNum(1);

  while (!done) {

    try {
      ExtHDU& inExtension = pInfile->extension(extNum);
      string ExtName = inExtension.name();

      bool found(false);
      for (size_t i=0; i<NumIgnore; i++) {
  	if ( ExtName == IgnoreNames[i] ) found = true;
      }
      if (!found) {
  	pOutfile->copy(pInfile->extension(extNum));
      }
      extNum++;
    } catch(...) {
      done = true;
    }

  }

  return(0);
}

// copy non-critical keywords from infile to outfile for the HDUnumber instance
// of the HDUname HDU.

Integer SPcopyKeys(string infile, string outfile, string HDUname, Integer HDUnumber)
{

  const vector<string> hduKeys;
  const vector<string> primaryKey;

  auto_ptr<FITS> pInfile(0);
  auto_ptr<FITS> pOutfile(0);

  // open the input file to the requested extension

  try {
    pInfile.reset(new FITS(infile,Read,HDUname,false,hduKeys,primaryKey,(int)HDUnumber));
  } catch(...) {
    return(NoSuchFile);
  }

  HDU *inHDU = &pInfile->extension(HDUname);

  // read in the keywords

  inHDU->readAllKeys();

  // and the output file to the requested extension

  try {
    pOutfile.reset(new FITS(outfile,Write,HDUname,false,hduKeys,primaryKey,(int)HDUnumber));
  } catch(...) {
    return(NoSuchFile);
  }

  ExtHDU& outHDU = pOutfile->extension(HDUname);

  // copy keywords in categories TYP_CMPRS_KEY (20), TYP_CKSUM_KEY (100), TYP_WCS_KEY (110), 
  // TYP_REFSYS_KEY (120), and TYP_USER_KEY (150). This choice is currently hardwired into
  // CCfits - it may be necessary to change this at some point.

  outHDU.copyAllKeys(inHDU);

  return(0);
}
