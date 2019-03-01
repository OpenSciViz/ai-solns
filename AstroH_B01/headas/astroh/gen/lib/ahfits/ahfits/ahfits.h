/** 
 * \file ahfits.h
 * \brief Wrapper to cfitsio
 * \author James Peachey
 * \date $Date: 2012/07/20 14:52:06 $
 *
 */
 
#ifndef AHFITS_AHFITS_H
#define AHFITS_AHFITS_H

#include "fitsio.h"

#include <map>
#include <string>

/// \ingroup mod_ahfits
namespace ahfits {


/** \addtogroup mod_ahfits
 *  @{
 */

// =============================================================================
// Structures and typedefs
// =============================================================================

/// \brief Structure containing column information
struct AhFitsColumnInfo {
  AhFitsColumnInfo(int colnum, std::string name, std::string units, int typecode, 
                long long repeat, long long width): 
                m_colnum(colnum), m_name(name), m_units(units), 
                m_typecode(typecode), m_repeat(repeat), m_width(width) {}

  int m_colnum;           ///< column index
  std::string m_name;     ///< column label
  std::string m_units;    ///< column units
  int m_typecode;         ///< type of column defined by cfitsio (TINT, TDOUBLE, etc); negative for variable column
  long long m_repeat;     ///< number of elements in binary table.
  long long m_width;      ///< width of each element in binary table.
};


/// \brief Structure for containing information about a FITS column. Note this 
///   currently only supports binary tables.
struct AhFitsColumn {
  AhFitsColumn(int colnum, int typecode, long long repeat, long long width, void * data, long long * data_count = 0):
    m_colnum(colnum), m_typecode(typecode), m_repeat(repeat), m_width(width), m_data(data), m_data_count(data_count) {}

  int m_colnum; ///< Number of the column.
  int m_typecode; ///< Type of the column as defined by cfitsio (TINT, TDOUBLE, etc.) Negative means variable column.
  long long m_repeat; ///< Number of elements in binary table.
  long long m_width; ///< Width of each element in binary table.
  void * m_data; ///< Pointer to the caller's defined local variable to hold data read/written to/from FITS files. May not be 0.
  long long * m_data_count; ///< MAY BE 0 for fixed size columns. Pointer to the caller's defined local variable.
};

/// \brief Structure for containing fitsfile and its buffers. Currently just
///   a placeholder. Actual content would include file names, Cfitsio fitsfile
///   pointers and buffers for reading/writing data.
struct AhFitsFile {
  AhFitsFile(const std::string & filename): m_connect(), m_filename(filename), m_cfitsfp(0), m_numrow(0), m_currow(1) {}

  typedef std::multimap<int, AhFitsColumn> connect_type;
  connect_type m_connect;
  std::string m_filename;
  fitsfile * m_cfitsfp;
  long long m_numrow;
  long long m_currow;
  std::map<std::string,int> m_name2num;
  std::map<int,std::string> m_num2name;
  std::map<int,AhFitsColumnInfo*> m_colinfo;
};

/// \brief Typedef to reduce the number of levels of pointer, to 
/// clarify/simplify what is going on in the code.
typedef AhFitsFile * AhFitsFilePtr;


// =============================================================================
// FITS file operations and HDU navigation
// =============================================================================

/// \brief Open the fits file(s) referenced by the filename, and move to the
///   given extension. Set up all internal buffering.
/// \param[in] filename The name of the file, (or @file_list for text file
///   listing multiple input files).
/// \param extension[in] The name of the extension in the file required.  An
///   empty string ("") will default to the 2nd extension (after Primary).
/// \param ahffp[out] The address of the output FITS file pointer that will
///   contain all the data needed to access the file(s).
void ahFitsOpen(const char * filename, const char * extension,
  AhFitsFilePtr * ahffp);

/// \brief Create the fits file(s) referenced by the filename, using the
///   provided FITS template. Set up all internal buffering.
/// \param[in] filename The name of the file, (or @file_list for text file
///   listing multiple output files).
/// \param[in] fits_template The name of the template file to use to create the
///   FITS file.
/// \param[out] ahffp The address of the output FITS file pointer that will
///   contain all the data needed to access the newly created file(s).
void ahFitsCreate(const char * filename, const char * fits_template, AhFitsFilePtr * ahffp, bool clobber = true);

/// \brief Copy FITS file.
/// \param[in] src filename to copy
/// \param[in] dest name of new file
/// \param[in] clobber true if replacing existing dest file (default: false)
void ahFitsClone(std::string src, std::string dest, bool clobber=false);

/// \brief Move to the given extension in the FITS file object.
/// \param[in] ahffp The FITS file pointer.
/// \param[in] extension The name of the extension in the file required. An
///   empty string ("") will default to the 2nd extension (after Primary).
void ahFitsMove(AhFitsFilePtr ahffp, const char * extension);

/// \brief Move to next extension in FITS file object.
/// \param[in] ahffp The FITS file pointer.
/// \return success of move
bool ahFitsNextHDU(AhFitsFilePtr ahffp);

/// \brief Close the given file object. Flush output, update checksum and
///   close the affected FITS files, free internal buffers.
/// \param[in] ahffp The FITS file pointer to be closed.
void ahFitsClose(AhFitsFilePtr ahffp);

/// \brief Return a string describing the current status of the given FITS
///   file object.
/// \param[in] ahffp The FITS file object.
const char * ahFitsStatusMessage(AhFitsFilePtr ahffp);

/// \brief Return true if HDU is a binary table.
/// \param[in] ahffp The FITS file object.
/// \internal
/// \note has not been tested for false
bool ahFitsIsBintable(AhFitsFilePtr ahffp);

/// \brief Return true if HDU is an image.
/// \param[in] ahffp The FITS file object.
/// \internal
/// \note has not been tested for true
bool ahFitsIsImage(AhFitsFilePtr ahffp);

/// \brief Return true if HDU is an ASCII table.
/// \param[in] ahffp The FITS file object.
/// \internal
/// \note has not been tested for true
bool ahFitsIsASCII(AhFitsFilePtr ahffp);

// =============================================================================
// Read/write header values
// =============================================================================

/// \brief From the active HDU, return header value from given key as a string.
/// \param[in] ahffp The FITS file object.
/// \param[in] key header key label
/// \return header value
std::string ahFitsGetKeyValStr(AhFitsFilePtr ahffp, const std::string & key);

/// \brief In the active HDU, write header value from given key as a string.
/// \param[in] ahffp The FITS file object.
/// \param[in] key header key label
/// \param[in] value header value
/// \param[in] comment header comment
void ahFitsWriteKeyValStr(AhFitsFilePtr ahffp, const std::string & key,
                          const std::string & value,
                          const std::string & comment);

/// \brief From the active HDU, return header value from given key as a double.
/// \param[in] ahffp The FITS file object.
/// \param[in] key header key label
/// \return header value
double ahFitsGetKeyValDbl(AhFitsFilePtr ahffp, const std::string & key);

/// \brief In the active HDU, write header value from given key as a double.
/// \param[in] ahffp The FITS file object.
/// \param[in] key header key label
/// \param[in] value header value
/// \param[in] comment header comment
void ahFitsWriteKeyValDbl(AhFitsFilePtr ahffp, const std::string & key,
                          const double & value, const std::string & comment);

/// \brief From the active HDU, return header value from given key as a long 
///  long.
/// \param[in] ahffp The FITS file object.
/// \param[in] key header key label
/// \return header value
long long ahFitsGetKeyValLLong(AhFitsFilePtr ahffp, const std::string & key);

/// \brief In the active HDU, write header value from given key as a long long.
/// \param[in] ahffp The FITS file object.
/// \param[in] key header key label
/// \param[in] value header value
/// \param[in] comment header comment
void ahFitsWriteKeyValLLong(AhFitsFilePtr ahffp, const std::string & key,
                          const long long & value, const std::string & comment);

/// \brief From the active HDU, return header value from given key as a boolean.
/// \param[in] ahffp The FITS file object.
/// \param[in] key header key label
/// \return header value
bool ahFitsGetKeyValBool(AhFitsFilePtr ahffp, const std::string & key);

/// \brief In the active HDU, write header value from given key as a boolean.
/// \param[in] ahffp The FITS file object.
/// \param[in] key header key label
/// \param[in] value header value
/// \param[in] comment header comment
void ahFitsWriteKeyValBool(AhFitsFilePtr ahffp, const std::string & key,
                           const bool & value, const std::string & comment);

// =============================================================================
// Function to load/retrieve column information
// =============================================================================

/// \brief read column information and store in column info map
/// \param[in] ahffp The FITS file object.
/// \param[in] name column label
void ahFitsLoadColInfo(AhFitsFilePtr ahffp, const std::string name);

/// \brief clear column information maps
/// \param[in] ahffp The FITS file object.
void ahFitsClearColInfo(AhFitsFilePtr ahffp);

/// \brief return column index given the column label
/// \param[in] ahffp The FITS file object.
/// \param[in] name column label
int ahFitsName2Num(AhFitsFilePtr ahffp, const std::string name);

/// \brief return column name given the column index
/// \param[in] ahffp The FITS file object.
/// \param[in] colnum column index
std::string ahFitsAddToColMap(AhFitsFilePtr ahffp, const int colnum);

/// \brief get column typecode
/// \param[in] ahffp The FITS file object.
/// \param[in] name column label
int ahFitsColumnType(AhFitsFilePtr ahffp, const std::string name);

/// \brief get column units
/// \param[in] ahffp The FITS file object.
/// \param[in] name column label
std::string ahFitsColumnUnits(AhFitsFilePtr ahffp, const std::string name);

/// \brief get column repeat
/// \param[in] ahffp The FITS file object.
/// \param[in] name column label
long long ahFitsColumnRepeat(AhFitsFilePtr ahffp, const std::string name);

/// \brief get column width
/// \param[in] ahffp The FITS file object.
/// \param[in] name column label
long long ahFitsColumnWidth(AhFitsFilePtr ahffp, const std::string name);

/// \brief check if column exists
/// \param[in] ahffp The FITS file object.
/// \param[in] name column label
bool ahFitsHaveColumn(AhFitsFilePtr ahffp, const std::string name);

// =============================================================================
// Connect to column data and read/write rows
// =============================================================================

/// \brief Connect the given data address to the given column in the
///   given AhFitsFilePtr. The AhFitsFilePtr will update the data whenever
///   it is advanced to the next row in the file, copying the data from
///   its internal buffers. In this way, a local variable can be tied to
///   a file, and used to read from and write to that file.
/// \param[in] ahffp The FITS file object that will connect to the data.
/// \param[in] colname The name of the column to associate with the data.
/// \param[in] data_ptr Address of the data array, which must be large enough
///   to hold one row of the data.
/// \param[in] data_count Address of a local variable that will hold the
///   count of elements read. May be 0 only for fixed-size columns.
///
/// For example, ahFitsConnect(fp, "TSTART", &tstart) would bind a local
/// variable named "tstart" to the TSTART column. When the file pointer
/// reads a new row, the local tstart variable would be filled with the
/// corresponding value from the TSTART column.
void ahFitsConnect(AhFitsFilePtr ahffp, const char * colname,
  void * data_ptr, long long * data_count);

/// \brief Disconnect all local data buffers from the given AhFitsFilePtr.
///   This must be called before any local data variables go out of scope
///   to prevent access violations.
/// \param[in] ahffp The FITS file object to be disconnected from local data.
void ahFitsDisconnectAll(AhFitsFilePtr ahffp);

/// \brief Read the next row from the input FITS file(s), filling
///   previously connected local variables with the appropriate data.
/// \param[in] ahffp The FITS file object to be read from.
///
/// For example, if the local tstart variable were previously connected
/// to the TSTART column, this would copy the TSTART value from the
/// current row to the tstart variable.
void ahFitsReadRow(AhFitsFilePtr ahffp);

/// \brief Write the next row to the output FITS file(s), taking
///   data from previously connected local variables.
/// \param[in] ahffp The FITS file object to be written to.
///
/// For example, if the local tstart variable were previously connected
/// to the TSTART column, this would copy the tstart variable to the
/// next row of the TSTART column.
void ahFitsWriteRow(AhFitsFilePtr ahffp);

/// \brief Go to first row of table.
/// \param[in] ahffp The FITS file object to be written to.
void ahFitsFirstRow(AhFitsFilePtr ahffp);

/// \brief Advance to next row of table.
/// \param[in] ahffp The FITS file object to be written to.
void ahFitsNextRow(AhFitsFilePtr ahffp);

/// \brief Return true if there is at least one unread row in the
///   given FITS file object, and the FITS file object has no errors currently.
/// \param[in] ahffp The FITS file object.
bool ahFitsReadOK(AhFitsFilePtr ahffp);


/** @} */


} // namespace ahfits

#endif   /* AHFITS_AHFITS_H */

/* Revision Log
   $Log: ahfits.h,v $
   Revision 1.11  2012/07/20 14:52:06  mwitthoe
   add Doxygen for ahfits

   Revision 1.10  2012/06/28 15:26:29  mwitthoe
   ahfits: added new functions to write/update header values

   Revision 1.9  2012/06/27 15:14:16  mwitthoe
   cleaned up ahfits code

   Revision 1.8  2012/06/27 14:37:02  mwitthoe
   remove ahFitsColumnIndex() from ahfits

   Revision 1.7  2012/06/26 23:23:33  mwitthoe
   add function to ahfits which checks if the given column exists

   Revision 1.6  2012/06/20 15:06:05  mwitthoe
   given an empty string, ahFitsOpen and ahFitsMove will go to the 2nd extension (1st=Primary being skipped)

   Revision 1.5  2012/06/18 14:55:46  mwitthoe
   added ahFitsNextHDU() to ahfits to allow looping over extensions in FITS files

   Revision 1.4  2012/06/14 22:27:29  mwitthoe
   ahfits: add functions to clone file, read header information, and get column information; test codes exist for each

   Revision 1.3  2012/02/03 15:12:17  peachey
   Remove "offset" and "m_offset" -- not needed after all.

   Revision 1.2  2012/02/02 20:41:48  peachey
   1. Add to AhFitsColumn structure members m_offset (offset for
      variable-size columns) and m_data_count (number of items read from
      variable-size columns).
   2. Change ahFitsRead/WriteNextRow to ahFitsRead/Write, i.e., remove
      automatic advancement to the next row.
   3. Add ahFitsFirstRow and ahFitsNextRow to start at the beginning and
      advance to the next row, respectively.

   Revision 1.1  2012/01/31 22:21:29  peachey
   Add first version of fits support code.

*/
