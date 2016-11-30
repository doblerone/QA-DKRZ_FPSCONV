#include "time_control.h"

TimeControl::TimeControl()
{
  initDefaults();
}

TimeControl::TimeControl(const TimeControl &p)
{
  initDefaults();
  copy(p);
}

TimeControl::~TimeControl()
{
  if( xcptn.ofsError )
    delete xcptn.ofsError ;
}

TimeControl&
TimeControl::operator=( const TimeControl &p)
{
  initDefaults();

  if( this == &p )
    return *this;

  copy(p);
  return *this;
}

void
TimeControl::applyOptions(void)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i],"=");
     if( split.size() == 2 )
     {
        if( split[0] == "path" )
        {
          setFilename(split[1]);
          continue;
        }

        if( split[0][0] == 'e' )  // end
        {
          isTimeFrame=true;
          setEndDate(split[1]);
          continue;
        }

        if( split[0][0] == 'i' )  // increment
        {
          isBuildFileName=true;
          setIncrement(split[1]);
          continue;
        }

        if( split[0][0] == 'p' )
        {
          isBuildFileName=true;
          setFilenamePattern(split[1]);
          continue;
        }

        if( split[0][0] == 's' )  // start
        {
          setBeginDate(split[1]);
          continue;
        }
     }
     else
     {
        // don't expect assignment '='

        // special: [!]YrBeg!IncrementType!pattern
        // with separator '!', by default ','
        // Note: ':' not permitted.
        std::string sep(",");
        if( ! split.isNumber(0) )
        {
          sep = split[0][0];
          split.erase(0,0); // first char of first item
        }

        Split splitS(split[0], ",");
        setBeginDate(splitS[0]);

        if( splitS.size() == 3 )
        {
          setIncrement(splitS[1]);
          setFilenamePattern(splitS[2]);
          isBuildFileName=true;
        }
     }
   }

   return;
}

void
TimeControl::copy(const TimeControl &p)
{

   isBuildFileName=p.isBuildFileName;
   isTimeFrame=p.isTimeFrame;

   currDate=p.currDate;
   refDate=p.refDate;
   endDate=p.endDate;
   beginDate=p.beginDate;

   isBegin=p.isBegin;
   isEnd=p.isEnd;

   beginValue=p.beginValue;
   endValue=p.endValue;

   isTime=p.isTime;

   identNum=p.identNum;
   argCount=p.argCount;
   incrementPattern=p.incrementPattern;
   incrementValue=p.incrementValue;
   filenamePattern=p.filenamePattern;
   file=p.file;
   objName=p.objName;

   for( size_t i=0 ; i < optStr.size() ; ++i )
     optStr.push_back( p.optStr[i] );

   return;
}

void
TimeControl::exceptionError(std::string str)
{
  // Occurrence of an error usually stops the run at once.
  // But, the calling program unit is to exit.
  static bool doInit=true;
  if( doInit )
  {
    xcptn.strError = "tc_error" ;

    // base name if available, i.e. after the initialisation
    // of the InFile obj
/*
    if( str.size() > 0 )
    {
      xcptn.strError += "_";
      xcptn.strError += hdhC::getBasename(str) ;
    }
*/
    xcptn.strError += ".txt";

    doInit = false;
  }

  // open file for writing
  if( ! xcptn.ofsError )
    xcptn.ofsError = new std::ofstream(xcptn.strError.c_str());

  *xcptn.ofsError << str << std::endl;

  return ;
}

void
TimeControl::exceptionWarning(std::string str)
{
  // a warning does not stop the run

  static bool doInit=true;
  if( doInit )
  {
    // This happens only once.
    xcptn.strWarning = "tc_warning" ;

/*
    if( str.size() > 0 )
    {
      xcptn.strWarning += "_";
      xcptn.strWarning += hdhC::getBasename(str) ;
    }
*/
    xcptn.strWarning += ".txt";

    doInit = false;
  }

  // open file for writing
  if( ! xcptn.ofsWarning )
    xcptn.ofsWarning = new std::ofstream(xcptn.strWarning.c_str());

  if( xcptn.ofsWarning->is_open() )
  {
    // write message
    xcptn.ofsWarning->write(str.c_str(), str.size()+1);
    *xcptn.ofsWarning << std::endl;
  }

  return ;
}

void
TimeControl::finally(int errCode)
{
  if( errCode > 0)
    exit(errCode);

  return ;
}

size_t
TimeControl::findRecAtTime(std::string recVarName, double tLimit,
   char side)
{
  // the outer bounds
  size_t r0 = 0 ;
  size_t r1 = pIn->nc.getNumOfRecords() -1 ;
  size_t rm, rm_save;

  // read time from the ncFile
  double t0 = pIn->nc.getData(tmp_mv, recVarName, r0) ;

  double t1 = pIn->nc.getData(tmp_mv, recVarName, r1) ;
  double tm;

  // test the location of the window.
  if( isBegin && t0 >= tLimit )
  {
    // adjust: end the loop before it starts
    if( pIn->tC )
      pIn->setRecBeg(pIn->ncRecEnd);

    return 0;
  }

  // no adjustment: all records are valid
  if( t1 < tLimit )
    return 0;

  rm=r0;

  // Kind of nested intervals. Find the record left from the limit
  do
  {
    rm_save=rm;
    rm = (r0+r1)/2 ;
    tm = pIn->nc.getData(tmp_mv, recVarName, rm) ;

    if( tm < tLimit )
    {
      t0=tm;
      r0=rm;
    }
    else
    {
      t1=tm;
      r1=rm;
    }

  } while( rm_save != rm );

  // final determination of the record
  if( side == 'l' )  // left-hand side of the window
  {
    if( tm < tLimit )
      ++rm;

    // adjust
    if( pIn->tC )
      pIn->setRecBeg(rm);
  }
  else
  {
    //special: value at r1 matches the limit.
    if( (rm+1) == r1)
    {
      tm = pIn->nc.getData(tmp_mv, recVarName, r1) ;
      if( tm == tLimit )
        rm=r1;
    }

    // adjust
    if( pIn->tC )
      pIn->setRecEnd(rm+1);  // one more to point behind the window
  }

  return rm;
}

void
TimeControl::getCurrDate(NcAPI &nc, size_t rec)
{
  std::string t;

  if( isTime )
  {
    // read time from ncFile
    currValue = nc.getData(tmp_mv, "time", rec) ;
    currDate = refDate.getDate( currValue );
  }

  return ;
}

std::string
TimeControl::getFilename(void)
{
  // Determine the first or the next full-path filename.
  // Increments recursiveley, if the built filename does not exist.

  bool isAfterFirst = file.is() ;

  if( isAfterFirst )
  {
     // incrementation
     if( incrementPattern == "YYYY" )
        beginDate.addYears(incrementValue);
     else if( incrementPattern == "MM" )
        beginDate.addMonths(incrementValue);
     else if( incrementPattern == "DD" )
        beginDate.addDays(incrementValue);
     else if( incrementPattern == "hh" )
        beginDate.addHours(incrementValue);
     else if( incrementPattern == "mm" )
        beginDate.addMinutes(incrementValue);
     else if( incrementPattern == "ss" )
        beginDate.addSeconds(incrementValue);
  }

  setFilename(filenamePattern) ;
  size_t pos;

  std::string sDateStr(beginDate.str());
  if( isTimeFrame)
     std::string eDateStr(endDate.str());

  // find the patterns
  if( (pos=file.basename.find("YYYY")) < std::string::npos )
  {
     std::string s= hdhC::double2String(
         beginDate.getYear(),4,"4_0");
     file.basename.replace(pos, 4, s);
  }
  if( (pos=file.basename.find("MM")) < std::string::npos )
  {
     std::string s= hdhC::double2String(
         beginDate.getMonth(),2,"2_0");
     file.basename.replace(pos, 2, s);
  }
  if( (pos=file.basename.find("DD")) < std::string::npos )
  {
     std::string s= hdhC::double2String(
         beginDate.getDay(),2,"2_0");
     file.basename.replace(pos, 2, s);
  }
  if( (pos=file.basename.find("hh")) < std::string::npos )
  {
     std::string s= hdhC::double2String(
         beginDate.getHour(),2,"2_0");
     file.basename.replace(pos, 2, s);
  }
  if( (pos=file.basename.find("mm")) < std::string::npos )
  {
     std::string s= hdhC::double2String(
         beginDate.getMinute(),2,"2_0");
     file.basename.replace(pos, 2, s);
  }
  if( (pos=file.basename.find("ss")) < std::string::npos )
  {
     std::string s= hdhC::double2String(
         beginDate.getSecond(),2,"2_0");
     file.basename.replace(pos, 2, s);
  }

  sDateStr = beginDate.str();

  if( ! file.exist() )
  {
    if( isAfterFirst )  // no recursion, but stop
      return "";

    setFilename(getFilename()); // into the recursion
  }

  return file.getFile();
}

void
TimeControl::help(void)
{
  std::cerr << "Options of controlling time windows (indicated by TC)\n";
  std::cerr << "of Base derived classes in whose option string\n" ;
  std::cerr << "they may be embedded\n" ;
  std::cerr << "or connected there by explicit index.\n" ;
  std::cerr << "   e[ndDate]=string_accepted_by_Date_class\n" ;
  std::cerr << "   i[ncrementType]=figure\n" ;
  std::cerr << "   path=path-fo-files-with-pattern\n" ;
  std::cerr << "   p[attern]=string\n" ;
  std::cerr << "   s[tartDate]=string_accepted_by_Date_class\n" ;
  std::cerr << "   [!]year-to-begin!incrementType!pattern\n" ;
  std::cerr << "   Note: ! is any non-digit char (',' by default)\n";
  std::cerr << "   but, must neither be ':' nor\n";
  std::cerr << "   the separation character, if TC is embedded.\n" ;
  std::cerr << std::endl;
  return;
}

bool
TimeControl::init(void)
{
  setObjName("TC");

  notes=0;
  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qA=0;
  tC=0;

  applyOptions();

  // also recVarName gets the variable name representing the unlimited dim
  std::string sU(pIn->getTimeUnit( recVarName ));

  if( sU.size() )
  {
    isTime=true;
    setReferenceDate( sU );

    if( isBegin )
    {
      if( beginValue == MAXDOUBLE )
      {
        beginDate.setUnits( sU );
        beginValue = refDate.getSince(beginDate);
      }
      else
      {
        beginDate = refDate ;
        beginDate.addTime( hdhC::double2String(beginValue) );
      }
    }

    if( isEnd )
    {
      if( endValue == MAXDOUBLE )
      {
        // set by date
        endDate.setUnits( sU );
        endValue = refDate.getSince(endDate);
      }
      else
      {
        // set by time
        endDate = refDate ;
        endDate.addTime( hdhC::double2String(endValue) );
      }
    }
  }

  currRec=beginRec=0;

  // set a time limit to the left-side of the time-window
  if( isBegin )
  {
    // any adjustment of the record range in pIn is
    // made in the method
    (void) findRecAtTime(recVarName, beginValue, 'l');

/*
    for( ; currRec < inRecNum ; ++currRec )
    {
      // currValue is always one rec behind
      if( currValue >= beginValue)
      {
         beginRec = currRec-1;
         pIn->setCurrRec(beginRec);
         break;
      }

      // read time from the ncFile
      currValue = Base::getTime(pIn->nc, currRec, recVarName) ;
    }
*/
  }
/*
  else
    currValue = beginValue = Base::getTime(pIn->nc, currRec, recVarName);
*/

  // set a time limit to the right-side of the time-window
  if( isEnd )
  {
    // any adjustment of the record range in pIn is
    // made in the method
    (void) findRecAtTime(recVarName, endValue, 'r');

/*
    // synchronise to the beginning of the time window
    for( ; currRec < inRecNum ; ++currRec )
    {
      // currValue is always one rec behind
      if( currValue > endValue)
      {
         endRec = currRec-1;
         pIn->setRecEnd(endRec);
         break;
      }

      currValue = Base::getTime(pIn->nc, currRec, recVarName) ;
    }
*/
  }
/*
  else
    currValue = beginValue = Base::getTime(pIn->nc, currRec, recVarName);
*/

  // release the InFile from the TC object, because the latter is no
  // longer needed after resetting the record frame to the time window.
  if( pIn->tC )
    pIn->tC=0;

  //re-set the execution pointer from init to entry
  execPtr = &IObj::entry ;

  return false;
}

void
TimeControl::initDefaults(void)
{
  objName="TC";

  argCount=0;

  isBuildFileName=false;
  isTimeFrame=false;
  isTime=false;

  isBegin=false;
  isEnd=false;

  beginValue=MAXDOUBLE;
  endValue=MAXDOUBLE;

  xcptn.ofsError=0;
  xcptn.ofsWarning=0;

  // set pointer to member function init()
  execPtr = &IObj::init ;

  return;
}

void
TimeControl::linkObject(IObj *p)
{
  std::string className = p->getObjName();

  if( className == "X" )
    notes = dynamic_cast<Annotation*>(p) ;
  else if( className ==  "CF" )
    cF = dynamic_cast<CF*>(p) ;
  else if( className == "FD_interface" )
    fDI = dynamic_cast<FD_interface*>(p) ;
  else if( className ==  "IN" )
    pIn = dynamic_cast<InFile*>(p) ;
  else if( className == "Oper" )
    pOper = dynamic_cast<Oper*>(p) ;
  else if( className == "Out" )
    pOut = dynamic_cast<OutFile*>(p) ;
  else if( className == "QA" )
    qA = dynamic_cast<QA*>(p) ;
  else if( className == "TC" )
    tC = dynamic_cast<TimeControl*>(p) ;

  return;
}

void
TimeControl::setBeginDate(std::string s)
{
  isBegin=true;

  if( s[0] == 't' || s[0] == 'T' )
    beginValue = hdhC::string2Double(s.substr(1)) ;
  else
    beginDate=s;

  return ;
}

void
TimeControl::setEndDate(std::string s)
{
  isEnd=true;

  if( s[0] == 't' || s[0] == 'T' )
    endValue = hdhC::string2Double(s.substr(1)) ;
  else
    endDate=s;

  return;
}

void
TimeControl::setIncrement(std::string s)
{
  // default: 1

  size_t pos=s.find('=');
  if( pos == std::string::npos )
  {
    incrementPattern=s;
    incrementValue=1;
    return;
  }

  incrementPattern=s.substr(0,pos);
  incrementValue=
    hdhC::string2Double(s.substr(pos+1));

  return;
}

bool
TimeControl::syncEnd(double t)
{
  // only for explicit time frame
  if( ! isTimeFrame )
    return false;

//  bool is=(currDate > endDate) ? true : false ;
  bool is=(t > endValue) ? true : false ;

  return is ;
}

bool
TimeControl::syncEnd(Date&)
{
  // only for explicit time frame
  if( ! isTimeFrame )
    return false;

  bool is=(currDate > endDate) ? true : false ;

  return is ;
}
