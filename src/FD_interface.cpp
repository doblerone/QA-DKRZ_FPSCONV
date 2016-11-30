// #include "fd_interface.h"

FD_interface::FD_interface()
{
  initDefaults();

  return;
}

void
FD_interface::applyOptions(void)
{
//  Base::applyOptions();

  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i],"=");

     if( split.size() == 2 )
     {
        if( split[0][0] == 'w' || split[0] == "binWidth"
              || split[0] == "class_width"
                || split[0] == "classWidth"  )
        {
          binWidth=hdhC::string2Double(split[1]);
          isSetByOption=true;
          continue;
        }

        if( split[0][0] == 'c' || split[0][0] == 'C' )
        {
          isCentric=(split[1][0] == 't' || split[1][0] == 'T')
                         ? true : false ;
          continue;
        }

        if( split[0][0] == 'i' )
        {
          initValue= hdhC::string2Double(split[1]);
          isInitValue=true;
          continue;
        }

        if( split[0] == "isSetByOption" )
        {
           if( split[1] == "true" )
             isSetByOption=true;
          continue;
        }

        if( split[0] == "part" )
        {
          timeWindowWidth=split[1];
          continue;
        }

        if( split[0] == "readProperties"
              || split[0] == "rp"
                 || split[0] == "read_properties" )
        {
          isReadProperties=true;
          rebuildFilename=split[1];
          continue;
        }

        if( split[0] == "rebuild" || split[0][0] == 'r'
             || split[0][0] == 'R' )
        {
          rebuildFilename=split[1];
          continue;
        }

        if( split[0][0] == 's' || split[0][0] == 'S' )
        {
          isSaveBuild=true;
          setFilename( split[1]);
          continue;
        }
     }
     else
     {
        if( split.isNumber(0) )
          continue;

        if( split[0] == "disableAutomResize" )
        {
          isAutoResizeDisabled=true;
          continue;
        }

        if( split[0] == "printPlain" )
        {
          isPrintPlain=true;
          continue;
        }

        if( split[0] == "printBars" )
        {
          isPrintPlain=true;
          isPrintBars=true;
          continue;
        }

        if( split[0] == "stdout" )
        {
          isStdOut=true;
          continue;
        }

        if( split[0] == "useAreaWeight" )
        {
          isUseAreaWeight=true;  //default
          continue;
        }

        if( split[0] == "useFrequencyWeight" )
        {
          isUseFrequencyWeight=true;
          isUseAreaWeight=false;  // turn off the default
          continue;
        }

        // short key options
        if( split[0][0] == 's' || split[0][0] == 'S' )
        {
          isSaveBuild=true;
          continue;
        }
     }
   }

   if( rebuildFilename.size() && ! isSaveBuild )
   {
      isSaveBuild=true;
      setFilename(rebuildFilename.substr(0,rebuildFilename.size()-6));
   }

   return;
}

bool
FD_interface::entry(void)
{
//  MtrxArr<float>& v=pGM->getCellValue();
//  MtrxArr<double>& w=pGM->getCellWeight();
  MtrxArr<float> v;
  MtrxArr<double> w;

  // used in finally and here
  if( pIn->isTime )
    currTime = pIn->nc.getData(w, "time", pIn->currRec);

  if( timeWindowWidth.size() > 0 )
  {
    currDate = refDate.getDate(currTime) ;
    currDateStr = currDate.str();

    // If sub-division is enabled, then here is a reset of the window.
    // The FreqDist itself was already reset in print
    if( currDate > endDate )
    {
//      for(size_t i=0 ; i < fD.size() ; ++i)
//        fD[i].disableAutomResize(); // freeze setting from first file

       print();

       for( size_t i=0 ; i < fD.size() ; ++i )
         fD[i].reset();  // this is more than just clear

       // reset the time window
       (void) initTimeWindow();
    }
  }

  // is any filling value in current data set?
//  bool is = pGM->getFillingValueStatus();
  bool is=false;

  for( size_t i=0 ; i < fD.size() ; ++i )
  {
    if( is )
      fD[i].enableFillingValue();
    else
      fD[i].disableFillingValue();

    if( isUseAreaWeight )
    {
      fD[i].push_val(v, w);
      fD[i].addEnsembleCount();
    }
    else
      fD[i].push_val(v);
  }

  return false;
}

void
FD_interface::exceptionError(std::string str)
{
  // Occurrence of an error usually stops the run at once.
  // But, the calling program unit is due to exit.
  static bool doInit=true;
  if( doInit )
  {
    xcptn.strError = "fd_error" ;

     // base name if available, i.e. after the initialisation of the InFile obj
    if( file.is() )
    {
      xcptn.strError += "_";
      xcptn.strError += file.basename ;
    }
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
FD_interface::exceptionWarning(std::string str)
{
  // a warning does not stop the run

  static bool doInit=true;
  if( doInit )
  {
    // This happens only once.

    xcptn.strWarning = "fd_warning" ;

    if( file.is() )
    {
      xcptn.strWarning += "_";
      xcptn.strWarning += file.basename ;
    }
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
FD_interface::finally(int errCode)
{
  if( errCode > 0)
    exit(errCode);

   // build-output: header section: BEGIN, END
   for( size_t i=0 ; i < fD.size() ; ++i )
   {
       // adjust the info segment
       bool is;
       is = true;

       for( size_t j=0 ; j < fD[i].infoLine.size() ; ++j )
       {
         if( endDateStr.size() > 0 )
         {
           if( fD[i].infoLine[j].find("#BEGIN:") < std::string::npos )
           {
             fD[i].infoLine[j] = "#BEGIN: " ;
             fD[i].infoLine[j] += beginDateStr ;
           }
         }
         else
            if( fD[i].infoLine[j].find("#BEGIN:") < std::string::npos )
              fD[i].infoLine[j]="#BEGIN: " +fD[i].infoLine[j].substr(8);

         if( fD[i].infoLine[j].find("#END:") < std::string::npos )
         {
            is=false;
            fD[i].infoLine[j] = "#END: " ;
            currDate = refDate.getDate(currTime) ;
            fD[i].infoLine[j] +=  currDate.str()  ;
          }
       }

       if( is )
       {
          fD[i].infoLine.push_back( "#BEGIN: " ) ;
          if( pIn->isTime )
            fD[i].infoLine.back() += beginDate.str()  ;
          else
            fD[i].infoLine.back() += "entire experiment"  ;

          fD[i].infoLine.push_back( "#END: " ) ;
          if( pIn->isTime )
          {
            // no partitioning
            currDate = refDate.getDate(currTime) ;
            fD[i].infoLine.back() +=   currDate.str()  ;
          }
          else
            fD[i].infoLine.back() += "entire experiment"  ;
       }
   }

  // write output
  print();

  return;
}

void
FD_interface::help(void)
{
  std::cerr << "Option string for enabling frequency distributions:\n" ;
  std::cerr << "(may be embedded in option strings of Base derived\n" ;
  std::cerr << "classes indicated by 'FD'\n";
  std::cerr << "or connected to these by explicit index.\n" ;
  std::cerr << "   binWidth | w | class_width=value\n" ;
  std::cerr << "   c[entric]|C=true|false\n" ;
  std::cerr << "   disableAutomResize\n" ;
  std::cerr << "   init=value\n" ;
  std::cerr << "   isSetByOption=true|false\n" ;
  std::cerr << "   printPlain\n" ;
  std::cerr << "   printBars\n" ;
  std::cerr << "   rebuild|R=rebuild-filename\n" ;
  std::cerr << "   s|S[=save-to-filename] (else the default)\n" ;
  std::cerr << "   stdout\n" ;
  std::cerr << "   useAreaWeight\n" ;
  std::cerr << "   useFrequencyWeight\n" ;
  std::cerr << std::endl;
  return;
}

bool
FD_interface::init(void)
{
  setObjName("FD");

  notes=0;
  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qA=0;
  tC=0;

  // A freqDist is a vector of FreqDist objects, which  is for
  // regions.
  // There is no piping from *this object to an OutFile object.

  //re-set the execution pointer from init to entry
  execPtr = &IObj::entry ;

  // apply parsed command-line args
  applyOptions();

  // care for output filenames. If there have originally
  // been given multiple varnames (may be by default), then
  // only the first one will be effective and that one is
  // attached to filename. Notice that using multiple
  // varnames in a request (or by default) and using stdout
  // will result in a mess, because several obj write to stdout.

  if( file.is() )
    file.setFile( file.getFile() + "." + pIn->variable[0].name) ;
//  pGM = pIn->variable[0].pGM;


  std::string str;

  // A freqDist for the total domain (by default) or
  // any specified number of regions.
  // Index == 0 indicates the total domain.
  for( size_t i=0 ; i <= regioStr.size() ; ++i)
  {
    fD.push_back( FreqDist<float>() );
    fD.back().setOutputFilename( file.filename) ;
    if( isReadProperties )
      fD.back().setReadOnlyProperties();
  }

  if( pIn->isTime )
  {
    // date: begin and end
    str=pIn->getTimeUnit();

    // set reference date
    refDate.setDate( str );

    MtrxArr<double> mv_d;
    double startTime = pIn->nc.getData(mv_d, "time", pIn->currRec);

    beginDate = refDate.getDate(startTime) ;
    std::string cTime = beginDate.str();

    endDate = beginDate; //for time window partitioning
  }

  // Partitioning of the entire time span. Uses also filename
  // as basis. Resume a session, if build-file(s) are given.
  bool isInitTWResumed=false; // true, if a session build-file is resumed.

  if( timeWindowWidth.size() > 0 )
  {
    // return true, if a fd from file has to be resumed
    isInitTWResumed = initTimeWindow();
    isFirstInit=false;  // effective only if partitioning is enabled
  }
  else
  {
    // Resume any session? Yes? Return true and quit init
    if( rebuild( rebuildFilename ) )
    {
//      for(size_t i=0 ; i < fD.size() ; ++i)
//        fD[i].setFillingValue(
//           pIn->variable[0].pMeta->fillValue );
////        gD_VarUnlim[0].pRootIn->ncVarParam[0].fillValue );

      // this calls entry the first time for a resumed session
      return entry();
    }
  }

  if( isInitTWResumed )
  {
    // if an incomplete file for the current time window exists,
    // then disable.
    for(size_t i=0 ; i < fD.size() ; ++i)
       fD[i].disableAutomResize();
  }
  else
  {
    // for the start of a new time window or no window at all
    for(size_t i=0 ; i < fD.size() ; ++i)
    {
      str = "#FILE=" ;
      str += pIn->file.filename;
      fD[i].setInfo(str);

      str ="#VARIABLE=";
      str += vName;
      fD[i].setInfo(str);

      str="#UNIT=";
      str += pIn->variable[0].units;
      fD[i].setInfo(str);

//      if( pIn->variable[0].pMeta->isMissingValue )
//        fD[i].setFillingValue(
//           pIn->variable[0].pMeta->fillValue );
    }

    if( isSetByOption )
    {
       // properties were specified explicitly
       for(size_t i=0 ; i < fD.size() ; ++i)
         if( isInitValue )
           fD[i].setProperties(binWidth, initValue, isCentric);
         else
           fD[i].setProperties(binWidth, isCentric);

       for(size_t i=0 ; i < fD.size() ; ++i)
          fD[i].disableAutomResize();
    }
    else
      //
      for(size_t i=0 ; i < fD.size() ; ++i)
        if( ! isAutoResizeDisabled )
          fD[i].enableAutomResize();
  }

  // this calls entry the first time
  return entry();
}

void
FD_interface::initDefaults()
{
  objName="FD";

  isAutoResizeDisabled=false;
  isFirstInit=true;
  isInitValue=false;
  isCentric=true;
  isPrintPlain=false;
  isPrintBars=false;
  isReadProperties=false;
  isSaveBuild=false;
  isSetByOption=false;
  isStdOut=false;
  isUseAreaWeight=true;  // turn off the default
  isUseFrequencyWeight=false;
  binWidth=1.;
  initValue=0;

  xcptn.ofsError=0;
  xcptn.ofsWarning=0;

  // set pointer to member function init()
  execPtr = &IObj::init ;
}

bool
FD_interface::initTimeWindow(void)
{
   std::string str, str1;

// ++++++ Determine current dates +++++++++++++++++++++++
// Note: dates could be modified using information from a file
   // change of context

   if( ! isFirstInit )
      beginDate = endDate ;

   endDate.addTime( timeWindowWidth );

//   long double jDays = endDate.getJulianDate() ;
//   jDays -= beginDate.getJulianDate() ;

   // change of context
   beginDateStr = beginDate.str();
   endDateStr = endDate.str() ;

   str = file.basename;
   str = str.substr(3); // ignore leading three; no extension.

// ++++++ Looking for a file +++++++++++++++++++++++
   if( isFirstInit )  // only once for a programm run
   {
     // Any file available from a previous time interval?
     DIR *dir;
     struct dirent *file;
     dir=opendir(".") ;  // there was a cd in the qa_Executor script

     ReadLine rF;
     Date fBeginDate, fEndDate;
     bool isFound;

     // scan files of current directory
     while ( (file = readdir(dir)) )  // file == 0 at the end of dir
     {
       str1= file->d_name ;

       if( str1.substr(0,3) != "fd_" )
         continue;

       isFound=false;

       // Ignore leading three chars and trailing part; only use
       // those part of str that must be contained in str1.
       // Test whether str1 > 3 is not necessary here.
       // Note: 'qa_' was stripped off str
       std::string t = str1.substr(3, str.size());

       if( str1.size() > 6 && str == t )
       {
          // extract time interval from freqDist info segment
          rF.open(str1);
          while( ! rF.readLine() )
          {
             if( rF.at(0) != '#')
               continue;

             if( rF.getItem(0) == "#BEGIN:" )
             {
                fBeginDate = rF.getItem(1) ;

                // do not read the end date from file, but
                // calculate the end of the time window.
                fEndDate = fBeginDate ;
                fEndDate.addTime( timeWindowWidth );
                isFound=true;
                break;
             }
          }

          rF.close();

          // file with date found; now check overlap of dates
          if( isFound)
          {
             if( fEndDate <= beginDate )
               continue;  // try next file

             if( beginDate >= fBeginDate )
             {
                beginDate = fBeginDate;
                endDate = fEndDate;
                beginDateStr = fBeginDate.str();
                endDateStr = fEndDate.str();

                // resume previous, incomplete session
                if( rebuild( str1 ) )
                {
                  for( size_t i=0 ; i < fD.size() ; ++i )
                    fD[i].setOutputFilename( str1 );
                  return true;
                }
                else
                {
                   std::ostringstream ostr(std::ios::app);
                   ostr << "FD_interface::initTimeWindow()\n";
                   ostr << "Could not be rebuild" << str1;
                   exceptionError( ostr.str() );
                }
             } // else: error: variable time interval
          }
       }
     }
     closedir(dir);
   }
// ++++++ Looking for a file +++++++++++++++++++++++

   // compose output filename; use only integer years
   // ranging from year A inclusively and year B exclusively.
   str = file.basename;
   str += "_" ;

   str += beginDateStr.substr(0,4);
   str += "-" ;
   str += endDateStr.substr(0,4);

   for( size_t i=0 ; i < fD.size() ; ++i )
     fD[i].setOutputFilename( str );

   return false;  //
}

void
FD_interface::linkObject(IObj *p)
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
FD_interface::print(void)
{
  for( size_t j=0 ; j < fD.size() ; ++j )
  {
    if( isPrintPlain )
    {
      if( isPrintBars )
        fD[j].printHist();
      else
        fD[j].print();
    }

    if( isSaveBuild )
      fD[j].saveBuild();
  }
  return;
}

void
FD_interface::print(std::string fromDate,
   std::string toDate)
{
   std::string tRangeStr;

   if( fromDate.size() > 0 )
   {
     size_t pos;
     if( (pos = fromDate.find("T00:00:00")) < std::string::npos)
       fromDate=fromDate.substr(0,fromDate.size()-9);

     if( (pos = toDate.find("T00:00:00")) < std::string::npos)
       toDate=toDate.substr(0,toDate.size()-9);

     tRangeStr=".";
     tRangeStr+=fromDate + "_";
     tRangeStr+=toDate;
   }

   std::string str;

   if( file.is() )
     str=file.filename;
   else
     str="frequencyDist." + vName;

   // prop file is only saved once.
   fD[0].saveProperties(str+".fD_prop");

   if( regioStr.size() > 0 )
     str += "." ;  // prepare for the regio extension in filename

   std::string s0(str);

   for( size_t i=0 ; i < fD.size() ; ++i)
   {
     if( regioStr.size() > 0 )
       s0=str+regioStr[i];

     if( tRangeStr.size() > 0 )
       s0+=tRangeStr;

     fD[i].saveBuild(s0+".fD_build");
     fD[i].reset();
   }

   return;
}

void
FD_interface::pushFreqDist(size_t seas)
{
 /*
  std::vector<double> var ;
  std::vector<double> area ;

  size_t num=1; //default for total domain
  if( regioStr.size() > 0 )
    num=regioStr.size();
*/

  return ;
}

bool
FD_interface::rebuild( std::string f)
{
  rebuildFilename = f;

  // some priority rules:
  // 1) build-file is named and exists --> use it
  // 2) build-file is named and does not exist,
  //    a) but, a file  named "frequencyDist.[gD_VarUnlim.name].prop"
  //       is in the path, then use it
  //    b) but, a file  named "frequencyDist.prop" is
  //       in the path, then use it
  // 3) a build-file is named and does not exist nor does
  //    "frequency.prop", but properties
  //    are given by options, then use the options
  // 4) no file is named but properties by options
  // 5) no file is named and no properties, then the properties
  //    are determined automatically and the build is saved,
  //    except 'isAutoResizeDisabled' was set true.

  // Notice that system() returns 0 i.e. false for 'file found'
  if( rebuildFilename.size() > 0 )
  {
     std::string s="\bin\bash -c \'ls ";
     s += rebuildFilename ;
     s += " >/dev/null 2>&1\'" ;

     if( ! system( s.c_str() ) )
     {
       for(size_t i=0 ; i < fD.size() ; ++i)
         fD[i].addBuild( rebuildFilename ) ;  // rule 1)
     }
     else
     {
       s="/bin/bash -c \'ls frequencyDist.";
       s += vName + ".prop >/dev/null 2>&1\'";
       if( ! system( s.c_str() ) )
       {
         s="frequencyDist." ;
         s+=vName + ".prop";
         for(size_t i=0 ; i < fD.size() ; ++i)
           fD[i].addBuild( s.c_str() ); // rule 2a)
       }
       else
       {
         s="/bin/bash -c \'ls frequencyDist.prop >/dev/null 2>&1\'";
         if( ! system( s.c_str() ) )
         {
           for(size_t i=0 ; i < fD.size() ; ++i)
             fD[i].addBuild( "frequencyDist.prop" ); // rule 2b)
         }
       }
     }
     return true;
  }

  return false;  // no rebuild
}

