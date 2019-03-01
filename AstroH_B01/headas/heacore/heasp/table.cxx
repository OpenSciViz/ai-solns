
#ifndef HAVE_table
#include "table.h"
#endif

//-------------------------------------------------------------------------------
// Class table

// default constructor

table::table()
{
}

// destructor

table::~table()
{
}

// Push table Parameter object

void table::pushParameter(const tableParameter& paramObject)
{
  Parameters.push_back(paramObject);
  return;
}

// Push spectrum Parameter object

void table::pushSpectrum(const tableSpectrum& spectrumObject)
{
  Spectra.push_back(spectrumObject);
  return;
}

// Get table Parameter object (counts from zero)

tableParameter table::getParameter(Integer number)
{
  tableParameter paramObject;
  if ( number >= 0 && number < (Integer)Parameters.size() ) paramObject = Parameters[number];
  return paramObject;
}

// Get table Spectrum object (counts from zero)

tableSpectrum table::getSpectrum(Integer number)
{
  tableSpectrum spectrumObject;
  if ( number >= 0 && number < (Integer)Spectra.size() ) spectrumObject = Spectra[number];
  return spectrumObject;
}

// display information about the table - return as a string

string table::disp()
{
  ostringstream outstr;

  outstr << "Table information : " <<endl;
  outstr << "Model name                         = " << ModelName << endl;
  outstr << "Model units                        = " << ModelUnits<< endl;
  outstr << "Number of interpolation parameters = " << NumIntParams << endl;
  outstr << "Number of additional parameters    = " << NumAddParams << endl;
  outstr << "Model contains errors              = " << isError << endl;
  outstr << "Model includes redshift            = " << isRedshift << endl;
  outstr << "Model is additive                  = " << isAdditive << endl;
  outstr << "Number of model energies           = " << Energies.size() << endl;

  for (size_t i=0; i<Parameters.size(); i++) {
    outstr << " " << endl;
    Parameters[i].disp();
  }
  for (size_t i=0; i<Spectra.size(); i++) {
    outstr << " " << endl;
    Spectra[i].disp();
  }

  return outstr.str();
}

// clear contents of table object (mainly useful for Python)

void table::clear()
{
  Parameters.clear();
  Spectra.clear();
  ModelName = " ";
  ModelUnits = " ";
  NumIntParams = 0;
  NumAddParams = 0;
  isError = false;
  isRedshift = false;
  isAdditive = false;
  Energies.clear();
  return;
}

string table::check()
{
  ostringstream outstr;

  // check consistency of energy arrays

  for (size_t i=0; i<Spectra.size(); i++) {
    if ( Energies.size()-1 != Spectra[i].Flux.size() ) {
      outstr << "The size of the Energies array (" << Energies.size() 
	   << ") is not one more than that for the Flux array (" 
	   << Spectra[i].Flux.size() << ") for spectrum " << i << endl;
    }
    for (size_t j=0; j<Spectra[i].addFlux.size(); j++ ) {
      if ( Spectra[i].addFlux[j].size() != Spectra[i].Flux.size() ) {
	outstr << "The size of the addFlux array (" << Spectra[i].addFlux[j].size() 
	     << ") for additional parameter " << j 
	     << " is not equal to that for the Flux array (" 
	     << Spectra[i].Flux.size() << ") for spectrum " << i << endl;
      }
    }
  }

  // check consistency of parameter arrays

  if ( NumIntParams + NumAddParams != (Integer)Parameters.size() ) {
    outstr << "The number of Parameters objects (" << Parameters.size() 
	 << ") does not match the sum of interpolated and additional parameters ("
	 << NumIntParams+NumAddParams << ")" << endl;
  }

  // check that there are the correct number of spectrum objects

  Integer Nspec(1);
  for (size_t i=0; i<Parameters.size(); i++) Nspec *= Parameters[i].TabulatedValues.size();
  if ( Nspec != (Integer)Spectra.size() ) {
    outstr << "The number of Spectra objects (" << Spectra.size()
	 << ") does not match that expected from the Parameters objects ("
	 << Nspec << ")" << endl;
  }

  return outstr.str();
}

// write to a FITS file

Integer table::write(string filename)
{

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

  // Write the keywords to the primary header

  pFits->pHDU().addKey("HDUCLASS", "OGIP"," ");
  pFits->pHDU().addKey("HDUCLAS1", "XSPEC TABLE MODEL"," ");
  pFits->pHDU().addKey("HDUVERS", "1.0.0"," ");
  pFits->pHDU().addKey("MODLNAME", ModelName,"Table model name");
  pFits->pHDU().addKey("MODLUNIT", ModelUnits,"Table model units");
  pFits->pHDU().addKey("REDSHIFT", isRedshift,"Add redshift parameter?");
  pFits->pHDU().addKey("ADDMODEL", isAdditive,"Is model additive?");

  // Set up and create the PARAMETERS extension

  ttype.resize(10);
  tform.resize(10);
  tunit.resize(10);

  ttype[0] = "NAME";
  ttype[1] = "METHOD";
  ttype[2] = "INITIAL";
  ttype[3] = "DELTA";
  ttype[4] = "MINIMUM";
  ttype[5] = "BOTTOM";
  ttype[6] = "TOP";
  ttype[7] = "MAXIMUM";
  ttype[8] = "NUMBVALS";
  ttype[9] = "VALUE";

  tform[0] = "12A";
  tform[1] = "J";
  for (size_t i=2; i<8; i++) tform[i] = "E";
  tform[8] = "J";
  tform[9] = "PE";

  for (size_t i=0; i<10; i++) tunit[i] = " ";

  Table* pparam = pFits->addTable("PARAMETERS",Parameters.size(),ttype,tform,tunit);
  Table& param = *pparam;

  param.addKey("HDUCLASS", "OGIP"," ");
  param.addKey("HDUCLAS1", "XSPEC TABLE MODEL"," ");
  param.addKey("HDUCLAS2", "PARAMETERS"," ");
  param.addKey("HDUVERS", "1.0.0"," ");
  param.addKey("NINTPARM", NumIntParams,"Number of interpolation parameters ");
  param.addKey("NADDPARM", NumAddParams,"Number of additional parameters ");

  // write the parameter info. Note use of valarray because CCfits doesn't
  // support Column::write(vector<T>, Integer).

  size_t Nparams(Parameters.size());

  vector<string> names;
  for (size_t ipar=0; ipar<Nparams; ipar++) names.push_back(Parameters[ipar].Name);
  param.column("NAME").write(names,1);

  valarray<Integer> ivalues(Nparams);
  for (size_t ipar=0; ipar<Nparams; ipar++) ivalues[ipar] = Parameters[ipar].InterpolationMethod;
  param.column("METHOD").write(ivalues,1);

  valarray<Real> rvalues(Nparams);
  for (size_t ipar=0; ipar<Nparams; ipar++) rvalues[ipar] = Parameters[ipar].InitialValue;
  param.column("INITIAL").write(rvalues,1);

  for (size_t ipar=0; ipar<Nparams; ipar++) rvalues[ipar] = Parameters[ipar].Delta;
  param.column("DELTA").write(rvalues,1);

  for (size_t ipar=0; ipar<Nparams; ipar++) rvalues[ipar] = Parameters[ipar].Minimum;
  param.column("MINIMUM").write(rvalues,1);

  for (size_t ipar=0; ipar<Nparams; ipar++) rvalues[ipar] = Parameters[ipar].Bottom;
  param.column("BOTTOM").write(rvalues,1);

  for (size_t ipar=0; ipar<Nparams; ipar++) rvalues[ipar] = Parameters[ipar].Top;
  param.column("TOP").write(rvalues,1);

  for (size_t ipar=0; ipar<Nparams; ipar++) rvalues[ipar] = Parameters[ipar].Maximum;
  param.column("MAXIMUM").write(rvalues,1);

  for (size_t ipar=0; ipar<Nparams; ipar++) ivalues[ipar] = Parameters[ipar].TabulatedValues.size();
  param.column("NUMBVALS").write(ivalues,1);

  // workaround required here because Column::writeArrays will not take
  // vector<vector<T> > only vector<valarray<T> >.

  vector<valarray<Real> > pvalues(Nparams);
  for (size_t ipar=0; ipar<Nparams; ipar++) {
    pvalues[ipar].resize(Parameters[ipar].TabulatedValues.size());
    for (size_t i=0; i<Parameters[ipar].TabulatedValues.size(); i++) {
      pvalues[ipar][i] = Parameters[ipar].TabulatedValues[i];
    }
  }
  param.column("VALUE").writeArrays(pvalues,1);

  // Create the ENERGIES extension

  ttype.resize(2);
  tform.resize(2);
  tunit.resize(2);

  ttype[0] = "ENERG_LO";
  tform[0] = "E";
  tunit[0] = " ";

  ttype[1] = "ENERG_HI";
  tform[1] = "E";
  tunit[1] = " ";

  Table* penergies = pFits->addTable("ENERGIES",Energies.size()-1,ttype,tform,tunit);
  Table& energies = *penergies;

  energies.addKey("HDUCLASS", "OGIP"," ");
  energies.addKey("HDUCLAS1", "XSPEC TABLE MODEL"," ");
  energies.addKey("HDUCLAS2", "ENERGIES"," ");
  energies.addKey("HDUVERS", "1.0.0"," ");

  // write the energies

  size_t Nenergies(Energies.size()-1);
  rvalues.resize(Nenergies);

  for (size_t ien=0; ien<Nenergies; ien++) rvalues[ien] = Energies[ien];
  energies.column("ENERG_LO").write(rvalues,1);

  for (size_t ien=0; ien<Nenergies; ien++) rvalues[ien] = Energies[ien+1];
  energies.column("ENERG_HI").write(rvalues,1);

  // Create the SPECTRA extension

  ttype.resize(2+NumAddParams);
  tform.resize(2+NumAddParams);
  tunit.resize(2+NumAddParams);

  stringstream RepeatStream;
  RepeatStream << NumIntParams;
  string Repeat(RepeatStream.str());

  ttype[0] = "PARAMVAL";
  tform[0] = Repeat+"E";
  tunit[0] = " ";

  RepeatStream.str("");
  RepeatStream << Nenergies;
  Repeat = RepeatStream.str();

  ttype[1] = "INTPSPEC";
  tform[1] = Repeat+"E";
  tunit[1] = " ";

  for (size_t iadd=1; iadd<=(size_t)NumAddParams; iadd++) {
    RepeatStream.str("");
    RepeatStream << iadd;
    Repeat = RepeatStream.str();
    if ( iadd < 10 ) {
      ttype[1+iadd] = "ADDSP00"+Repeat;
    } else if ( iadd < 100 ) {
      ttype[1+iadd] = "ADDSP0"+Repeat;
    } else if ( iadd < 1000 ) {
      ttype[1+iadd] = "ADDSP"+Repeat;
    }
    RepeatStream.str("");
    RepeatStream << Nenergies;
    Repeat = RepeatStream.str();
    tform[1+iadd] = Repeat+"E";
    tunit[1+iadd] = " ";
  }

  Table* pspectra = pFits->addTable("SPECTRA",Spectra.size(),ttype,tform,tunit);
  Table& spectra = *pspectra;

  spectra.addKey("HDUCLASS", "OGIP"," ");
  spectra.addKey("HDUCLAS1", "XSPEC TABLE MODEL"," ");
  spectra.addKey("HDUCLAS2", "MODEL SPECTRA"," ");
  spectra.addKey("HDUVERS", "1.0.0"," ");

  // Write the spectra

  size_t Nspectra(Spectra.size());
  vector<valarray<Real> > rarray(Nspectra);

  if ( NumIntParams > 1 ) {
    for (size_t isp=0; isp<Nspectra; isp++) {
      rarray[isp].resize(NumIntParams);
      for (size_t j=0; j<(size_t)NumIntParams; j++) {
	rarray[isp][j] = Spectra[isp].ParameterValues[j];
      }
    }
    spectra.column("PARAMVAL").writeArrays(rarray,1);
  } else {
    rvalues.resize(Nspectra);
    for (size_t isp=0; isp<Nspectra; isp++) rvalues[isp] = Spectra[isp].ParameterValues[0];
    spectra.column("PARAMVAL").write(rvalues,1);
  }

  if ( Nenergies > 1 ) {
    for (size_t isp=0; isp<Nspectra; isp++) {
      rarray[isp].resize(Nenergies);
      for (size_t j=0; j<(size_t)Nenergies; j++) {
	rarray[isp][j] = Spectra[isp].Flux[j];
      }
    }
    spectra.column("INTPSPEC").writeArrays(rarray,1);

    for (size_t iadd=1; iadd<=(size_t)NumAddParams; iadd++) {
      for (size_t isp=0; isp<Nspectra; isp++) {
	rarray[isp].resize(Nenergies);
	for (size_t j=0; j<(size_t)Nenergies; j++) {
	  rarray[isp][j] = Spectra[isp].addFlux[iadd-1][j];
	}
      }
      spectra.column(ttype[iadd+1]).writeArrays(rarray,1);
    }
  } else {
    rvalues.resize(Nspectra);
    for (size_t isp=0; isp<Nspectra; isp++) rvalues[isp] = Spectra[isp].Flux[0];
    spectra.column("INTPSPEC").write(rvalues,1);

    for (size_t iadd=1; iadd<=(size_t)NumAddParams; iadd++) {
      for (size_t isp=0; isp<Nspectra; isp++) rvalues[isp] = Spectra[isp].addFlux[iadd-1][0];
      spectra.column(ttype[iadd+1]).write(rvalues,1);
    }

  }  


  return(0);

}

//-------------------------------------------------------------------------------
// Class tableParameter

// default constructor

tableParameter::tableParameter()
{
}

// destructor

tableParameter::~tableParameter()
{
}

// display information about the table parameter - return as a string

string tableParameter::disp()
{
  ostringstream outstr;

  outstr << "Parameter information : " << endl;
  outstr << "Parameter name       = " << Name << endl;
  outstr << "Interpolation method = ";
  if ( InterpolationMethod == 0 ) {
    outstr << " linear " << endl;
  } else {
    outstr << " logarthmic " << endl;
  }
  outstr << "Initial value        = " << InitialValue << endl;
  outstr << "Delta                = " << Delta << endl;
  outstr << "Minimum              = " << Minimum << endl;
  outstr << "Bottom               = " << Bottom << endl;
  outstr << "Top                  = " << Top << endl;
  outstr << "Maximum              = " << Maximum << endl;
  outstr << "Tabulated values     = ";
  for (size_t i=0; i<TabulatedValues.size(); i++) outstr << TabulatedValues[i] << "  ";
  outstr << endl;

  return outstr.str();
}

// clear contents of the table parameter (mainly useful for Python)

void tableParameter::clear()
{
  Name = " ";
  InterpolationMethod = 0;
  InitialValue = 0.0;
  Delta = 0.0;
  Minimum = 0.0;
  Bottom = 0.0;
  Top = 0.0;
  Maximum = 0.0;
  TabulatedValues.clear();
  return;
}


//-------------------------------------------------------------------------------
// Class tableSpectrum

// default constructor

tableSpectrum::tableSpectrum()
{
}

// destructor

tableSpectrum::~tableSpectrum()
{
}

// push an additional parameter spectrum

void tableSpectrum::pushaddFlux(vector<Real> input)
{
  addFlux.push_back(input);
  return;
}

// get an additional parameter spectrum

vector<Real> tableSpectrum::getaddFlux(Integer Number)
{
  vector<Real> values;
  if ( Number >=0 && Number < (Integer)addFlux.size() ) {
    for (size_t i=0; i<addFlux[Number].size(); i++) values.push_back(addFlux[Number][i]);
  }
  return values;
}

// Display information about the table spectrum - return as a string

string tableSpectrum::disp()
{
  ostringstream outstr;

  outstr << "Spectrum information : " << endl;
  outstr << "Number of model flux bins        = " << Flux.size() << endl;
  outstr << "Parameter values                 = ";
  for (size_t i=0; i<ParameterValues.size(); i++) outstr << ParameterValues[i] << "  ";
  outstr << endl;
  outstr << "Number of additional flux arrays = " << addFlux.size();
  return outstr.str();
}

// clear contents of the table parameter (mainly useful for Python)

void tableSpectrum::clear()
{
  Flux.clear();
  ParameterValues.clear();
  addFlux.clear();
  return;
}
