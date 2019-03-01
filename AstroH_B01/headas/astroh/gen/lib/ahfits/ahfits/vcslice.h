#if !defined(__AHvcslice_h__)
#define __AHvcslice_h__ = "$Name: AstroH_B01 $ $Id: vcslice.h,v 1.1 2012/05/09 20:48:54 dhon Exp $"
//#define AHvcslice_h(arg) const char arg##AHvcslice_hRCSId[] = __AHvcslice_h__

#include "ahfits/ahfits.h"

#include <map>
#include <string>
#include <stdexcept>
#include <vector>

using namespace std;

/// \attention this supplements the ahfits namespace with convenience funcs. for 
/// variable-length binary-table apps. and such.
/// \auther David B. Hon
/// \date Mar-Apr 2012

namespace ahfits {
#define AHFOO // are c++ macros uniquely defined in namespaces? 
  /// some useful typedefs
  typedef std::vector<int> IntVec;
  typedef std::pair<int, string> IntStringPair;
  typedef std::pair<int, int*> IntVecPair;
  typedef std::vector<IntVecPair> VecOfIntVec;
  typedef std::pair<string, string> StringPair;
  typedef std::map<int, string> HashIntString;
  typedef std::map< string, IntStringPair > HDUInfo;
  typedef std::map< string, StringPair > TableInfo; // for one or more HDUs
  typedef std::map< string, IntVecPair> Cell1Dint; // integer data 1-d vec for HDU by name and table row index?
  typedef std::map< string, VecOfIntVec > Cell2Dint; // integer data 2-d vec for HDU by name and table row index (and row count)?
 
  /// \brief conveninience utility func. for use with cfitsio C lib args.
  /// \param AHFitsFilePtr the fits-file struct pointer
  /// \return cfitsio opened FITS file struct pointer:
  inline fitsfile* fitsFileStructPtr(AhFitsFilePtr ahf) {
    if( 0 == ahf ) {
      throw std::logic_error(string(__func__) + " AhFitsFilePtr null pointer passed.");
    }
    if( 0 == ahf->m_cfitsfp ) {
      throw std::runtime_error(string(__func__) + " No (open) FITS file.");
    }
    return ahf->m_cfitsfp;
  }

  /// \brief binary table (open) fits-file name
  /// \param AHFitsFilePtr the fits-file struct pointer
  /// \return string open file name
  string fileName(AhFitsFilePtr ahf);

  /// \brief HDU extension info.
  /// \param AhFitsFilePtr the fits-file struct pointer
  /// \param map< string hdu_name, std::pair<int, string> hdu_type > & an empty hash-map that gets populated and returned (by ref.)
  /// \return int number of hdus and sets hashmap hdu_name and type info (as pair):
  int infoHDU(HDUInfo & extinfo, AhFitsFilePtr ahf = 0);

  /// \brief binary table blob slice HDU table info.
  /// \param AHFitsFilePtr the fits-file struct pointer
  /// \param map< string hdu_name, pair<string, string> col_names >& an empty hash-map that gets populated and returned (by ref.)
  /// \return int number of hdus and set hdu_names and table dimenension info:
  /// For example a table cell containing a 1-D array should yield "x" info and an empty string for the "y" dim.
  /// ... note this establishes the name(s) of the table columns that hold x and optionally y dim. data.
  //int infoTable(AhFitsFilePtr ahf, map< string hdu_name, std::pair<string, string> col_names > & celldims);
  int infoTable(TableInfo & celldims, AhFitsFilePtr ahf = 0);

  /// \brief (binary) HDU table (for 1-D blob slice) data.
  /// \param map< string hdu_name, pair<int, int*> >& a hash-map of hdu names, with values that get populated from btable.
  /// The values should be variable arrays passed as a std::pair that provides a length and a
  /// pointer to an array of the indicated length.
  /// \param AHFitsFilePtr the fits-file struct pointer, if 0 use internal test data?
  /// \param rowIdx row indexd defaults to first row.   
  /// \return int size of vector and set hdu_names and table cell data:
  int get1Dcells(Cell1Dint & cellvec1d, string& col_name, AhFitsFilePtr ahf = 0, int rowIdx = 0, int rowCnt = 1);
  int get1Dcells(Cell1Dint & cellvec1d, int colIdx, AhFitsFilePtr ahf = 0, int rowIdx = 0, int rowCnt = 1);

  /// \brief (binary) HDU table (for 2-D blob slice) data.
  /// \param map< string hdu_name, vector<:pair<int, int*>> >& a hash-map of hdu name,
  ///  with value pairs that get populated and returned (by ref.)
  /// The values should be variable arrays passed as a vector of pairs, where each pair provides a length and a
  /// pointer to an array of the indicated length.
  /// \param AHFitsFilePtr the fits-file struct pointer, if 0 use internal test data?
  /// \param rowIdx row indexd defaults to first row.   
  /// \return int size of vector and set hdu_names and table cell data:
  //int get2Dcells(AhFitsFilePtr ahf, map< string hdu_name, std::vector<:pair<int, int*>>  vals > & cellvec2d);
  int get2Dcells(Cell2Dint & cellvec2d, string& col_name, AhFitsFilePtr ahf = 0, int rowIdx = 0, int rowCnt = 1);
  int get2Dcells(Cell2Dint & cellvec2d, int coldx, AhFitsFilePtr ahf = 0, int rowIdx = 0, int rowCnt = 1);
}

#endif /// __AHvcslice_h__
