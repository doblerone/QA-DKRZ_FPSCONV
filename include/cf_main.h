#ifndef CF_MAIN_H
#define CF_MAIN_H

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
//#include "freqDist.h"
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
#include "cf.h"
#include "annotation.h"
#include "in_file.h"
#include "parse.h"

/*! Command-line options are gathered from ordinary command-line arguments,
from file (prefixed by '--file'), and input-redirection (must be indicated by
'--input' on the command-line. The usual option-parsing is conducted, but also
a specific parsing (class Parse) in view of
instructions for the generation and cross-connection of Base-derived
objects as well as QualityControl, TimeControl, and FD-interface
objects. Brief explanation of Base-derived option-strings is given by
--help on the command-line*/
void
  parseOptions(int argc, char *org_argv[],
    InFile &in, CF &cf, Annotation &notes) ;

/*! Called in function parseOptions(); see there.*/
void
  readOptions(std::vector<std::string> &argv);

  hdhC::FileSplit file;

void
  setFilename(std::string f){file.setFile(f);}

#endif
