#ifndef QA_MAIN_H
#define QA_MAIN_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <valarray>
#include <vector>
#include <map>
#include <cmath>

#include <sys/time.h>

#include "udunits2.h"
#include "converter.h"

#include "split.h"
#include "data_statistics.h"
#include "date.h"
#include "freqDist.h"
#include "geo_meta.h"
#include "getopt_hdh.h"
#include "hdhC.h"
#include "nc_api.h"
#include "matrix_array.h"  // includes sources
#include "readline.h"
#include "statistics.h"

#include "iobj.h"
#include "variable.h"
#include "base.h"
#include "brace_op.h"
#include "fd_interface.h"
#include "cell_statistics.h"
#include "cf.h"
#include "annotation.h"
#include "in_file.h"
#include "oper.h"
#include "out_file.h"
#include "parse.h"
#include "qa.h"
#include "qa_data.h"
#include "qa_drs_cv_table.h"
#include "qa_time.h"
#include "qa_cnsty.h"

#if defined CORDEX
  #include "qa_CORDEX.h"
#elif defined CMIP5
  #include "qa_CMIP5.h"
#endif

#include "time_control.h"

/*! \file qa_main.h
 \brief The C++ main() entry point. Frame program for the Quality Control.

 int main(int argc, char *argv[]) contains
 vectors of objects, invokes option parsing and
 opening netCDF input file(s) accessing data.
 Multiple files may be accessed, also with matching a time-string
 pattern (see class TimeControl).
 Function entry() is called for each file, where all the
 record processing is done (QualityControl and frequency
 distribution).\n
 Provides an instance to the Annotation procedure.
 Various annotation levels trigger different exit
 states.\n
 Class and include files performing the qualitiy control
 must be linked to QA.cpp and qa.h, respectively.
*/

  struct IObjContainer
  {
    std::vector<Annotation> an;
    std::vector<CellStatistics> cS;
    std::vector<CF> cF ;
    std::vector<FD_interface> fDI ;
    std::vector<InFile> in ;
    std::vector<Oper> op ;
    std::vector<OutFile> out ;
    std::vector<QA> qA ;
    std::vector<TimeControl> tC;

    std::vector<IObj *> vIObj;
  } ;

//  ####  simple functions ####
/*! Where all the work is done: accessing of data,
 initialisation of base derived
 objects (not their instantiation) and record-wise processing.*/
bool
  entry(std::vector<IObj*> &);

/*! The finally() members of Cellstatistics, OutPut, QualtiyControl,
and FD_interface objects are processed.*/
int
  finally(IObjContainer &);

bool
  getFilename( std::string &s, std::string &name);

/*! Instantiation of objects to be generated in function makeObjects().
This is done in a separate function due to the template capability.*/
template <typename Type>
void
  instantiateObject(std::vector<Type> &, std::string &name, int id, std::string &options);

/*! Link obj referenced in the link-list to the calling obj.*/
  void
  linkObj( std::vector<std::vector<std::string> > &, IObjContainer &);

/*! Objects are created for all names defined in class Parse
(i.e. QA, TC, FD_interface, and Base-derived classes).*/
void
  makeObject( std::vector<std::vector<std::string> > &list, IObjContainer &);

/*! Command-line options are gathered from ordinary command-line arguments,
from file (prefixed by '--file'), and input-redirection (must be indicated by
'--input' on the command-line. The usual option-parsing is conducted, but also
a specific parsing (class Parse) in view of
instructions for the generation and cross-connection of Base-derived
objects as well as QualityControl, TimeControl, and FD-interface
objects. Brief explanation of Base-derived option-strings is given by
--help on the command-line*/
void
  parseOptions( int argc, char *argv[], IObjContainer &);

/*! Called in function parseOptions(); see there.*/
void
  readOptions(std::vector<std::string> &argv);

/*! Obsolete; to be replaced by the reference-counting feature of
MatVal<T> objects embedded in GeoData objects. Not effective in the Quality Control.*/
void
  scanGdRelation( std::vector<std::vector<std::string> > &list, IObjContainer &);

/*! Set options to objects. Calls a template function for the setting.*/
void
  setObjProperties( std::vector<std::vector<std::string> > &,
    IObjContainer & );

/*! Opening and accessing properties of netCDF files for input (InFile
objects). Also managing closing and opening of multiple files
whose names are extracted from an embedded TimeControl objects
via time-pattern matching. Not applied to the Quality Control.*/
bool
  updateIn(std::vector<InFile> &);

/*! Default value for filling grids with undefined value.*/
double fillingValue=-9.e+33 ;

/*! If there is a single input netCDF file, then its name may be
specified on the command-line by ordinary option '-f name'
(alternatively in the option-description-string for the InFile obj..*/
std::string NC_FILENAME;
std::string NC_PATH;
std::string TABLE_PATH;
std::string VERSION_FILE;

bool isPostProc;

void getWallClockSeconds(void);
void getCPUSeconds(void);
bool isCPU_time;

Annotation *notes=0 ;  //holds pointer of the obj with PROJECT_check-lists

#endif
