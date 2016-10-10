#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "hdhC.h"
#include "brace_op.h"
#include "date.h"
#include "getopt_hdh.h"
#include "readline.h"
#include "netcdf.h"  // local instance
#include "annotation.h"

#include "hdhC.cpp"
#include "BraceOP.cpp"
#include "Date.cpp"
#include "GetOpt_hdh.cpp"
#include "ReadLine.cpp"
#include "Split.cpp"
#include "Annotation.cpp"

/*! \file syncFiles.cpp
\brief Get the next filename for the QA processing.

netCDF files provided on the command-line (exclusive) or on stdin
are synchronised to a command-line given target.\n
The next filename to process is printed.\n

Options:\n
   -E               Print info about the entire set of files (date-sorted names.\n
                    and annotations; last line is for the target file (if any).\n
   -d [date0-]date1 Time limit for a starting and/or ending point.
                    A range may be separated by a slash or a dash;\n
                    omission indicates the end.\n
   -m               Allow for mixing of filenames with and without time range.\n
   -M               Test modification times.\n
   -P string        Path to the files.\n
   -p qa-nc-file    QA result file with path.\n
   -S               As -s, additionally with begin and end time.\n
                    Note that the range is given anyway in case of error.\n
   -T               Determine and append the total time range to the output.\n
   -t [t0-]t2       As for -d, but referring to the time values.\n
                    Time attributes of files must be identical.\n
   --help \n
   --note           not implemented, yet\n
   --only-marked    Only issue failing properties\n
   --line-feed=str  Newline indicator \n
\n
return:  output: \n
  0      Name of next file(s).\n
  1      "" , i.e. qa-file is up-to-date or invalid.\n
  2      Last filename if the date is older than.\n
         the end-date in the QA-result file.\n
  3      Unspecific error.\n
  4      No time info found; output filename.\n
  5      Invalid data.\n
>10      Ambiguity test failed (values accumulate):\n
  +1        misaligned dates in filenames \n
  +2        modification time check failed \n
  +4        identical start and/or end date across files \n
  +8        misaligned time-values across files \n
  +16       filenames with a mix of date formats \n
  +32       suspicion of a renewed ensemble. \n
         Output: file names sorted according to the modification time. */

class Member
{
  public:
  Member();

  void   enableModTimeNote(void){isModTimeNote=true;}
  void   enablePrintDateRange(void){isPrintDateRange=true;}
  void   enablePrintOnlyMarked(void){isPrintOnlyMarked=true;}
  void   enableNewLine(std::string &nl){newline=nl;}
  void   getModificationTime(void);
  std::string
         getFile(void){ return path + filename ; }
  std::string
         getOutput(bool printSeparationLine=false);
  void   print(bool printSeparationLine=false);
  void   putState(std::string, bool printDateRange=true);
  void   setBegin(Date);
  void   setEnd(Date);
  void   setFile(std::string&); // decomposes
  void   setFilename(std::string f){ filename = f;}
  void   setPath(std::string &p){ path = p;}

  std::string  filename;
  std::string  path;
  std::string  state;
  std::string  newline;

  bool         isPrintOnlyMarked ;
  bool         isPrintDateRange;
  bool         isModTimeNote;

  Date        begin;
  Date        end;
  Date        refDate ;
  long        modTime;
} ;

class Ensemble
{
   public:
   Ensemble();

   void   addTarget( std::string &qa_target );
   int    constraint(std::string &timeLimitStr);
   int    constraintSeries(void);
   int    constraintTimeLimit(std::string &timeLimitStr);
   void   enablePrintEnsemble(void) {isPrintEnsemble=true;}
   void   enablePrintOnlyMarked(void){isPrintOnlyMarked=true;}
   void   enablePrintDateRange(void){isPrintDateRange=true;}
   void   enableNewLine(std::string &nl){newline=nl;}
   std::string
          getDateSpan(void);
   std::string
          getIso8601(std::string);
   std::string
          getOutput(void);
   int    getTimes(std::string &);
   void   print(void);
   void   printDateSpan(void);
//   void   setAnnotation( Annotation *n ) { notes = n ;}
   void   setPath( std::string &p){path=p;}
   void   sortDate(void);
   int    testAmbiguity(std::string &,
                bool isOnlyMarked=false, bool isMod=false, bool isNoMix=true);

   bool   isInvalid;
   bool   isFormattedDate;
   bool   isNoRec;
   bool   isPrintEnsemble;
   bool   isPrintOnlyMarked ;
   bool   isPrintDateRange;
   bool   isWithTarget;

   size_t startIndex;  // default: 0, modifiable by a time-limit.
   size_t sz ;         // end of effective ensemble, which is modifiable.
   size_t last;        // the last index of the ensemble (target)

   std::string           newline;
   std::string           path;
   std::vector<Member*>  member ;
//   Annotation           *notes;
} ;

class SyncFiles
{
   public:
   SyncFiles( Ensemble &);

   void enableMixingRefused(void)       {isMixingRefused=true;}
   void enableModificationTimeTest(void){isModificationTest=true;}
   void enablePrintDateRange(void)      {ensemble->enablePrintDateRange();}
   void enablePrintEnsemble(void)       {ensemble->enablePrintEnsemble();}
   void enablePrintTotalTimeRange(void) {isPeriod=true;}
   void enableNewLine(std::string nl)   {ensemble->enableNewLine(nl);}
   void description(void);
   void print(void);
   int  printTimelessFile(std::string &);
   void readInput(void);
   void readArgv(int optInd, int argc, char *argv[]);
   int  run(void);

   void setTarget(std::string q)        {qa_target=q;}
   void setPath(std::string &p);
   void setQA_target(std::string s)     {qa_target=s;}
   void setTimeLimit(std::string s)     {timeLimitStr = s;}

   std::string path;
   std::string qa_target ;
   std::string timeLimitStr;

   bool isAddTimes;
   bool isInvalid;
   bool isMixingRefused;
   bool isModificationTest;
   bool isNoRec;
   bool isPeriod;
   bool isPrintOnlyMarked;

   int  returnValue;

   Ensemble   *ensemble;
//   Annotation *notes;
} ;

int main(int argc, char *argv[])
{
  Ensemble ensemble;

  // Note that ensemble is internally converted to pointer-type
  SyncFiles syncFiles(ensemble);

  std::string noteOpts;
//  Annotation *notes=0;

  std::string annotText;
  std::string outputText;

  std::string path;

  if( argc == 1 )
  {
    syncFiles.description();
    return 3;
  }

  GetOpt opt;
  opt.opterr=0;
  int copt;
  std::string str;
  std::string str0;
  std::string oStr("Ad:Ehl:mMP:p:St:T");
  oStr += "<--only-marked>";
  oStr += "<--help>";
  oStr += "<--line-feed>:";
  oStr += "<--note>:";

  while( (copt = opt.getopt(argc, argv, oStr.c_str() )) != -1 )
  {
    if( opt.longOption > 0 )
    {
      str0=opt.longOption;

      if( str0 == "--help" )
      {
        syncFiles.description();
        return 3;
      }

      if( str0 == "--line-feed" )
      {
        syncFiles.enableNewLine(opt.optarg);
        continue;
      }

      if( str0 == "--note" )
      {
        noteOpts=opt.optarg;
        continue;
      }

      if( str0 == "--only-marked" )
      {
        syncFiles.isPrintOnlyMarked=true;
        continue;
      }
    }

    switch ( copt )
    {
      case 'A':
        //obsolete
        break;
      case 'd':
        {
        str0=opt.optarg;
        std::string str("_");
        size_t p=0;

        if( (p=str0.find('/')) < std::string::npos )
          str = str0.replace(p, 0, "_") ; // is a range
        else if( (p=str0.find('-')) <  std::string::npos )
          str = str0.replace(p, 0, "_") ; // is a range
        else if( hdhC::isNumber(str0) )
          str += str0 ;  // end time
        else
          return 3;

        syncFiles.setTimeLimit(str) ;
        }
        break;
      case 'E':
        syncFiles.enablePrintEnsemble();
        break;
      case 'h':
        syncFiles.description();
        return 3;
        break;
      case 'l':
        syncFiles.setTimeLimit(opt.optarg);
        break;
      case 'm':
        syncFiles.enableMixingRefused();
        break;
      case 'M':
        syncFiles.enableModificationTimeTest();
        break;
      case 'P':
        path = opt.optarg ;
        syncFiles.setPath(path) ;
        break;
      case 'p':
        syncFiles.setQA_target(opt.optarg) ;
        break;
      case 'S':
        // printing date range of each file in output requested
        syncFiles.enablePrintDateRange();
        break;
      case 't':
        {
        str0=opt.optarg;
        std::string str("/");
        size_t p=0;

        if( (p=str0.find('-')) < std::string::npos )
          str = str0.replace(p, 0, "/") ; // is a range
        else if( (p=str0.find('/')) <  std::string::npos )
          str = str0 ;    // is a range separated by '/'
        else if( hdhC::isNumber(str0) )
          str += str0 ;  // end time
        else
          return 3;

        syncFiles.setTimeLimit(str) ;
        }
        break;
      case 'T':
        syncFiles.enablePrintTotalTimeRange() ;
        break;
      default:
        std::ostringstream ostr(std::ios::app);
        ostr << "syncFiles:getopt() unknown option -" ;
        ostr << copt;
        ostr << "\nreturned option -" << copt ;
        std::cout << ostr.str() << std::endl ;
        break;
    }
  }

  // Note= argc counts from 1, but argv[0] == this executable
  if( opt.optind == argc )
    syncFiles.readInput();
  else
    syncFiles.readArgv(opt.optind, argc, argv);

  // this does all
  return syncFiles.run();
}


Ensemble::Ensemble()
{
  isInvalid        = false;
  isFormattedDate  = false;
  isNoRec          = false;
  isPrintEnsemble  = false;
  isPrintOnlyMarked= false;
  isPrintDateRange = false;
  isWithTarget     = false;

  startIndex  = 0;
  newline="\\n";
}

void
Ensemble::addTarget( std::string &qa_target )
{
  // test for existance
  std::string testFile("/bin/bash -c \'test -e ") ;
  testFile += qa_target ;
  testFile += '\'' ;

  // see 'man system' for the return value, here we expect 0,
  if( system( testFile.c_str()) )
  {
     // we assume that we start a series; so there is no target
     return;
  }

  member.push_back( new Member );
  member.back()->setFile(qa_target) ;
  isWithTarget = true;
  ++last ;
  return ;
}

int
Ensemble::constraint(std::string &timeLimitStr)
{
  int retVal=0;

  if( timeLimitStr.size() )
    if( constraintTimeLimit(timeLimitStr) == 1 )
      return 1;

  // up-to-date
  if( isWithTarget )
  {
    if( member[last]->end  ==  member[sz-1]->end)
    {
       startIndex = sz; // ==> empty output

       if( sz == 1 )
         return 1;
    }
  }

  retVal += constraintSeries() ;

  return retVal;
}

int
Ensemble::constraintSeries(void)
{
  // Synchronisation according dates in a target file.

  // If target not available, then all files
  if( isWithTarget )
  {
    if( startIndex == sz )
        return 1;  // up-to-date
  }
  else
    return 0;

  int retVal=0;

  for( size_t i=startIndex ; i < sz ; ++i)
  {
    //Note that member[last] is for the target
    if( member[last]->end  <  member[i]->end)
    {
       startIndex = i;
       if( i == (sz-1) )
         retVal= 0;

       break;
    }
  }

  return retVal;
}

int
Ensemble::constraintTimeLimit(std::string &timeLimitStr)
{
  Date tl_beg;
  Date tl_end;

  bool  isBeg=false;
  bool  isEnd=false;

  // determine the kind of string provided
  // a) a range or a single instance of date or time
  // b) date(s) or time(s)

  // A range of times is separated by a slash.
  // A leading or trailing / is required, if only
  // a single value is provided for a time limit;
  // Omission of '_' indicates the end-date limit.

  bool isDate=true;
  char sep='_';
  if( timeLimitStr.find('/') < std::string::npos )
  {
    sep='/';
    isDate=false;
  }

  // Requires the same reference date for all files.
  // Requires the same calendar for all files and the time limit.
  Split splt(timeLimitStr,sep);
  splt.enableEmptyItems();

  if( isDate )
  { // it is a date limit
    if( splt[0].size() )
    {
      tl_beg.setDate( getIso8601( splt[0] ) );
      isBeg=true;
    }
    else
    {
      tl_end.setDate( getIso8601( splt[1] ) );
      isEnd=true;
    }
  }
  else
  { // a limit in terms of time values
    if( splt[0].size() )
    {
      tl_beg.setDate( member[0]->refDate );
      tl_beg.addTime(splt[0]);
      isBeg=true;
    }
    else
    {
      tl_end.setDate( member[0]->refDate );
      tl_end.addTime(splt[1]);
      isEnd=true;
    }
  }

/*
    else
    {
      std::ostringstream ostr(std::ios::app);
      ostr << "syncFiles.x::constraint(): option -t:\nargument ";
      ostr << timeLimitStr ;
      ostr << " requires separator '-' or '/'.\n";

      std::cout << ostr.str() << std::endl;
      return 3;
    }
*/

  // test
  if( isBeg )
  {
    // find first member fulfilling the time limit
    if( tl_beg > member[sz-1]->end )
    {
      startIndex=sz;  //in fact an up-to-date state
      return 1;
    }
    else
    {
      for( size_t i=0 ; i < sz ; ++i )
      {
        if( tl_beg <= member[i]->end )
        {
          startIndex = i ;  // reduce the file ensemble virtually
          break;
        }
      }
    }
  }

  // find last member fulfilling the time limit
  if( isEnd )
  {
    // limit set before the first file
    if( tl_end > member[startIndex]->begin )
    {
      for( size_t i=startIndex ; i < sz ; ++i )
      {
        if( tl_end < member[i]->begin )
        {
          sz = i ;  // reduce the file ensemble virtually
          break;
        }
      }
    }
    else
    {
      startIndex=sz;  //in fact an up-to-date state
      return 1;
    }
  }

  return 0;
}

std::string
Ensemble::getDateSpan(void)
{
   std::string span("PERIOD=");

   span += member[startIndex]->begin.str();
   span += '_' ;
   span += member[sz-1]->end.str();
   span +="\n";

   return span ;
}

std::string
Ensemble::getIso8601(std::string c)
{
  // compose ISO-8601 strings
  std::string iso("0000-01-01T00:00:00");

  if( c.size() > 3 )
    iso.replace(0, 4, c, 0, 4);
  if( c.size() > 5 )
    iso.replace(5, 2, c, 4, 2);
  if( c.size() > 7 )
    iso.replace(8, 2, c, 6, 2);
  if( c.size() > 9 )
    iso.replace(11, 2, c, 8, 2);
  if( c.size() > 11 )
    iso.replace(14, 2, c, 10, 2);
  if( c.size() > 13 )
    iso.replace(17, 2, c, 12, 2);

  return iso;
}

std::string
Ensemble::getOutput(void)
{
  if( isPrintEnsemble )
  {
    startIndex=0;
    sz=member.size();
    enablePrintDateRange();
//    isPrintOnlyMarked=false;
  }

  std::string s;
  bool printSepLine=false;

  for( size_t i=startIndex ; i < sz ; ++i)
  {
    if( isPrintOnlyMarked )
      member[i]->enablePrintOnlyMarked();
    if( isPrintDateRange )
      member[i]->enablePrintDateRange();
    if( newline.size() )
      member[i]->enableNewLine(newline);

    if( isWithTarget && i == (member.size()-1) )
      printSepLine=true;

    s += member[i]->getOutput(printSepLine) ;
  }

  return s ;
}

int
Ensemble::getTimes(std::string &str)
{
  // reading netcdf files
  int ncid, dimid, varid, status;

  size_t recSize, len;
  size_t index=0;
  double val ;
  int retVal=0;

  std::string begStr;
  std::string endStr;

  for( size_t i=0 ; i < member.size() ; ++i )
  {
     if( nc_open( member[i]->getFile().c_str(), 0, &ncid) )
     {
       isInvalid = true;
       member[i]->state = "could not open file";
       continue;
     }

     nc_inq_unlimdim(ncid, &dimid) ;

     if( dimid == -1 )
        // try for a regular dimension time
        nc_inq_dimid(ncid, "time", &dimid);

     // no unlimited variable found; this is not
     // neccessarily an error.
     if( dimid == -1 )
     {
       isNoRec = true;
       nc_close(ncid);

       if( sz == 1 )
       	 return 0; // a single file
       else
         retVal=4;

       isInvalid = true;
       member[i]->state += "missing time dimension";
       continue;
     }

     if( (status = nc_inq_dimlen(ncid, dimid, &recSize) ) )
     {
       isInvalid = true;
       member[i]->state = "could not read time dimension";
       nc_close(ncid);
       continue;
     }

     if( status == 0 && recSize == 0 )
     {
       // this could be wrong
       // or just a fixed variable with time dimension defined.
       isNoRec = true;
       nc_close(ncid);

       if( sz == 1 )
      	  return 0;

       isInvalid = true;
       member[i]->state = "no time values available";
       continue;
     }

     if( (status = nc_inq_varid(ncid, "time", &varid) ) )
     {
       isNoRec = true;
       nc_close(ncid);

       if( sz == 1 )
      	  return 0;

       member[i]->state = "missing time variable";
       continue;
     }

     // reference calendar
     if( (status = nc_inq_attlen(ncid, varid, "calendar", &len) ) )
     {  // failure
       member[i]->state = "missing calendar";
       nc_close(ncid);
       continue;
     }
     else
     {
       char *uc= new char [len+1] ;
       if( (status = nc_get_att_text(ncid, varid, "calendar", uc) ) )
       {
         member[i]->state = "missing calendar";
         nc_close(ncid);
         continue;
       }
       else
       { // successful reading of calendar
         uc[len]='\0';
         member[i]->refDate.setCalendar(uc) ;
         delete [] uc;

         // reference date
         if( (status = nc_inq_attlen(ncid, varid, "units", &len) ) )
         {
           member[i]->state = "missing reference date";
           nc_close(ncid);
           continue;
         }
         else
         {
           char *uc= new char [len+1] ;
           if( (status = nc_get_att_text(ncid, varid, "units", uc) ) )
           {
              member[i]->state = "missing time units";
              nc_close(ncid);
              continue;
           }
           else
           {
             uc[len]='\0';

             std::string str(uc);
             if( str.find("%Y") < std::string::npos )
                member[i]->refDate.setFormattedDate();
             else
               member[i]->refDate = uc ;
             delete [] uc;
           }
         }
       }
     }

     // time values: fist and last
     index=0;
     if( (status = nc_get_var1_double(ncid, varid, &index, &val) ) )
     {
        if( sz > 1 )
        {
          isInvalid = true;
          member[i]->state = "could not read first time value";
          nc_close(ncid);
          continue;
        }
     }

     // any reasonable values available?
     nc_type var_type ;
     status = nc_inq_vartype(ncid, varid, &var_type) ;

     // return value type is double
     double fV;
     int noFill=0;
     int statusF = nc_inq_var_fill(ncid, varid, &noFill, &fV);

     if( noFill != 1 && (statusF || val == fV) )
     {
        isInvalid = true;
        member[i]->state = "first time value equals _FillValue";
        nc_close(ncid);
        continue;
     }

     if(member[i]->refDate.isValid(val))
       member[i]->setBegin( member[i]->refDate.getDate(val) );
     else
     {
       isInvalid=true;
       retVal=5;
       member[i]->putState("invalid data, found " + hdhC::double2String(val), false);
       nc_close(ncid);
       continue;
     }

     index=recSize-1;
     if( (status = nc_get_var1_double(ncid, varid, &index, &val) ) )
     {
        if( sz > 1 )
        {
          isInvalid = true;
          member[i]->putState("could not read last time value");
          nc_close(ncid);
        }
     }

     if( noFill != 1 && (statusF || val == fV) )
     {
        isInvalid = true;
        member[i]->putState("last time value equals _FillValue");
        nc_close(ncid);
        continue;
     }

     if(member[i]->refDate.isValid(val))
       member[i]->setEnd( member[i]->refDate.getDate(val) );
     else
     {
       isInvalid=true;
       retVal=5;
       member[i]->putState("invalid data, found " + hdhC::double2String(val), false);
     }

     nc_close(ncid);
  }

  // any serious conditions
  if( isInvalid )
  {
     // only files with annotations
     enablePrintOnlyMarked();
     str = getOutput();

    if( retVal )
      return retVal;
    else
      return 3 ;  // unspecific error
  }

  // get modification times of files
  for(size_t i=0 ; i < member.size() ; ++i )
    member[i]->getModificationTime() ;

  // sorted vector of pointers of dates
  sortDate();

  return 0;
}

void
Ensemble::print()
{
  std::cout << getOutput() ;

  return ;
}

void
Ensemble::printDateSpan(void)
{
   std::string span;
   span  = member[startIndex]->begin.str();
   span += '_' ;
   span += member[sz-1]->end.str();

   std::cout << "PERIOD=" << span << std::endl;
   return ;
}

void
Ensemble::sortDate(void)
{
  // sort member coresponding to dates (without target)
  // sort dates from old to young
  for( size_t i=0 ; i < sz-1 ; ++i)
  {
    for( size_t j=i+1 ; j < sz ; ++j)
    {
      // 'greater than' means later
      if( member[i]->begin > member[j]->begin )
      {
        Member *tmp=member[i];
        member[i]=member[j];
        member[j]=tmp;
      }
    }
  }

  return;
}

int
Ensemble::testAmbiguity( std::string &str,
   bool is_only_marked,
   bool isModificationTest,
   bool isMixingRefused )
{
  // this function operates on the time-sorted files

  //detect a renewed data set testing internal time values
  if( isWithTarget )
  {
    // Note that last points to the target
    if( member[last]->end  >  member[sz-1]->end )
    {
      startIndex = sz-1;
      sz = member.size();
      enablePrintOnlyMarked();

      str = getOutput();
      return 32;
    }
  }

  // may have added 10 eventually, depending on the type of error
  int returnVal=0;

  // test modification time against the target
  if( isModificationTest && isWithTarget )
  {
    int rV=0;
    for( size_t i=0 ; i < sz ; ++i)
    {
       // note: 'later than' is equivalent to 'greater than'
       if( member[last]->modTime  <  member[i]->modTime )
       {
         enablePrintEnsemble();
         member[i]->enableModTimeNote();

         member[i]->putState("modification time") ;
         if(!rV)
         {
           rV=2;
           returnVal += rV;
         }
       }
    }
  }

  // check the correct sequence of dates in the filename
  std::string t;
  std::string t0_b;
  std::string t0_e;
  std::string t1_b;
  std::string t1_e;
  Split splt;
  splt.setSeparator('-');

  bool isRange=false;
  bool isSingular=false;

  // check ascending begin and end dates from filenames, respectively.
  for( size_t i=0 ; i < sz ; ++i)
  {
     t1_b.clear();
     t1_e.clear();

     size_t pos=member[i]->filename.rfind('_') + 1  ;
     t=member[i]->filename.substr( pos ) ;

     if( (pos=t.rfind(".nc")) < std::string::npos )
       t=t.substr(0,pos);

     splt=t;  // separate the dates

     // ensures that filenames without dates work
     t1_b='X';
     t1_e='X';

     if( splt.size() > 0 )
     {
       if( hdhC::isDigit( splt[0] ) )
          t1_b = splt[0] ;

       if( splt.size() == 2 )
       {
         if( hdhC::isDigit( splt[1] ) )
            t1_e = splt[1] ;
         isRange=true;
       }
       else  // singular date
       {
          t1_e = t1_b;
          isSingular=true;
       }
     }

     if( i > 0 && t0_e > t1_b )
     {
       member[i-1]->putState("misaligned filename dates");
       member[i]->putState("misaligned filename dates");

       returnVal += 1;
       break;
     }

     t0_b=t1_b;
     t0_e=t1_e;
  }

  // Detection of a mix of ranges of dates and singular dates in filenames
  if( isMixingRefused && isSingular && isRange )
  {
       enablePrintEnsemble();
       str = getOutput();

       returnVal += 16;
  }

  // check temporal coverage; any identical begin or end?
  for( size_t i=1 ; i < sz ; ++i)
  {
     int rV=0;
     if( member[i-1]->begin == member[i]->begin )
     {
       enablePrintEnsemble();

       member[i-1]->putState("identical begin");
       member[i]->putState("identical begin");

         if(!rV)
         {
           rV=4;
           returnVal += rV;
         }
     }
     if( member[i-1]->end == member[i]->end )
     {
       enablePrintEnsemble();

       member[i-1]->putState("identical end");
       member[i]->putState("identical end");

       if(!rV)
       {
         rV=4;
         returnVal += rV;
       }
     }
  }

  // test for overlapping temporal coverage
  for( size_t i=1 ; i < sz ; ++i)
  {
     int rV=0;
     if( member[i-1]->end > member[i]->begin )
     {
       enablePrintEnsemble();

       member[i-1]->putState("misaligned time across files");

       if(!rV)
       {
         rV=8;
         returnVal += rV;
       }
     }
  }

  if( returnVal )
  {
    if( is_only_marked )
      enablePrintOnlyMarked();

    returnVal += 10;
    str = getOutput();
  }

  return returnVal ;
}

Member::Member()
{
  isPrintDateRange=false;
  isModTimeNote=false;
  isPrintOnlyMarked=false ;
  newline="\\n";
}

void
Member::getModificationTime(void)
{
  struct stat buffer;
  int         status;

  status = stat(getFile().c_str(), &buffer);

  if( status == 0 )
  {
    time_t t=buffer.st_mtime ;
    modTime = t ;
  }
  else
    modTime=0L;

  return  ;
}

std::string
Member::getOutput(bool printSepLine)
{
  std::string str ;

  if( isPrintOnlyMarked && state.size() == 0 )
    return str;

  // construct the output string
  if( printSepLine )
  {
    str = "------------------------------------------------";
    str += newline;
  }

  str += filename ;

  if( isPrintDateRange )
  {
   std::string t0( begin.str() );
   if( t0.substr(0,10) != "4294962583" )
   {
     str += "  ";
     str += t0 ;
     str += " - " ;
     str += end.str() ;
   }
  }

  if( state.size() )
  {
     str += ": " ;
     str += state ;
  }

/*
  if( isModTimeNote )
  {
     double v = static_cast<double>( modTime );
     str += hdhC::double2String(v);
  }
*/

  str += newline;

  return str ;
}

void
Member::print(bool printSeparationLine)
{
  std::cout << getOutput(printSeparationLine) ;

  return ;
}

void
Member::putState(std::string s, bool is)
{
  if(state.size())
    state += ",";

  state += s;

  if( is )
      isPrintDateRange = is;
  return;
}

void
Member::setBegin(Date d)
{
 begin = d;

 return;
}

void
Member::setEnd(Date d)
{
 end = d;

 return;
}

void
Member::setFile( std::string &f)
{
  size_t pos;
  if( (pos=f.rfind('/') ) < std::string::npos )
  {
    path = f.substr(0,++pos);
    filename = f.substr(pos);
  }
  else
    filename = f;

  return ;
}

SyncFiles::SyncFiles( Ensemble &e)
{
  ensemble = &e;
//  notes=0;

  isAddTimes         = false;
  isInvalid          = false;
  isMixingRefused    = true;
  isModificationTest = false;
  isNoRec            = false;
  isPeriod           = false;

  returnValue=0;
}

void
SyncFiles::description(void)
{
  std::cout << "usage: synxFiles.x [Opts] file[s]\n";
  std::cout << "netCDF files provided on the command-line\n";
  std::cout << "are synchronised to a command-line given target.\n";
  std::cout << "No option: print name of file with first date.\n";
  std::cout << "-d [date0/]date1\n";
  std::cout << "         Time limit for a starting and/or ending point.\n";
  std::cout << "         A range may be separated by a slash or a dash;\n";
  std::cout << "         omission indicates the end.\n";
  std::cout << "-E       Show info about all files (purpose: debugging).\n";
  std::cout << "-P path  Common path of all data files, but the target.\n";
  std::cout << "-p qa_<variable>.nc-file\n";
  std::cout << "         QA result file with path.\n";
  std::cout << "-s       Output the sequence of all remaining files.\n";
  std::cout << "-t begin_date-end_date \n";
  std::cout << "         Requires separator '-'; no spaces\n";
  std::cout << "-t [t0/]t1  As for -d, but referring to the time values.\n";
  std::cout << "         Time attributes of files must be identical.\n\n";
  std::cout << "Output: one  or more filenames.\n";
  std::cout << "return:  0  File(s) synchronised to the target.\n";
  std::cout << "         1  qa-file is up-to-date.\n";
  std::cout << "         2  File with latest date; but, \n";
  std::cout << "            the date is older than in the qa-file.\n";
  std::cout << "         3  Unspecific error.\n";
  std::cout << "         4  No unlimited dimension found.\n";
  std::cout << "        10  Ambiguity check failed (return values accumulate):\n";
  std::cout << "            +1  misaligned dates in filenames.\n";
  std::cout << "            +2  modification time check failed.\n";
  std::cout << "            +4  identical start and/or end date across files.\n";
  std::cout << "            +8  misaligned time-values across files.\n";
  std::cout << "            +16 filenames with a mix of date formats.\n";
  std::cout << "            +32 suspicion of a renewed ensemble.\n";
  std::cout << "       +50 the last file of ensemble is next or up-to-date.\n";

  std::cout << std::endl;

  return;
}

void
SyncFiles::print(void)
{
  ensemble->print();

  if( isPeriod )
    ensemble->printDateSpan();

  return ;
}

int
SyncFiles::printTimelessFile(std::string &str)
{
   // true below means: print only marked entries

   if( ensemble->sz > 1 )
   {  // occurence within an ensemble of files: error
     std::string key("17_1");
     std::string capt("Determination of temporal sequence of  files failed.");

     // More than a single file is an error; output filenames
     ensemble->enablePrintOnlyMarked();
     str = ensemble->getOutput() ;
     return 10;
   }
   else if( ensemble->sz == 1 )
   {
      if( ensemble->member[0]->state.size() )
      {
        ensemble->enablePrintOnlyMarked();
        str = ensemble->getOutput() ;
        return 3;  // a fixed field file, but with error
      }
      else
        return 4;  // a fixed field file
   }
   else
   {
      ensemble->enablePrintOnlyMarked();
      str = ensemble->getOutput() ;
      return 3 ;  // unspecific error
   }
}

void
SyncFiles::readInput(void)
{
  ReadLine rL;

  // now switch to std::cin
  rL.connect_cin();  // std::cin

  // read instructions
  while( ! rL.readLine() )
  {
     for( size_t i=0 ; i < rL.size() ; ++i )
     {
       ensemble->member.push_back( new Member );

       if( path.size() )
         ensemble->member.back()->setPath(path);

       ensemble->member.back()->setFilename(rL.getItem(i));
     }
  }

  // sz will be changed corresponding to the effective range
  ensemble->sz = ensemble->member.size() ;
  ensemble->last = ensemble->sz -1 ; // the real size
  return;
}

void
SyncFiles::readArgv
(int i, int argc, char *argv[])
{
  for( ; i < argc ; ++i)
  {
    ensemble->member.push_back( new Member );

    if( path.size() )
    {
      ensemble->member.back()->setPath(path);
      ensemble->member.back()->setFilename(argv[i]);
    }
    else
      ensemble->member.back()->setFilename(argv[i]);
  }

  // sz will be changed corresponding to the effective range
  ensemble->sz = ensemble->member.size() ;
  ensemble->last = ensemble->sz -1; // the real size

  return;
}

int
SyncFiles::run(void)
{
  returnValue=0;

  // we append the qa_target, if available
  if( qa_target.size()  )
    ensemble->addTarget(qa_target);

  // read dates from nc-files,
  // sort dates in ascending order (old --> young)
  // and get modification times of files.
  // Any annotation would be done there.
  std::string str;

  returnValue = ensemble->getTimes(str) ;

  if( str.size() )  // exit condition found
  {
    std::cout << str ;
    return returnValue ;
  }

  // Did any 'no-record' occur? Error cases are trapped.
  if( ensemble->isNoRec )
  {
    returnValue = printTimelessFile(str) ;
    if( str.size() )
    {
      std::cout << str ;
      return returnValue ;
    }
  }

  // Check for ambiguities, return 0 for PASS.
  // Safe for a single file, because this was processed before
  if( (returnValue = ensemble->testAmbiguity
        (str, isPrintOnlyMarked, isModificationTest, isMixingRefused )) )
  {
    if( str.size() )
    {
      std::cout << str ;
      return returnValue;
    }
  }

  // Apply user supplied time-limits and/or
  // synchronisation to a target file.
  // Note that dates are already sorted.
  // Note: does not detect any error; just adjusts index range
  returnValue = ensemble->constraint(timeLimitStr) ;

  // successful run
  print();

  return returnValue ;
}

void
SyncFiles::setPath(std::string &p)
{
  path=p;

  if( path.size() && path[ path.size()-1 ] != '/' )
    path += '/' ;

  ensemble->setPath(path);

  return;
}
