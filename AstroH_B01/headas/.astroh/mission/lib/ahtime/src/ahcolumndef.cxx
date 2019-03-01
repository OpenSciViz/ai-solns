/** 
 * \file ahcolumndef.cxx
 * \brief functions to act on the CALDB columndef file for AstroH.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/13 18:32:08 $
 *
 */
 
#include "ahtime/ahcolumndef.h"
#include "ahfits/ahfits.h"
#include "ahlog/ahlog.h"

#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  void write_coldef_template(const std::string & tplfile) {
    std::ofstream out;
    out.open(tplfile.c_str());
    out << "# Template file for COLUMNDEF FITS file" << std::endl;
    out << "" << std::endl;
    out << "xtension=bintable" << std::endl;
    out << "" << std::endl;
    out << "extname='COLUMNDEF'" << std::endl;
    out << "" << std::endl;
    out << "naxis2=1" << std::endl;
    out << "" << std::endl;
    out << "# Name of instrument or 'HK'" << std::endl;
    out << "# Values: SXS-Pixel, SXS-Antico, SXI, HXI, SGD_CC, SGD_WAM, HK" 
        << std::endl;
    out << "ttype# = SYSTEM" << std::endl;
    out << "tform# = 16A" << std::endl;
    out << "" << std::endl;
    out << "# column from current file (event or HK)" << std::endl;
    out << "ttype# = COLUM1" << std::endl;
    out << "tform# = 16A" << std::endl;
    out << "" << std::endl;
    out << "# column from current file (event or HK)" << std::endl;
    out << "ttype# = COLUM2" << std::endl;
    out << "tform# = 16A" << std::endl;
    out << "" << std::endl;
    out << "# column from event file" << std::endl;
    out << "ttype# = LTIME1EVT" << std::endl;
    out << "tform# = 16A" << std::endl;
    out << "" << std::endl;
    out << "# column from event file" << std::endl;
    out << "ttype# = LTIME2EVT" << std::endl;
    out << "tform# = 16A" << std::endl;
    out << "" << std::endl;
    out << "# column from HK file" << std::endl;
    out << "ttype# = LTIME1HK" << std::endl;
    out << "tform# = 16A" << std::endl;
    out << "" << std::endl;
    out << "# column from HK file" << std::endl;
    out << "ttype# = LTIME2HK" << std::endl;
    out << "tform# = 16A" << std::endl;
    out << "" << std::endl;
    out.close();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void write_coldef_fits(const std::string & filename, 
                         const std::string & tplfile) {

    // make FITS file from template
    ahfits::AhFitsFilePtr fptr;
    ahfits::ahFitsCreate(filename.c_str(),tplfile.c_str(),&fptr,true);
    ahfits::ahFitsClose(fptr);

    // re-open new file
    ahfits::ahFitsOpen(filename.c_str(),"COLUMNDEF",&fptr);
    if (!ahfits::ahFitsReadOK(fptr)) {
      AH_THROW_RUNTIME("failed to re-open columndef FITS file");
    }

    // make connections from local variables to FITS columns
    const int colw=16+1;
    char c_system[colw];
    char* f_system=c_system;
    ahfits::ahFitsConnect(fptr,"SYSTEM",&f_system,0);

    char c_colum1[colw];
    char* f_colum1=c_colum1;
    ahfits::ahFitsConnect(fptr,"COLUM1",&f_colum1,0);

    char c_colum2[colw];
    char* f_colum2=c_colum2;
    ahfits::ahFitsConnect(fptr,"COLUM2",&f_colum2,0);

    char c_ltime1evt[colw];
    char* f_ltime1evt=c_ltime1evt;
    ahfits::ahFitsConnect(fptr,"LTIME1EVT",&f_ltime1evt,0);

    char c_ltime2evt[colw];
    char* f_ltime2evt=c_ltime2evt;
    ahfits::ahFitsConnect(fptr,"LTIME2EVT",&f_ltime2evt,0);

    char c_ltime1hk[colw];
    char* f_ltime1hk=c_ltime1hk;
    ahfits::ahFitsConnect(fptr,"LTIME1HK",&f_ltime1hk,0);

    char c_ltime2hk[colw];
    char* f_ltime2hk=c_ltime2hk;
    ahfits::ahFitsConnect(fptr,"LTIME2HK",&f_ltime2hk,0);

    // SXS-Pixel
//    ahfits::ahFitsFirstRow(fptr);
    strcpy(c_system,"SXS-Pixel");
    strcpy(c_colum1,"TI");
    strcpy(c_colum2,"S_TIME");
    strcpy(c_ltime1evt,"SAMPLE_CNT");
    strcpy(c_ltime2evt,"TIME_VERNIER");
    strcpy(c_ltime1hk,"SAMPLE_CNT");
    strcpy(c_ltime2hk,"BASE_CNT");
    ahfits::ahFitsWriteRow(fptr);

    // SXS-Antico
    ahfits::ahFitsNextRow(fptr);
    strcpy(c_system,"SXS-Antico");
    strcpy(c_colum1,"TI");
    strcpy(c_colum2,"S_TIME");
    strcpy(c_ltime1evt,"SAMPLE_CNT");
    strcpy(c_ltime2evt,"");
    strcpy(c_ltime1hk,"");
    strcpy(c_ltime2hk,"");
    ahfits::ahFitsWriteRow(fptr);

    // SXI
    ahfits::ahFitsNextRow(fptr);
    strcpy(c_system,"SXI");
    strcpy(c_colum1,"TI");
    strcpy(c_colum2,"S_TIME");
    strcpy(c_ltime1evt,"Seq_Start_Time");
    strcpy(c_ltime2evt,"");
    strcpy(c_ltime1hk,"");
    strcpy(c_ltime2hk,"");
    ahfits::ahFitsWriteRow(fptr);

    // HXI
    ahfits::ahFitsNextRow(fptr);
    strcpy(c_system,"HXI");
    strcpy(c_colum1,"TI");
    strcpy(c_colum2,"S_TIME");
    strcpy(c_ltime1evt,"TRIGGER_TIME");
    strcpy(c_ltime2evt,"");
    strcpy(c_ltime1hk,"");
    strcpy(c_ltime2hk,"");
    ahfits::ahFitsWriteRow(fptr);

    // SGD_CC
    ahfits::ahFitsNextRow(fptr);
    strcpy(c_system,"SGD_CC");
    strcpy(c_colum1,"TI");
    strcpy(c_colum2,"S_TIME");
    strcpy(c_ltime1evt,"TRIGGER_TIME");
    strcpy(c_ltime2evt,"");
    strcpy(c_ltime1hk,"");
    strcpy(c_ltime2hk,"");
    ahfits::ahFitsWriteRow(fptr);

    // SGD_WAM
    ahfits::ahFitsNextRow(fptr);
    strcpy(c_system,"SGD_WAM");
    strcpy(c_colum1,"TI");
    strcpy(c_colum2,"S_TIME");
    strcpy(c_ltime1evt,"MIO_LOCALTIME");
    strcpy(c_ltime2evt,"");
    strcpy(c_ltime1hk,"");
    strcpy(c_ltime2hk,"");
    ahfits::ahFitsWriteRow(fptr);

    // HK
    ahfits::ahFitsNextRow(fptr);
    strcpy(c_system,"HK");
    strcpy(c_colum1,"TI");
    strcpy(c_colum2,"S_TIME");
    strcpy(c_ltime1evt,"");
    strcpy(c_ltime2evt,"");
    strcpy(c_ltime1hk,"");
    strcpy(c_ltime2hk,"");
    ahfits::ahFitsWriteRow(fptr);

    // close FITS file
    ahfits::ahFitsClose(fptr);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  std::string get_caldb_coldef() {
    return "columndef.fits";
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  /// \internal
  /// \todo return value is currently hard-coded; ultimately want to read 
  /// width from header in FITS file
  int get_coldef_width(const std::string & filename) {
    return 16;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void get_coldef_columns(const std::string & filename, 
                          const std::string & inst,
                          coldef_names & cols) {

    // open file and go to correct row
    ahfits::AhFitsFilePtr fptr;
    ahfits::ahFitsOpen(filename.c_str(),"COLUMNDEF",&fptr);
    if (!ahfits::ahFitsReadOK(fptr)) {
      std::stringstream msg;
      msg << "failed to open columndef FITS file: " << filename;
      AH_THROW_RUNTIME(msg.str());
    }

    // allocate char* for temporary column names
    const int colw=get_coldef_width(filename);
    char* c_system=new char[colw];
    char* c_colum1=new char[colw];
    char* c_colum2=new char[colw];
    char* c_ltime1evt=new char[colw];
    char* c_ltime2evt=new char[colw];
    char* c_ltime1hk=new char[colw];
    char* c_ltime2hk=new char[colw];

    bool okay=true;       // set to false if a failure occurs
    bool gotit=false;     // set to true when instrument is found
    try {
      // make connections
      ahfits::ahFitsConnect(fptr,"SYSTEM",&c_system,0);
      ahfits::ahFitsConnect(fptr,"COLUM1",&c_colum1,0);
      ahfits::ahFitsConnect(fptr,"COLUM2",&c_colum2,0);
      ahfits::ahFitsConnect(fptr,"LTIME1EVT",&c_ltime1evt,0);
      ahfits::ahFitsConnect(fptr,"LTIME2EVT",&c_ltime2evt,0);
      ahfits::ahFitsConnect(fptr,"LTIME1HK",&c_ltime1hk,0);
      ahfits::ahFitsConnect(fptr,"LTIME2HK",&c_ltime2hk,0);

      for (ahfits::ahFitsFirstRow(fptr); ahfits::ahFitsReadOK(fptr);
           ahfits::ahFitsNextRow(fptr)) {

        ahfits::ahFitsReadRow(fptr);
        if (c_system != inst) continue;
        gotit=true;
        break;
      }
    }
    catch (...) {
      okay=false;
    }

    if (!okay || !gotit) {
      ahfits::ahFitsClose(fptr);
      delete c_system;
      delete c_colum1;
      delete c_colum2;
      delete c_ltime1evt;
      delete c_ltime2evt;
      delete c_ltime1hk;
      delete c_ltime2hk;
      std::stringstream msg;
      msg << "failure getting column definitions for " << inst;
      AH_THROW_RUNTIME(msg.str());
    }

    // read and store data
    cols.colum1=(std::string)c_colum1;
    cols.colum2=(std::string)c_colum2;
    cols.ltime1evt=(std::string)c_ltime1evt;
    cols.ltime2evt=(std::string)c_ltime2evt;
    cols.ltime1hk=(std::string)c_ltime1hk;
    cols.ltime2hk=(std::string)c_ltime2hk;

    // delete pointers
    delete c_system;
    delete c_colum1;
    delete c_colum2;
    delete c_ltime1evt;
    delete c_ltime2evt;
    delete c_ltime1hk;
    delete c_ltime2hk;

    // clear string if just a single space
    if (cols.colum1 == " ") cols.colum1.clear();
    if (cols.colum2 == " ") cols.colum2.clear();
    if (cols.ltime1evt == " ") cols.ltime1evt.clear();
    if (cols.ltime2evt == " ") cols.ltime2evt.clear();
    if (cols.ltime1hk == " ") cols.ltime1hk.clear();
    if (cols.ltime2hk == " ") cols.ltime2hk.clear();

    // close FITS file
    ahfits::ahFitsClose(fptr);
  }

  // ---------------------------------------------------------------------------

}


/* Revision Log
 $Log: ahcolumndef.cxx,v $
 Revision 1.2  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
