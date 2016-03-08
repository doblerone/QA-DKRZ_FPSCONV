//#include "qa.h"

// Macro option to enable output of all messages.
// Please compile with '-D RAISE_ERRORS'

QA::QA()
{
  initDefaults();
}

QA::~QA()
{
  if( nc )
  {
    nc->close();
   delete nc ;
    nc=0;
  }
}

void
QA::appendToHistory(void)
{
  // date and time at run time
  std::string today( Date::getTodayStr() );
  today += ":";

  std::string s;
  std::string hst;
  std::string hstVersion;

  s = nc->getAttString("history");
  if( s.size() )
  {
    // append to history

    // check for a changed path to data
    Split splt(s);
    if( splt.size() > 2 && splt[1] == "path_to_data:" )
    {
      std::string hstPath(splt[2]);
      size_t pos;
      if( (pos=hstPath.rfind("\n")) < std::string::npos )
         hstPath.erase(pos,1);
      if( qaFile.path != hstPath )
      {
        hst += "\n" ;
        hst += today;
        hst += " changed path to data=";
        hst += qaFile.path + "\n" ;
      }
    }
  }
  else
  {
    // the root of the history string
    hst += today;
    hst += " path_to_data=";
    hst += qaFile.path ;
  }

  // did the package version change? Yes? Then add to history
  if( s.size() && enableVersionInHistory )
  {
    // look for the last version file named in the historys string,
    // thus a reversed scan for the first version element
    Split splt(s);
    size_t index=splt.size()-1;
    for( ; ; --index )
    {
      if( splt[index] == "revision:" )
        break;

      // note: i==0; --i; i== 4294967295
      if( index == 0 )
        break;
    }

    std::string tmp ;
    if( index > 0 )
      // version number in the next position
      tmp = splt[++index] ;
    else
      // not in the history, so take the one from the version attribute
      tmp = nc->getAttString("QA revision");

    if( revision != tmp )
    {
      hst += "\n" ;
      hst += today;
      hst += " changed QA revision=" ;
      hst += revision ;
    }
  }

  if( hst.size() )
  {
    hst = s + hst ;
    nc->setGlobalAtt( "history", hst);
  }

  return;
}

void
QA::applyOptions(bool isPost)
{
  enablePostProc=isPost;

  // the first loop for items with higher precedence
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "pP" || split[0] == "postProc"
       || split[0] == "post_proc")
     {
       enablePostProc=true;
       break;
     }
  }

  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "cM"
       || split[0] == "checkMode"
         || split[0] == "check_mode" )
     {
       if( split.size() == 2 )
         setCheckMode(split[1]);
        continue;
     }

     if( split[0] == "dIP"
          || split[0] == "dataInProduction"
              || split[0] == "data_in_production" )
     {
        // effects completeness test in testPeriod()
        isFileComplete=false;
        continue;
     }

     if( split[0] == "dP"
          || split[0] == "dataPath" || split[0] == "data_path" )
     {
        if( split.size() == 2 )
          // path to the directory where the execution takes place
          qaFile.setPath(split[1]);

        continue;
     }

     if( split[0] == "f" )
     {
       if( split.size() == 2 )
          qaFile.setFile(split[1]);
        continue;
     }

     if( split[0] == "fS" || split[0] == "FileSequence"
         || split[0] == "file_sequence" )
     {
        if( split.size() == 2 )
          fileSequenceState=split[1][0];

        continue;
     }

     if( split[0] == "oT"
           || split[0] == "outlierTest"
                || split[0] == "outlier_test" )
     {
        if( enablePostProc && split.size() == 2 )
          outlierOpts.push_back(split[1]);

        continue;
     }

     if( split[0] == "qNF" || split[0] == "qaNcfileFlags"
       || split[0] == "qa_ncfile_flags" )
     {
       if( split.size() == 2 )
          qaNcfileFlags=split[1];
        continue;
     }

     if( split[0] == "rR"
            || split[0] == "replicatedRecord"
                 || split[0] == "replicated_record" )
     {
       if( split.size() > 1 )
       {
          // input could be: ..., only_groups=num
          // then num would be in split[2]
          std::string tmp( split[1] );
          if( split.size() == 3 )
          {
            tmp += '=' ;
            tmp += split[2] ;
          }

          Split csv ;
          csv.setSeparator(",");
          BraceOP groups(tmp);
          while ( groups.next(tmp) )
          {
            csv = tmp;
            for( size_t i=0 ; i < csv.size() ; ++i )
              replicationOpts.push_back(csv[i]);
          }

          continue;
       }
     }
   }

   return;
}

bool
QA::checkDataBody(std::string vName)
{
  // any empty data segments?

  if( vName.size() )
  {
    for( size_t i=0 ; i < pIn->varSz ; ++i)
    {
      Variable& var = pIn->variable[i];
      if( var.name == vName )
        return pIn->nc.isEmptyData(var.name) ;
    }
  }
  else if( ! pIn->dataVarIndex.size() )
    return true;

  std::vector<size_t> vs;

  for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i)
  {
    Variable& var = pIn->variable[ pIn->dataVarIndex[i] ];
    if( ! pIn->nc.isEmptyData(var.name) )
    {
      vs.push_back(pIn->dataVarIndex[i]) ;
      notes->setCheckDataStr(fail);
    }
  }

  if( vs.size() )
    // only try for variables with data
    pIn->dataVarIndex = vs ;
  else
    // all data variables without any data
    return true;

  return false;
}

void
QA::checkConsistency(InFile &in, std::vector<std::string> &opt,
                     std::string& tPath)
{
  defaultPrjTableName() ;

  // Read or write the project table.
  Consistency consistency(this, &in, opt, tPath);

  if( !consistency.isEnabled() )
    return;

  consistency.setAnnotation(notes);
  consistency.setExcludedAttributes( excludedAttribute );

  consistency.check();

  // inquire whether the meta-data checks passed
  int ev;
  if( (ev = notes->getExitValue()) > 1 )
    setExit( ev );

  return ;
}

void
QA::closeEntry(void)
{
   if( isCheckData )
   {
     // data: structure defined in hdhC.h
     for( size_t i=0 ; i < qaExp.varMeDa.size() ; ++i )
     {
       if( qaExp.varMeDa[i].var->isNoData )
          continue;

       // skip time test for proceeding time steps when var is fixed
       if( isNotFirstRecord && qaExp.varMeDa[i].var->isFixed  )
         continue;

       hdhC::FieldData fA( qaExp.varMeDa[i].var->pDS->get() ) ;

       // test overflow of ranges specified in a table, or
       // plausibility of the extrema.
       qaExp.varMeDa[i].qaData.test(i, fA);

       storeData(qaExp.varMeDa[i], fA);
     }
   }

   // This here is only for the regular QA time series file
   if( qaTime.isTime )
     storeTime();

   ++currQARec;

   return;
}

void
QA::defaultPrjTableName(void)
{
  // tables names usage: both project and standard tables
  // reside in the same path.
  // Naming of the project table:
  if( ! consistencyFile.is )
    consistencyFile.setFilename("pt_NONE.csv");

  return;
}

bool
QA::entry(void)
{
   if( isCheckData )
   {
     // read next field
     pIn->entry() ;

     for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i)
     {
       Variable &var = pIn->variable[pIn->dataVarIndex[i]];

       if( var.isNoData )
          continue;

       if( pIn->currRec && var.isFixed )
         continue;

       var.pDS->clear();

       if( var.isArithmeticMean )
         // use the entire record
         var.pDS->add( (void*) var.pMA );
       else
       {
         // loop over the layers and there should be at least one.
         var.pGM->resetLayer();
         do
         {
           // due to area weighted statistics, we have to cycle
           // through the layers one by one.
//         if( var.pGM->isLayerMutable() )
//           ;

           var.pDS->add( var.pGM->getCellValue(),
                       *var.pGM->getCellWeight() );
           // break the while loop, if the last layer was processed.
         } while ( var.pGM->cycleLayers() ) ; //cycle through levels
       }
     }
   }

   // the final for this record
   closeEntry();
   return false;
}

int
QA::finally(int xCode)
{
  if( nc )
    xCode = finally_data(xCode) ;

  if( xCode != 63 && qaTime.isTime )
    qaTime.finally( nc );

  setExit(xCode);

  // distinguish from a sytem crash (segmentation error)
  std::cout << "STATUS-BEG" << xCode << "STATUS-END";
  std::cout << std::flush;

  nc->close();

  return exitCode ;
}

int
QA::finally_data(int xCode)
{
  setExit(xCode);

  // write pending results to qa-file.nc. Modes are considered there
  for( size_t i=0 ; i < qaExp.varMeDa.size() ; ++i )
    setExit( qaExp.varMeDa[i].finally() );

  // post processing, but not for conditions which indicate
  // incomplete checking.
  if( exitCode < 3 )
  {  // 3 or 4 interrupted any checking
    if( enablePostProc )
    {
      if( postProc() )
      {
        if( exitCode == 63 )
            exitCode=1;  // this is considered a change

        notes->setCheckDataStr(fail);
      }
    }
  }

  if( exitCode == 63 ||
     ( nc == 0 && exitCode ) || (currQARec == 0 && pIn->isTime ) )
  { // qa is up-to-date or a forced exit right from the start;
    // no data to write
    if( exitCode == 63 )
      exitCode=0 ;

    isExit=true;
    return exitCode ;
  }

  // read history from the qa-file.nc and append new entries
  appendToHistory();

  // check for flags concerning the total data set,
  // but exclude the case of no record
  if( pIn->currRec > 0 )
    for( size_t j=0 ; j < qaExp.varMeDa.size() ; ++j )
      qaExp.varMeDa[j].qaData.checkFinally(qaExp.varMeDa[j].var);

  if( isCheckData )
  {
    for( size_t j=0 ; j < qaExp.varMeDa.size() ; ++j )
    {
       // write qa-results attributes about statistics
       qaExp.varMeDa[j].qaData.setStatisticsAttribute(nc);

       // plausibility range checks about units
       qaExp.varMeDa[j].verifyPercent();
    }
  }

  return exitCode ;
}

bool
QA::getExit(void)
{
  // note that isExit==true was forced
  if( exitCode > 1 || isExit )
    return true;

  return false;
}


void
QA::help(void)
{
  std::cerr << "Option string of the quality control class QA:\n" ;
  std::cerr << "(may be embedded in option strings of Base derived\n" ;
  std::cerr << "classes)\n";
  std::cerr << "or connected to these by explicit index 'QA0...'.\n" ;
  std::cerr << "   checkTimeBounds\n" ;
  std::cerr << "   noCalendar\n" ;
  std::cerr << "   printASCII (disables writing to netCDF file.\n" ;
  std::cerr << "   printTimeBoundDates\n" ;
  std::cerr << std::endl;

  return;
}

bool
QA::init(void)
{
   // Open the qa-result.nc file, when available or create
   // it from scratch. Meta data checks are performed.
   // Initialisation of time and time boundary testing.
   // Eventually, entry() is called to test the data of fields.

   notes->init();  // safe

   qaExp.setParent(this);
   qaTime.setParent(this);

   // default for the qaFile
   setFilename( pIn->file );

   // apply parsed command-line args
   applyOptions();

   // exclude user-defined data variables from any checking
   pIn->excludeVars();

   qaTime.init(optStr);

   qaExp.init(optStr);

   // DRS and CV specifications
   drs_cv_table.setParent(this);
   drs_cv_table.setPath(tablePath);
   drs_cv_table.applyOptions(optStr);
   drs_cv_table.read();

   // experiment specific obj: set parent, pass over options
   qaExp.run();

   // check existance of any data at all
   if( (qaTime.isTime || isCheckData )
            && pIn->ncRecBeg == 0 && pIn->ncRecEnd == 0 )
   {
      isCheckData=false;

      std::string key("6_15");
      if( notes->inq( key, fileStr) )
      {
        std::string capt("No records in the file") ;

        if( notes->operate(capt) )
        {
          notes->setCheckMetaStr( fail );
          notes->setCheckTimeStr( fail );
          notes->setCheckDataStr( fail );
          setExit( notes->getExitValue() ) ;
        }
      }
   }

   notes->setConstraintFreq( qaExp.getFrequency() );

   // enable detection of outlier and replicated records
   setProcessing();

   // open netCDF for creating, continuation or resuming qa_<varname>.nc.
   // note that this must happen before checkMetaData which uses currQARec
   openQA_Nc(*pIn);

   if( getExit() || qaExp.isUseStrict || qaTime.isNoProgress )
   {
     isCheckData=false;
     return true;
   }

   // check consistency between sub-sequent files or experiments
   checkConsistency(*pIn, optStr, tablePath) ;

   if( !isCheckTime )
     notes->setCheckTimeStr("OMIT");
   else if( qaTime.isTime && checkDataBody(qaTime.name) )
   {
     // time is defined, but there is no data
     qaTime.isTime = false;
     notes->setCheckTimeStr(fail);
   }
   else if( ! qaTime.isTime )
     notes->setCheckTimeStr("FIXED");
   else
     notes->setCheckTimeStr("PASS");

   if( isCheckData )
   {
     if( checkDataBody() )
     {
       notes->setCheckDataStr(fail);
       return true;
     }

     notes->setCheckDataStr("PASS");

     // set pointer to function for operating tests
     execPtr = &IObj::entry ;
     bool is = entry();

     if( getExit() || is )
       return true;

     isNotFirstRecord = true;
     return false;
   }

   return true;  // only meta-data check
}

void
QA::initDataOutputBuffer(void)
{
  if( qaTime.isTime )
  {
    qaTime.timeOutputBuffer.initBuffer(this, currQARec, bufferSize);
    qaTime.sharedRecordFlag.initBuffer(this, currQARec, bufferSize);
  }

  if( isCheckData )
    qaExp.initDataOutputBuffer();

  return;
}

void
QA::initDefaults(void)
{
  setObjName("QA");

  // pre-setting of some pointers
  nc=0;
  notes=0;

  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qA=0;
  tC=0;

  // time steps are regular. Unsharp logic (i.e. month
  // Jan=31, Feb=2? days is ok, but also numerical noise).

  fail="FAIL";
  fileStr="file";
  notAvailable="not available";
  s_global="global";
  s_mismatch="mismatch";

  n_axis="axis";
  n_cell_methods="cell_methods";
  n_long_name="long_name";
  n_outputVarName="output variable name";
  n_positive="positive";
  n_standard_name="standard_name";
  n_units="units";

  enablePostProc=false;
  enableVersionInHistory=true;

  isExit=false;
  isFileComplete=true;
  isFirstFile=false;
  isNotFirstRecord=false;
  isResumeSession=false;

  nextRecords=0;  //see init()

  importedRecFromPrevQA=0; // initial #rec in out-nc-file.
//  currQARec=UINT_MAX;
  currQARec=0;

  // pre-set check-modes: all are used by default
  isCheckMeta=true;
  isCheckTime=true;
  isCheckData=true;

  bufferSize=1500;

  exitCode=0;

  // file sequence: f[first], s[equence], l[ast]
  fileSequenceState='x' ;  // 'x':= undefined

#ifdef REVISION
  // -1 by default
  revision=hdhC::double2String(static_cast<int>(REVISION));
#endif

  // set pointer to member function init()
  execPtr = &IObj::init ;
}

void
QA::initGlobalAtts(InFile &in)
{
  // global atts at creation.
  std::string today( Date::getTodayStr() );

  nc->setGlobalAtt( "project", "CORDEX");
  nc->setGlobalAtt( "product", "quality check of CORDEX data set");

  nc->setGlobalAtt( "QA revision", revision);
  nc->setGlobalAtt( "contact", "hollweg@dkrz.de");

#ifndef NONE
  std::string t("csv formatted ");
  t += qaExp.varReqTable.basename ;
  nc->setGlobalAtt( "standard_table", t);
#endif

  nc->setGlobalAtt( "creation_date", today);

  // helper vector
  std::vector<std::string> vs;

  for( size_t m=0 ; m < qaExp.varMeDa.size() ; ++m )
    vs.push_back( qaExp.varMeDa[m].var->name );

  nc->copyAtts(in.nc, "NC_GLOBAL", &vs);

  return;
}

void
QA::initResumeSession(void)
{
  // this method may be used for two different purposes. First,
  // resuming within the same experiment. Second, continuation
  // from a parent experiment.

  // At first, a check over the ensemble of variables.
  // Get the name of the variable(s) used before
  // (only those with a trailing '_ave').
  std::vector<std::string> vss( nc->getVarName() ) ;

  std::vector<std::string> prevTargets ;

  size_t pos;
  for( size_t i=0 ; i < vss.size() ; ++i )
     if( (pos = vss[i].find("_ave")) < std::string::npos )
       prevTargets.push_back( vss[i].substr(0, pos) );

  // experiment specific
  qaExp.initResumeSession(prevTargets);

  // time specific
  qaTime.timeOutputBuffer.setNextFlushBeg(currQARec);
  qaTime.setNextFlushBeg(currQARec);

  if( qaTime.isTime )
    qaTime.initResumeSession();

  return;
}

void
QA::linkObject(IObj *p)
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

/*
bool
QA::locate( GeoData<float> *gd, double *alat, double *alon, const char* crit )
{
  std::string str(crit);

// This is a TxOxDxO; but not needed for QA
  MtrxArr<float> &va=gd->getCellValue();
  MtrxArr<double> &wa=gd->getCellWeight();

  size_t i=0;
  double val;  // store extrem value
  int ei=-1;   // store index of extreme

  // find first valid value for initialisation
  for( ; i < va.size() ; ++i)
  {
      if( wa[i] == 0. )
        continue;

      val= va[i];
      ei=i;
      break;
  }

  // no value found
  if( ei < 0 )
  {
    *alat = 999.;
    *alon = 999.;
    return true;
  }

  if( str == "min" )
  {
    for( ; i < va.size() ; ++i)
    {
        if( wa[i] == 0. )
          continue;

        if( va[i] < val )
        {
          val= va[i];
          ei=i;
        }
    }
  }
  else if( str == "max" )
  {
    for( ; i < va.size() ; ++i)
    {
        if( wa[i] == 0. )
          continue;

        if( va[i] > val )
        {
          val= va[i];
          ei=i;
        }
    }
  }

  *alat = gd->getCellLatitude(static_cast<size_t>(ei) );
  *alon = gd->getCellLongitude(static_cast<size_t>(ei) );

  return false;
}
*/


void
QA::openQA_Nc(InFile &in)
{
  // Generates a new nc file for QA results or
  // opens an existing one for appending data.
  // Copies time variable from input-nc file.

  // name of the result file was set before
  if ( !qaFile.is )
  {
    std::string key("00");

    if( notes->inq( key) )
    {
      std::string capt("openQA_Nc(): undefined file.") ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
      return;
    }
  }

  nc = new NcAPI;
  if( notes )
    nc->setNotes(notes);

  // don't create a netCDF file, when only meta data are to be checked.
  // but, NcAPI object nc must exist.
  if( ! (isCheckTime || isCheckData)  )
    return;

  if( nc->open(qaFile.getFile(), "NC_WRITE", false) )
  {
    // continue a previous session
    importedRecFromPrevQA=nc->getNumOfRecords();
    currQARec += importedRecFromPrevQA;
    if( currQARec )
      isNotFirstRecord = true;

    initDataOutputBuffer();

    initResumeSession();
    isResumeSession=true;

    return;
  }

  // So, we have to generate a netCDF file from almost scratch;
  isFirstFile=true;

  if( currQARec == 0 && in.nc.getNumOfRecords() == 1 )
    qaTime.isSingleTimeValue = true;

  // open new netcdf file
  if( qaNcfileFlags.size() )
    nc->create(qaFile.getFile(),  qaNcfileFlags);
  else
    nc->create(qaFile.getFile(),  "NC_NETCDF4");

  if( qaTime.time_ix > -1 )
    qaTime.openQA_NcContrib(nc);
  else
  {
    // dimensions
    qaTime.name="fixed";
    nc->defineDim("fixed", 1);
  }

    // create variable for the data statistics etc.
  for( size_t m=0 ; m < qaExp.varMeDa.size() ; ++m )
    qaExp.varMeDa[m].qaData.openQA_NcContrib(nc, qaExp.varMeDa[m].var);

  // global atts at creation.
  initGlobalAtts(in);

  initDataOutputBuffer();

  return;
}

bool
QA::postProc(void)
{
  bool retCode=false;

  if( ! isCheckData )
    return retCode;

  if( postProc_outlierTest() )
     retCode=true;

  return retCode;

}

bool
QA::postProc_outlierTest(void)
{
  bool retCode=false;

  for( size_t i=0 ; i < qaExp.varMeDa.size() ; ++i )
  {
     VariableMetaData &vMD = qaExp.varMeDa[i] ;

     if( ! vMD.qaData.enableOutlierTest )
        continue ;

     if( ! vMD.qaData.statAve.getSampleSize()  )
     {
       // compatibility mode: determine the statistics of old-fashioned results
       // note: the qa_<variable>.nc file becomes updated
       std::vector<std::string> vars ;
       std::string statStr;

       size_t recNum=nc->getNumOfRecords(true); // force new inquiry
       double val;
       std::vector<double> dv;
       MtrxArr<double> mv;

       if( vMD.var->isNoData )
         continue;

       vars.clear();

       // post-processing of data before statistics was vMD inherent?
       vars.push_back( vMD.var->name + "_min" );
       vars.push_back( vMD.var->name + "_max" );
       vars.push_back( vMD.var->name + "_ave" );
       vars.push_back( vMD.var->name + "_std_dev" );

       bool is=false;
       bool is2=true;
       for( size_t j=0 ; j < vars.size() ; ++j )
       {
         // read the statistics
         nc->getAttValues( dv, "valid_range", vars[j]);
         if( dv[0] == MAXDOUBLE )
         {
            is=true; // no valid statistics
            break;
         }
         statStr  ="sampleMin=" ;
         statStr += hdhC::double2String( dv[0] );
         statStr +=", sampleMax=" ;
         statStr += hdhC::double2String( dv[1] );
         statStr += ", ";
         statStr += nc->getAttString("statistics", vars[j], is2) ;
         if( ! is2 )
         {
            is=true; // no valid statistics
            break;
         }

         if( j == 0 )
           vMD.qaData.statMin.setSampleProperties( statStr );
         else if( j == 1 )
           vMD.qaData.statMax.setSampleProperties( statStr );
         else if( j == 2 )
           vMD.qaData.statAve.setSampleProperties( statStr );
         else if( j == 3 )
           vMD.qaData.statStdDev.setSampleProperties( statStr );

         is=false;
       }

       if( is )
       {
         val = nc->getData(mv, vars[0], 0);
         if( val < MAXDOUBLE )
         {
           // build the statistics from scratch
           int nRecs = static_cast<int>(recNum);

           // constrain memory allocation to a buffer size
           int sz_max=1000;
           size_t start[] = {0};
           size_t count[] = {0};

           // buffer for netCDF data storage
           double vals_min[sz_max];
           double vals_max[sz_max];
           double vals_ave[sz_max];
           double vals_std_dev[sz_max];

           // read data from file, chunk by chunk
           while (nRecs > 0)
           {
             int sz = nRecs > sz_max ? sz_max : nRecs;
             nRecs -= sz;

             count[0]=static_cast<size_t>(sz);

             nc_get_vara_double(
                nc->getNcid(), nc->getVarID(vars[0]),
                   start, count, vals_min );
             nc_get_vara_double(
                nc->getNcid(), nc->getVarID(vars[1]),
                   start, count, vals_max );
             nc_get_vara_double(
                nc->getNcid(), nc->getVarID(vars[2]),
                   start, count, vals_ave );
             nc_get_vara_double(
                nc->getNcid(), nc->getVarID(vars[3]),
                   start, count, vals_std_dev );

             start[0] += static_cast<size_t>(sz) ;

// vals_max[10]=400.;
// vals_min[10]=150.;
             // feed data to the statistics
             vMD.qaData.statMin.add( vals_min, sz );
             vMD.qaData.statMax.add( vals_max, sz );
             vMD.qaData.statAve.add( vals_ave, sz );
             vMD.qaData.statStdDev.add( vals_std_dev, sz );
           }

           // write statistics attributes
           vMD.qaData.setStatisticsAttribute(nc);
         }
       }
     }

     // the test: regular post-processing on the basis of stored statistics
     if( vMD.qaData.outlier->test(&vMD.qaData) )
       retCode = true;
  }

  return retCode;
}

void
QA::setCheckMode(std::string m)
{
  isCheckMeta=false;
  isCheckTime=false;
  isCheckData=false;

  Split cvs(m, ',');
  for( size_t j=0 ; j < cvs.size() ; ++j )
  {
    if( cvs[j] == "meta" )
      isCheckMeta=true ;
    else if( cvs[j] == "time" )
      isCheckTime=true ;
    else if( cvs[j] == "data" )
      isCheckData=true ;
  }

  return;
}

void
QA::setExit( int e )
{
  if( e > exitCode )
    exitCode=e;

  return ;
}

void
QA::setFilename(hdhC::FileSplit& fC)
{
  std::string f(fC.basename);

  Split x(f, '_');

  if( x.size() )
  {
    Split y(x[x.size()-1], '-');

    size_t sz = y.size();

    if( sz == 2 &&
          hdhC::isDigit( y[0]) && hdhC::isDigit( y[1]) )
    {
      f = "qa";

      for( size_t i=0 ; i < x.size()-1 ; ++i )
        f += "_" + x[i] ;
    }
  }

  qaFile.setFilename(f + ".nc");

  return ;
}

void
QA::setProcessing(void)
{
  for( size_t i=0 ; i < qaExp.varMeDa.size() ; ++i )
  {
    VariableMetaData &vMD = qaExp.varMeDa[i] ;

    if( replicationOpts.size() )
    {
      if( ReplicatedRecord::isSelected( *(vMD.var),
              replicationOpts, qaTime.name ) )
      {
        vMD.qaData.replicated =
            new ReplicatedRecord(this, i, vMD.var->name);
        vMD.qaData.replicated->setAnnotation(notes);
        vMD.qaData.replicated->parseOption(replicationOpts) ;
      }
    }

    if( enablePostProc && outlierOpts.size() )
    {
      if( Outlier::isSelected( *(vMD.var),
              outlierOpts, qaTime.name ) )
      {
        vMD.qaData.enableOutlierTest=true;

        vMD.qaData.outlier = new Outlier(this, i, vMD.var->name);
        vMD.qaData.outlier->setAnnotation(notes);
        vMD.qaData.outlier->parseOption(outlierOpts);
      }
    }
  }

  return;
}

void
QA::storeData(VariableMetaData& vMD, hdhC::FieldData& fA)
{
  //FieldData structure defined in geoData.h

    if( vMD.var->isNoData )
      return;

    if( isNotFirstRecord && vMD.var->isFixed  )
      return;

    vMD.qaData.store(fA) ;

  return ;
}

void
QA::storeTime(void)
{
   // testing time steps and bound (if any)
   qaTime.testDate(pIn->nc);

   qaTime.timeOutputBuffer.store(qaTime.currTimeValue, qaTime.currTimeStep);
   qaTime.sharedRecordFlag.store();

   return ;
}

void
DRS_CV_Table::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "tCV"
           || split[0] == "tableControlledVocabulary"
                || split[0] == "table_controlled_vocabulary" )
     {
       if( split.size() == 2 )
          table_DRS_CV.setFile(split[1]) ;

       continue;
     }

     if( split[0] == "tP"
          || split[0] == "tablePath" || split[0] == "table_path")
     {
       if( split.size() == 2 )
          tablePath=split[1];

       continue;
     }

   }

   if( table_DRS_CV.path.size() == 0 )
      table_DRS_CV.setPath(tablePath);

   return;
}

void
DRS_CV_Table::read(void)
{
  hdhC::FileSplit& fs = table_DRS_CV;

  ReadLine ifs(fs.getFile());

  if( ! ifs.isOpen() )
  {
     std::string key("7_1");
     if( pQA->notes->inq( key) )
     {
        std::string capt("no path to a table, tried " + fs.getFile()) ;

        if( notes->operate(capt) )
        {
          pQA->notes->setCheckMetaStr( pQA->fail );
          pQA->setExit( notes->getExitValue() ) ;
        }
     }

     return;
  }

  std::string s0;
  std::vector<std::string> vs;

  // parse table; trailing ':' indicates variable or 'global'
  ifs.skipWhiteLines();
  ifs.skipComment();
  ifs.skipCharacter("<>");
  ifs.clearSurroundingSpaces();

  Split x_item;
  x_item.setSeparator("=");
  x_item.setStripSides(" ");

  bool isDS=false;
  bool isFE=false;

  while( ! ifs.getLine(s0) )
  {
    if( s0.size() == 0 )
      continue;

    if( s0 == "BEGIN: DRS" )
    {
      while( ! ifs.getLine(s0) )
      {
        size_t pos;
        if( s0 == "END: DRS" )
        {
          isDS=false;
          isFE=false;
          break;
        }
        else if( (pos=s0.find(':')) < std::string::npos )
        {
          if( s0.substr(pos-3, 4) == "-DS:" )
          {
            isDS=true;
            isFE=false;
            pathEncodingName.push_back(s0.substr(0, pos-3));
            pathEncoding.push_back(
              hdhC::stripSides( s0.substr(++pos)) );
          }
          else if( s0.substr(pos-3, 4) == "-FE:" )
          {
            isDS=false;
            isFE=true;
            fileEncodingName.push_back(s0.substr(0, pos-3));
            fileEncoding.push_back(
              hdhC::stripSides( s0.substr(++pos)) );
          }
        }
        else if( s0[0] == '!' )
        {
          if(isDS)
            pathEncoding.back() += hdhC::stripSides( s0.substr(1)) ;
          else if(isFE)
            fileEncoding.back() += hdhC::stripSides( s0.substr(1)) ;
        }
        else
        {
          x_item = s0;

          if( x_item.size() > 1 )
          {
            cvMap[x_item[0]] = x_item[1];
            if( x_item.size() > 2 )
            {
              std::string s(x_item[0]+"_constr");
              cvMap[s] = x_item[2];
            }
          }
          else if( x_item.size() )
            cvMap[x_item[0]] = "*";
        }
      }

      continue;
    }

    if( s0.substr(0,11) == "BEGIN_LINE:" )
    {
      section.push_back( hdhC::stripSides( s0.substr(11) ) ) ;
      line.push_back(std::vector<std::string>());

      while( ! ifs.getLine(s0) )
      {
        if( s0.substr(0,9) == "END_LINE:" )
          break;
        else if( s0[0] == '!' )
          line.back().back() += s0.substr(1) ;
        else
          line.back().push_back(s0);
      }

      continue;
    }

    // variable section
    size_t last = s0.size() - 1;
    if( s0[last] == ':' )
    {
      // new variable
      // special ':' is equivalent to 'global:'
      if( s0[0] == ':' )
        s0 = "global:";

      varName.push_back(s0.substr(0, last));
      attName.push_back( std::vector<std::string>() );
      attValue.push_back( std::vector<std::string>() );
      continue;
    }

    // attributes
    if( attName.size() == 0 )
    {
      std::string key("7_2");
      if( notes->inq( key) )
      {
        std::string capt(
           "Syntax fault in the DRS_CV table: orphaned attribute, found ");
        capt += s0 ;

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
      }

      break;
    }

    x_item = s0;
    size_t sz = x_item.size() ;

    if( sz )
    {
      attName.back().push_back(x_item[0]);

      if( sz > 1 )
        attValue.back().push_back(x_item[1]);
      else
        attValue.back().push_back(hdhC::empty);
    }
  }

  ifs.close();

  return;
}

void
DRS_CV_Table::setParent(QA* p)
{
   pQA = p;
   notes = p->notes;
   return;
}

