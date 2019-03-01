#include "ahfits/ahfits.h"
#include "ahlog/ahlog.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <string.h>
#include "sys/stat.h"


namespace ahfits {


// =============================================================================
// FITS file operations and HDU navigation
// =============================================================================

void ahFitsOpen(const char * filename, const char * extension, AhFitsFilePtr * ahffp) {
  // Banner for constructing error messages.
  std::string banner(__func__); banner += ": "; 

  // Check inputs.
  if (0 == filename || 0 == extension || 0 == ahffp) 
    AH_THROW_RUNTIME("null pointer passed");

  // Initialize output variable to 0.
  *ahffp = 0;

  // Create local variable to hold created AhFitsFile object.
  AhFitsFilePtr fp(new AhFitsFile(filename));

  int status = 0;
  if (0 != fits_open_file(&(fp->m_cfitsfp), filename, READWRITE, &status)) {
    ahFitsClose(fp);
    AH_THROW_RUNTIME("unable to open file " + (std::string)filename);
  }

  try {
    ahFitsMove(fp, extension);
  } catch (const std::exception & x) {
    ahFitsClose(fp);
    AH_THROW_RUNTIME("failed to move to given extension, "+
                     (std::string)extension+", after opening file "+
                     (std::string)filename);
  }

  // Object is completely constructed: return it to the caller via the ahffp parameter.
  *ahffp = fp;
}

// -----------------------------------------------------------------------------

void ahFitsCreate(const char * filename, const char * fits_template, AhFitsFilePtr * ahffp, bool clobber) {
  // Banner for constructing error messages.
  std::string banner(__func__); banner += ": "; 

  // Check inputs.
  if (0 == filename || 0 == fits_template || 0 == ahffp) 
    AH_THROW_RUNTIME("null pointer passed");

  // Initialize output variable to 0.
  *ahffp = 0;

  // Create local variable to hold created AhFitsFile object.
  AhFitsFilePtr fp(new AhFitsFile(filename));

  int status = 0;
  std::string full_filename(clobber ? "!" : "");
  full_filename += std::string(filename) + "(" + fits_template + ")";
  if (0 != fits_create_file(&(fp->m_cfitsfp), const_cast<char *>(full_filename.c_str()), &status)) {
    ahFitsClose(fp);
    AH_THROW_RUNTIME("unable to create file " + full_filename);
  }

  // Object is completely constructed: return it to the caller via the ahffp parameter.
  *ahffp = fp;
}

// -----------------------------------------------------------------------------

void ahFitsClone(std::string src, std::string dest, bool clobber){

  // check if two files are the same; if so, throw an error
  // need to inquire about allowing to overwrite input file
  // are stat() and lstat() okay to use?
  struct stat fileStat1,fileStat2;
  if (0 != stat(src.c_str(),&fileStat1)) {
    AH_THROW_RUNTIME("cannot stat source file (does it exist?): "+src);
  }
  if (0 == stat(dest.c_str(),&fileStat2)) {
    if (fileStat1.st_ino == fileStat2.st_ino) 
      AH_THROW_RUNTIME("source and destination files are the same");

    lstat(dest.c_str(),&fileStat2);
    if (0 != S_ISLNK(fileStat2.st_mode))
      AH_THROW_RUNTIME("destination is a symbolic link");
  }

  // Check inputs.
  /// \internal
  /// \todo Do we need to trim spaces?
  if (0 == src.size()) 
    AH_THROW_RUNTIME("empty string given for source file");
  if (0 == dest.size()) 
    AH_THROW_RUNTIME("empty string given for destination file");

  // open source file
  /// \internal
  /// \note Not using ahFitsOpen since it needs requires an extension.  I can 
  /// only be sure of the Primary extension, but using that returns an error.
  int status = 0;
  AhFitsFilePtr fp_src(new AhFitsFile(src));
  if (0 != fits_open_file(&(fp_src->m_cfitsfp), const_cast<char *>(src.c_str()),
      READONLY, &status)) {
    ahFitsClose(fp_src);
    AH_THROW_RUNTIME("unable to open source file (does it exist?): "+src);
  }

  // open destination file
  status=0;
  AhFitsFilePtr fp_dest(new AhFitsFile(dest));
  std::string full_filename(clobber ? "!" : "");
  full_filename+=dest;
  if (0 != fits_create_file(&(fp_dest->m_cfitsfp), 
      const_cast<char *>(full_filename.c_str()), &status)) {
    ahFitsClose(fp_src);
    ahFitsClose(fp_dest);
    AH_THROW_RUNTIME("unable to create output file: "+dest);
  }

  // copy entire file using fits_copy_file
  if (0 != fits_copy_file(fp_src->m_cfitsfp,fp_dest->m_cfitsfp,true,true,true,
      &status)) {
    ahFitsClose(fp_src);
    ahFitsClose(fp_dest);
    AH_THROW_RUNTIME("unable to clone file: "+src+" -> "+dest);
  }

  // close files
  ahFitsClose(fp_src);
  ahFitsClose(fp_dest);
}

// -----------------------------------------------------------------------------

void ahFitsMove(AhFitsFilePtr ahffp, const char * extension) {
  // Banner for constructing error messages.
  std::string banner(__func__); banner += ": "; 

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp || 0 == extension) 
    AH_THROW_RUNTIME("null pointer passed");

  // Need the cast in the following because cfitsio doesn't know about "const".
  int status = 0;
  if (strcmp((char*)extension,(char*)"") == 0)
  {
    if (0 != fits_movabs_hdu(ahffp->m_cfitsfp,2,NULL,&status)) {
      AH_THROW_RUNTIME("unable to move to first extension of "+ahffp->m_filename);
    }
  } else {
    if (0 != fits_movnam_hdu(ahffp->m_cfitsfp, ANY_HDU, const_cast<char *>(extension), 0, &status)) {
      AH_THROW_RUNTIME("unable to move to extension "+
                       (std::string)extension+" in file "+
                       (std::string)ahffp->m_filename);
    }
  }

  // Clear column information and move to first row
  ahFitsClearColInfo(ahffp);
  ahffp->m_currow=1;

  // Note this assumes the current extension is a table.
  if (ahFitsIsBintable(ahffp)) {
    if (0 != fits_get_num_rowsll(ahffp->m_cfitsfp, &ahffp->m_numrow, &status)) {
      AH_THROW_RUNTIME("unable to get number of rows in extension "+
      (std::string)extension+" in file "+(std::string)ahffp->m_filename);
    }
  }
}

// -----------------------------------------------------------------------------

bool ahFitsNextHDU(AhFitsFilePtr ahffp) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp) 
    AH_THROW_RUNTIME("null pointer passed");

  // attempt to move to next HDU
  int status=0;
  fits_movrel_hdu(ahffp->m_cfitsfp,1,NULL,&status);

  // Clear column information and move to first row
  ahFitsClearColInfo(ahffp);
  ahffp->m_currow=1;

  // check if move okay or reached last extension
  if (status == 0) return true;
  if (status == END_OF_FILE) return false;

  // other error
  char* err=new char[255];
  fits_get_errstatus(status,err);
  std::string msg=(std::string)err;
  AH_THROW_RUNTIME("problem with moving to next HDU; cfits message: "+msg);
  return false;
}

// -----------------------------------------------------------------------------

void ahFitsClose(AhFitsFilePtr ahffp) {
  if (0 != ahffp) {
    ahFitsDisconnectAll(ahffp);
    if (0 != ahffp->m_cfitsfp) {
      int status = 0;
      for (int hdunum = 1; 0 == fits_movabs_hdu(ahffp->m_cfitsfp, hdunum, 0, &status); ++hdunum) {
        fits_write_chksum(ahffp->m_cfitsfp, &status);
      }
      status = 0;
      fits_close_file(ahffp->m_cfitsfp, &status);
      ahffp->m_cfitsfp = 0;
    }
    delete ahffp; ahffp = 0;
  }
}

// -----------------------------------------------------------------------------

#if 0
// Return a string describing the current status of the given FITS file object.
const char * ahFitsStatusMessage(AhFitsFilePtr ahffp) {}
#endif

// -----------------------------------------------------------------------------

bool ahFitsIsBintable(AhFitsFilePtr ahffp) {
  int status=0;
  int hdutype;
  if (0 != fits_get_hdu_type(ahffp->m_cfitsfp,&hdutype,&status))
    AH_THROW_RUNTIME("cannot obtain extension type");
  if (BINARY_TBL == hdutype) return true;
  return false;
}


// -----------------------------------------------------------------------------

bool ahFitsIsImage(AhFitsFilePtr ahffp) {
  int status=0;
  int hdutype;
  if (0 != fits_get_hdu_type(ahffp->m_cfitsfp,&hdutype,&status))
    AH_THROW_RUNTIME("cannot obtain extension type");
  if (IMAGE_HDU == hdutype) return true;
  return false;
}


// -----------------------------------------------------------------------------

bool ahFitsIsASCII(AhFitsFilePtr ahffp) {
  int status=0;
  int hdutype;
  if (0 != fits_get_hdu_type(ahffp->m_cfitsfp,&hdutype,&status))
    AH_THROW_RUNTIME("cannot obtain extension type");
  if (ASCII_TBL == hdutype) return true;
  return false;
}

// =============================================================================
// Read/write header values
// =============================================================================

std::string ahFitsGetKeyValStr(AhFitsFilePtr ahffp, const std::string & key) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  int status=0;
  char* value=new char[80];       // 80 is the maximum size of value+comment?
  if (0 != fits_read_key(ahffp->m_cfitsfp,TSTRING,key.c_str(),value,NULL,
      &status)) {
    delete value;
    AH_THROW_RUNTIME("failed to read header keyword: "+key+
                     " in file: "+ahffp->m_filename);
  }

  // prepare output and clean-up
  std::string out=(std::string)value;
  delete value;

  return out;
}

// -----------------------------------------------------------------------------

void ahFitsWriteKeyValStr(AhFitsFilePtr ahffp, const std::string & key,
                          const std::string & value, 
                          const std::string & comment) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  int status=0;
  char* tvalue=(char*)value.c_str();
  char* tcomment=(char*)comment.c_str();
  if (0 != fits_update_key(ahffp->m_cfitsfp,TSTRING,key.c_str(),tvalue,
                           tcomment,&status)) {
    AH_THROW_RUNTIME("failed to write/update header keyword: "+key+
                     "in file: "+ahffp->m_filename);
  }
}

// -----------------------------------------------------------------------------

double ahFitsGetKeyValDbl(AhFitsFilePtr ahffp, const std::string & key) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  int status=0;
  double value;
  if (0 != fits_read_key(ahffp->m_cfitsfp,TDOUBLE,key.c_str(),&value,NULL,
      &status)) {
    AH_THROW_RUNTIME("failed to read header keyword: "+key+
                     "in file: "+ahffp->m_filename);
  }

  return value;
}


// -----------------------------------------------------------------------------

void ahFitsWriteKeyValDbl(AhFitsFilePtr ahffp, const std::string & key,
                          const double & value, const std::string & comment) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  int status=0;
  double tvalue=(double)value;
  char* tcomment=(char*)comment.c_str();
  if (0 != fits_update_key(ahffp->m_cfitsfp,TDOUBLE,key.c_str(),&tvalue,
                           tcomment,&status)) {
    AH_THROW_RUNTIME("failed to write/update header keyword: "+key+
                     "in file: "+ahffp->m_filename);
  }
}

// -----------------------------------------------------------------------------

long long ahFitsGetKeyValLLong(AhFitsFilePtr ahffp, const std::string & key) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  int status=0;
  long long value;
  if (0 != fits_read_key(ahffp->m_cfitsfp,TLONGLONG,key.c_str(),&value,NULL,
      &status)) {
    AH_THROW_RUNTIME("failed to read header keyword: "+key+
                     "in file: "+ahffp->m_filename);
  }

  return value;
}

// -----------------------------------------------------------------------------

void ahFitsWriteKeyValLLong(AhFitsFilePtr ahffp, const std::string & key,
                          const long long & value, 
                          const std::string & comment) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  int status=0;
  long long tvalue=(long long)value;
  char* tcomment=(char*)comment.c_str();
  if (0 != fits_update_key(ahffp->m_cfitsfp,TLONGLONG,key.c_str(),&tvalue,
                           tcomment,&status)) {
    AH_THROW_RUNTIME("failed to write/update header keyword: "+key+
                     "in file: "+ahffp->m_filename);
  }
}

// -----------------------------------------------------------------------------

bool ahFitsGetKeyValBool(AhFitsFilePtr ahffp, const std::string & key) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  int status=0;
  char* value=new char[1];
  if (0 != fits_read_key(ahffp->m_cfitsfp,TLOGICAL,key.c_str(),value,NULL,
      &status)) {
    AH_THROW_RUNTIME("failed to read header keyword: "+key+
                     "in file: "+ahffp->m_filename);
  }

  // prepare output and cleanup
  bool out=false;
  if ((int)value[0] == 1) out=true;
  delete value;

  return out;
}

// -----------------------------------------------------------------------------

void ahFitsWriteKeyValBool(AhFitsFilePtr ahffp, const std::string & key,
                           const bool & value, const std::string & comment) {

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  int status=0;
  int tvalue=0;
  if (value) tvalue=1;
  char* tcomment=(char*)comment.c_str();
  if (0 != fits_update_key(ahffp->m_cfitsfp,TLOGICAL,key.c_str(),&tvalue,
                           tcomment,&status)) {
    AH_THROW_RUNTIME("failed to write/update header keyword: "+key+
                     "in file: "+ahffp->m_filename);
  }
}

// =============================================================================
// Function to load/retrieve column information
// =============================================================================

void ahFitsLoadColInfo(AhFitsFilePtr ahffp, const std::string name) {

  // if name in map, exit
  if (ahffp->m_name2num.count(name) > 0) return;

  // Find the column number associated with the given column name.
  int status = 0;
  int colnum = 0;
  if (0 != fits_get_colnum(ahffp->m_cfitsfp, CASEINSEN, 
                           const_cast<char *>(name.c_str()),&colnum,&status)) {
    AH_THROW_RUNTIME("column not found: " + name + ", in "+ahffp->m_filename);
  }

  // add name/num to map
  ahffp->m_name2num[name]=colnum;
  ahffp->m_num2name[colnum]=name;

  // get TYPECODE, REPEAT, and WIDTH
  int typecode = 0;
  long long repeat = 0;
  long long width = 0;
  status=0;
  if (0 != fits_get_coltypell(ahffp->m_cfitsfp,colnum,&typecode,&repeat,&width,
                              &status)) {
    AH_THROW_RUNTIME("could not get column types: " + name);
  }

  // get units (ignore rest of return parameters)
  std::string units="";
  if (ahFitsIsBintable(ahffp)) {
    char* tunits=new(char[80]);
    status=0;
    if (0 != fits_get_bcolparmsll(ahffp->m_cfitsfp,colnum,NULL,tunits,NULL,NULL,
                                  NULL,NULL,NULL,NULL,&status)) {
      delete tunits;
      AH_THROW_RUNTIME("could not get column parameters (binary table)");
    }
    units=(std::string)tunits;
    delete tunits;
  }

  // assign to struct
  ahffp->m_colinfo[colnum]=new AhFitsColumnInfo(colnum,name,units,typecode,repeat,
                                             width);

  return;
}

// -----------------------------------------------------------------------------

void ahFitsClearColInfo(AhFitsFilePtr ahffp) {

  // need to deallocate AhFitsColumnInfo structs
  std::map<int,AhFitsColumnInfo*>::iterator it;
  for (it=ahffp->m_colinfo.begin(); it != ahffp->m_colinfo.end(); it++) {
    delete (*it).second;
  }

  ahffp->m_name2num.clear();
  ahffp->m_num2name.clear();
  ahffp->m_colinfo.clear();
}

// -----------------------------------------------------------------------------

int ahFitsName2Num(AhFitsFilePtr ahffp, const std::string name) {
  return ahffp->m_name2num[name];
}

// -----------------------------------------------------------------------------

std::string ahFitsAddToColMap(AhFitsFilePtr ahffp, const int colnum){
  return ahffp->m_num2name[colnum];
}

// -----------------------------------------------------------------------------

int ahFitsColumnType(AhFitsFilePtr ahffp, const std::string name) {
  ahFitsLoadColInfo(ahffp,name);   // existence check inside
  return ahffp->m_colinfo[ahFitsName2Num(ahffp,name)]->m_typecode;
}

// -----------------------------------------------------------------------------

std::string ahFitsColumnUnits(AhFitsFilePtr ahffp, const std::string name) {
  ahFitsLoadColInfo(ahffp,name);   // existence check inside
  return ahffp->m_colinfo[ahFitsName2Num(ahffp,name)]->m_units;
}

// -----------------------------------------------------------------------------

long long ahFitsColumnRepeat(AhFitsFilePtr ahffp, const std::string name) {
  ahFitsLoadColInfo(ahffp,name);   // existence check inside
  return ahffp->m_colinfo[ahFitsName2Num(ahffp,name)]->m_repeat;
}

// -----------------------------------------------------------------------------

long long ahFitsColumnWidth(AhFitsFilePtr ahffp, const std::string name) {
  ahFitsLoadColInfo(ahffp,name);   // existence check inside
  return ahffp->m_colinfo[ahFitsName2Num(ahffp,name)]->m_width;
}

// -----------------------------------------------------------------------------

bool ahFitsHaveColumn(AhFitsFilePtr ahffp, const std::string name){
  try {
    ahFitsLoadColInfo(ahffp,name);
  } catch (...) {
    return false;
  }
  return true;
}

// =============================================================================
// Connect to column data and read/write rows
// =============================================================================

void ahFitsConnect(AhFitsFilePtr ahffp, const char * colname, void * data_ptr, long long * data_count) {
  // Banner for constructing error messages.
  std::string banner(__func__); banner += ": "; 

  // Check inputs. Note that 0 *may be* acceptable for data_count!
  if (0 == ahffp || 0 == ahffp->m_cfitsfp || 0 == colname || 0 == data_ptr)
    AH_THROW_RUNTIME("null pointer passed");

  // Find the column number associated with the given column name.
  int status = 0;
  int colnum = 0;
  if (0 != fits_get_colnum(ahffp->m_cfitsfp, CASEINSEN, const_cast<char *>(colname), &colnum, &status)) {
    AH_THROW_RUNTIME("could not get column number for column " + (std::string)colname);
  }

  // Find out what is in this column so that it may be read/written in its native format.
  int typecode = 0;
  long long repeat = 0;
  long long width = 0;
  if (0 != fits_get_coltypell(ahffp->m_cfitsfp, colnum, &typecode, &repeat, &width, &status)) {
    AH_THROW_RUNTIME("could not get column type for column " + (std::string)colname);
  }

  // Make sure data_count is defined for fixed-width columns.
  if (0 > typecode && 0 == data_count) {
    AH_THROW_RUNTIME("null data_count pointer passed for a variable-size column");
  }

  // Connect this column with the given data pointer.
  // TODO: this needs to confirm the pointer has enough memory.
  ahffp->m_connect.insert(std::make_pair(colnum, AhFitsColumn(colnum, typecode, repeat, width, data_ptr, data_count)));
}

// -----------------------------------------------------------------------------

void ahFitsDisconnectAll(AhFitsFilePtr ahffp) {
  if (0 != ahffp) ahffp->m_connect.clear();
}

// -----------------------------------------------------------------------------

void ahFitsReadRow(AhFitsFilePtr ahffp) {
  // Banner for constructing error messages.
  std::string banner(__func__); banner += ": "; 

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  // If anything remains in the file, move on to next row, and fill all
  // connected local variables.
  if (ahFitsReadOK(ahffp)) {
    int status = 0;
    fitsfile * fp = ahffp->m_cfitsfp; // For short.
    for (AhFitsFile::connect_type::iterator itor = ahffp->m_connect.begin(); itor != ahffp->m_connect.end(); ++itor) {
      int colnum = itor->first;
      AhFitsColumn & column(itor->second);
      int typecode = column.m_typecode;
      long long firstrow = ahffp->m_currow;
      long long repeat = column.m_repeat;
      long long width = column.m_width;
      void * data = column.m_data;
      // Check for negative typecode, which indicates a variable-size array.
      if (0 > typecode) {
        // Typecode passed below to fits_read_col must be positive.
        typecode *= -1;
        // Read the descriptor of this row to get the number of elements actually contained.
        if (0 != fits_read_descriptll(fp, colnum, firstrow, &repeat, 0, &status)) {
          AH_THROW_RUNTIME("error getting count of elements in table " + ahffp->m_filename);
        }
      }

      // In the case of string columns, the "repeat" variable is the total number of bytes in the column.
      // Need to divide by width in case there are more than one string.
      long long num_el = (TSTRING == typecode) ? repeat /= width : repeat;

      // Reset number of elements read -- if data count was assigned.
      if (0 != itor->second.m_data_count) *itor->second.m_data_count = 0;

      /// \todo This needs to read the null information and put it somewhere.
      // Read data from this row from cfitsio.
      if (0 != fits_read_col(fp, typecode, colnum, firstrow, 1, num_el, 0, data, 0, &status)) {
        AH_THROW_RUNTIME("error reading column in table " + ahffp->m_filename);
      }

      // Record the number of elements actually read -- if data count was assigned.
      if (0 != itor->second.m_data_count) *itor->second.m_data_count = repeat;
    }
  }
}

// -----------------------------------------------------------------------------

void ahFitsWriteRow(AhFitsFilePtr ahffp) {
  // Banner for constructing error messages.
  std::string banner(__func__); banner += ": "; 

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  // If anything remains in the file, move on to next row, and fill all
  // connected local variables.
  int status = 0;
  fitsfile * fp = ahffp->m_cfitsfp;
  for (AhFitsFile::connect_type::iterator itor = ahffp->m_connect.begin(); itor != ahffp->m_connect.end(); ++itor) {
    int colnum = itor->first;
    AhFitsColumn & column(itor->second);
    int typecode = column.m_typecode;
    long long firstrow = ahffp->m_currow;
    long long repeat = column.m_repeat;
    long long width = column.m_width;
    void * data = column.m_data;

    // Check for negative typecode, which indicates a variable-size array.
    if (0 > typecode) {
      // Typecode passed below to fits_read_col must be positive.
      typecode *= -1;
      // Repeat count must be taken from the count provided by the caller.
      if (0 != itor->second.m_data_count) repeat = *itor->second.m_data_count;
    }

    // In the case of string columns, the "repeat" variable is the total number of bytes in the column.
    // Need to divide by width in case there are more than one string.
    long long num_el = (TSTRING == typecode) ? repeat /= width : repeat;

    if (0 != fits_write_col(fp, typecode, colnum, firstrow, 1, num_el, data, &status)) {
      AH_THROW_RUNTIME("error writing column in table " + ahffp->m_filename);
    }
  }

  // Grow the table if needed.
  if (ahffp->m_currow > ahffp->m_numrow) ahffp->m_numrow = ahffp->m_currow;
}

// -----------------------------------------------------------------------------

void ahFitsFirstRow(AhFitsFilePtr ahffp) {
  // Banner for constructing error messages.
  std::string banner(__func__); banner += ": "; 

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

  // Confirm this file has at least one row.
  if (0 < ahffp->m_numrow) ahffp->m_currow = 1;
  else AH_THROW_RUNTIME("there are no rows in the table");
}

// -----------------------------------------------------------------------------

void ahFitsNextRow(AhFitsFilePtr ahffp) {
  // Banner for constructing error messages.
  std::string banner(__func__); banner += ": "; 

  // Check inputs.
  if (0 == ahffp || 0 == ahffp->m_cfitsfp)
    AH_THROW_RUNTIME("null pointer passed");

//  // Move on to next row, but don't go past end of file.
//  if (ahFitsReadOK(ahffp)) ++ahffp->m_currow;
//  else throw std::runtime_error(banner + "attempted to move past end of table");
  // Move to next row. Note advancing past end of table is possible.
  ++ahffp->m_currow;
}

// -----------------------------------------------------------------------------

bool ahFitsReadOK(AhFitsFilePtr ahffp) {
  return (0 != ahffp && 0 != ahffp->m_cfitsfp && ahffp->m_currow <= ahffp->m_numrow);
}

// =============================================================================


} // namespace ahfits

/* Revision Log
   $Log: ahfits.cxx,v $
   Revision 1.14  2012/06/28 23:14:49  mwitthoe
   ahmath: add function to convert string into interpolation type enumerated index; ahfits: small fix to error message

   Revision 1.13  2012/06/28 15:26:29  mwitthoe
   ahfits: added new functions to write/update header values

   Revision 1.12  2012/06/27 15:14:16  mwitthoe
   cleaned up ahfits code

   Revision 1.11  2012/06/27 14:37:03  mwitthoe
   remove ahFitsColumnIndex() from ahfits

   Revision 1.10  2012/06/26 23:23:34  mwitthoe
   add function to ahfits which checks if the given column exists

   Revision 1.9  2012/06/26 16:40:53  mwitthoe
   ahfits now moves to the first row after ahFitsMove() and ahFitsNextHDU()

   Revision 1.8  2012/06/20 17:03:44  mwitthoe
   now using throw macros from ahlog in ahfits

   Revision 1.7  2012/06/20 15:06:05  mwitthoe
   given an empty string, ahFitsOpen and ahFitsMove will go to the 2nd extension (1st=Primary being skipped)

   Revision 1.6  2012/06/18 14:55:46  mwitthoe
   added ahFitsNextHDU() to ahfits to allow looping over extensions in FITS files

   Revision 1.5  2012/06/14 22:27:30  mwitthoe
   ahfits: add functions to clone file, read header information, and get column information; test codes exist for each

   Revision 1.4  2012/05/17 19:37:08  peachey
   Add support for strings -- correct for different meaning of column 'repeat' value

   Revision 1.3  2012/02/03 15:16:00  peachey
   1. Remove "offset" and "m_offset" from AhFitsColumn. They are not
      necessary after all.
   2. Allow ahFitsNextRow to advance past the end of file. This allows
      ahFitsWriteRow to write new rows and grow the file. This is not
      dangerous for ahFitsReadRow because loops can use the ahFitsReadOK
      function to determine whether more input exists.
   3. Clarify messages.
   4. Correct update checksums to update checksums and datasums for the
      whole file before closing.

   Revision 1.2  2012/02/02 20:52:50  peachey
   1. Add to AhFitsColumn structure members m_offset (offset for
      variable-size columns) and m_data_count (number of items read from
      variable-size columns).
   2. Change ahFitsRead/WriteNextRow to ahFitsRead/Write, i.e., remove
      automatic advancement to the next row.
   3. Add ahFitsFirstRow and ahFitsNextRow to start at the beginning and
      advance to the next row, respectively.
   4. Use cfitsio to get/set new members of AhFitsColumn.

   Revision 1.1  2012/01/31 22:21:29  peachey
   Add first version of fits support code.

*/
