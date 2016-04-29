
#include "qa_main.h"

// All sources are included for the shipping version.
// ------------------

// Most compilers don't know to build libs with templates.

#include "hdhC.cpp"

#include "BraceOP.cpp"
#include "Date.cpp"
#include "FreqDist.cpp"
#include "GetOpt_hdh.cpp"
#include "NcAPI.cpp"
#include "ReadLine.cpp"
#include "Split.cpp"
#include "Statistics.cpp"

#include "Variable.cpp"
#include "Base.cpp"
#include "CellStatistics.cpp"
#include "CF.cpp"
#include "FD_interface.cpp"
#include "Annotation.cpp"
#include "InFile.cpp"
#include "Oper.cpp"
#include "OutFile.cpp"
#include "Parse.cpp"
#include "QA.cpp"
#include "QA_data.cpp"
#include "QA_DRS_CV_Table.cpp"
#include "QA_time.cpp"
#include "QA_cnsty.cpp"

#if defined CORDEX
  #include "QA_CORDEX.cpp"
#elif defined CMIP5
  #include "QA_CMIP5.cpp"
#endif

#include "TimeControl.cpp"

// ------------------

int main(int argc,char *argv[])
{
  IObjContainer ioc;

  isPostProc = false;
  isCPU_time=false;

  parseOptions(argc, argv, ioc);

  if( isCPU_time )
  {
    getWallClockSeconds();
    getCPUSeconds();
  }

  // path/filename was set in setObjProperties()

  // About processing: init() and entry() functions of the
  // base-derived objects are addressed by pointers,
  // where the first call points to init() and than the pointer
  // switches to entry().
  // Objects for Frequency Dist., TimeControl, and QualityControl,
  // respectively, are linked by pointers to the base-derived objects.

  do
  {
    // Do we go for building file names?
    // Processing of a sequence of multiple netCDF files.
    // Or just a single instance for a specified filename.

    // build filenames and sync with date or process single files.
    if( updateIn(ioc.in) )
      break ;  // no more files or an error

  // do all the real stuff
  } while( entry(ioc.vIObj) );

  return finally(ioc);
}

bool entry(std::vector<IObj*> &vIObj)
{
  if( vIObj.size() == 0 )  // no operation
    return false;

  size_t i=0;

  // there will be only a single InFile for the QA
  InFile *pin = 0;

//  ####### the LOOP ###############

// preparation: rules for vIObj pointers
// 1) InFile obj must be the first
// 2) QA the second, if available
// 3) if QA available, remove InFile from loop
//    (in::entry() is called in QA::entry())
  std::vector<IObj*> loopObj;
  size_t objLoopBeg=0;  //index to point eventually to the QA object

  // identify the objects (in, CF) which are not executed in a loop below
  for( i=0 ; i < vIObj.size() ; ++i )
  {
    if( vIObj[i]->getObjName() == "IN")
      // there will be only a single InFile
      pin = static_cast<InFile*>(vIObj[i]);

    else if( vIObj[i]->getObjName() == "QA")
      loopObj.push_back(vIObj[i]);
  }

  if( pin == 0 )
     return false;  // no InFile

  // records could have been reset by TC,
/*
  for( ; pin->currRec < pin->ncRecEnd
           ; pin->currRec += pin->recStride )
  {
    // Loop through the chain of objects given by IObj pointers.
    // The method pointed to is init() or entry()
    // according to polymorphism (abstract classes).
    // Note: In context of the Quality Control: the first object
    // relates to the netCDF input file (InFile), then there must be
    // a QA object to conduct the quality control. Objects may be
    // given for the calculation of frequency distributions,
    // depending on the selections in the configuration.
    // (Time control is imbedded in the InFile object).
*/
    // The called method return true, if a break condition was found,
    // e.g. termination due to a time constraint
    for( i=objLoopBeg ; i < loopObj.size() ; ++i )
       if( (loopObj[i]->*(loopObj[i]->execPtr))() )
         return false;
//  }  // record loop

  return false ;  // end of record or end of time window
}

int
finally(IObjContainer &ioc)
{
  int retVal=0;

  if( ioc.qA[0].isProgress() )
  {
    for( size_t i=0 ; i < ioc.fDI.size() ; ++i)
      ioc.fDI[i].finally();

    for( size_t i=0 ; i < ioc.cF.size() ; ++i)
    {
      if( &ioc.cF[i] )
      {
        // there will be only a single instance
        retVal = ioc.cF[i].finally();
      }
    }

    for( size_t i=0 ; i < ioc.qA.size() ; ++i)
    {
      if( &ioc.qA[i] )
      {
        int rV = ioc.qA[i].notes->getExitValue();
        retVal = ( retVal > rV ) ? retVal : rV ;
        retVal = ioc.qA[i].finally( retVal );
      }
    }

    // get check results
    std::map<std::string, std::string> status_id;
    std::map<std::string, int> status_rank;

    Split x_res;
    x_res.setSeparator("|");
    Split x_rank;
    x_rank.setSeparator("=");
    Split x_item;
    x_item.setSeparator("%");

    // Check conclusions from all annotation objects.
    // When there are more than a single set of check results per annotation object,
    // then these stre merged according to the ranking
    // FAIL:4, PASS: 3, FIXED: 2, disabled: 1, N/A: 0
    // getCheckRsults() returns: CF_conv:string=rank|meta-data:string=rank ...
    for( size_t i=0 ; i < ioc.an.size() ; ++i)
    {
      if( &ioc.an[i] )
      {
        ioc.an[i].printFlags();

        x_res = ioc.an[i].getCheckStatus(true);
        for( size_t i=0 ; i < x_res.size() ; ++i )
        {
            x_rank = x_res[i];
            x_item = x_rank[0];
            int rank = hdhC::string2Double(x_rank[1]);

            if( status_id.count(x_rank[0]) == 0 )
            {
              status_id[x_item[0]] = x_item[1] ;
              status_rank[x_item[0]] = rank;
            }
            else
            {
               if( rank >= status_rank[x_item[0]] )
               {
                  status_id[x_item[0]] = x_item[1] ;
                  status_rank[x_item[0]] = rank;
               }
            }
        }
      }
    }

    // print check results
    std::string out( "CHECK-BEG" );

    std::map<std::string, std::string>::iterator it;

    bool isComma=false;
    for( it=status_id.begin() ; it != status_id.end() ; ++it )
    {
      if(isComma)
        out += ", ";
      out += it->first;
      out += ": ";
      out += it->second;

      isComma=true;
    }

    out += "CHECK-END";  // mark of the end of an output line
    std::cout << out << std::endl;

    if( ioc.qA.size() == 0 )
      std::cout << "STATUS-BEG" << retVal << "STATUS-END";

    // distinguish from a sytem crash (segmentation error)
    //  std::cout << "NormalExecution;" << std::endl;
    //  notes.print();
  }
  else
  {
    std::cout << "STATUS-BEG63STATUS-END";
    retVal=0;
  }

  if( isCPU_time )
  {
    getWallClockSeconds();
    getCPUSeconds();
  }

  return retVal;
}


void getCPUSeconds(void)
{
  static bool isFirst=true;

  double v;
  clock_t t;

  t = clock() ;

  if( isFirst )
  {
     isFirst=false;
     return ;
  }

  v = ((double) t / CLOCKS_PER_SEC);

  std::cout << "CPU-BEG" << v << "CPU-END" << std::endl;
  std::cout << ";" ;

  std::cout << std::endl;

  return ;
}

template <typename Type>
void
instantiateObject(std::vector<Type> &obj, std::string &name, int id, std::string &param)
{
  // Create instance of obj and append to vector.
  obj.push_back( Type() );

  obj.back().setObjID(id);

  if( param.size() )
  {
    Split splt(param,":");
    for( size_t i=0 ; i < splt.size() ; ++i )
      obj.back().setOptions(splt[i]);
  }

  return ;
}

void
linkObj(std::vector<std::vector<std::string> > &list,
   IObjContainer &ioc)
{
  // uses the vector vIObj
  std::string objName;
  int objID;

  for( size_t j=0 ; j < list.size() ; ++j )
  {
    // get name+id of the owning object
    Parse::getListObj( list[j][0], objName, objID);

    // find the index of the owning object
    size_t v;
    for( v=0 ; v < ioc.vIObj.size() ; ++v )
       if( ioc.vIObj[v]->getObjName() == objName )
         if( ioc.vIObj[v]->getObjID() == objID )
             break;

    for( size_t i=1 ; i < list[j].size() ; ++i )
    {
      // get name+id of a linked object
      Parse::getListObj( list[j][i], objName, objID);

      // find the linked object
      size_t w;
      for( w=0 ; w < ioc.vIObj.size() ; ++w )
      {
        if( ioc.vIObj[w]->getObjName() == objName )
        {
          if( ioc.vIObj[w]->getObjID() == objID )
          {
             ioc.vIObj[v]->linkObject( ioc.vIObj[w] );
             break;
          }
        }
      }
    }
  }

  return;
}

void
makeObject(std::vector<std::vector<std::string> > &list, IObjContainer &ioc)
{
  std::string name;
  std::string param;
  int id;
  for( size_t i0=0 ; i0 < list.size() ; ++i0 )
  {
    (void) Parse::getListObj( list[i0][0], name, id, param);

    if( name == "X" )
      instantiateObject(ioc.an, name, id, param );
    else if( name == "CS" )
      instantiateObject(ioc.cS, name, id, param );
    else if( name == "CF" )
      instantiateObject(ioc.cF, name, id, param );
    else if( name == "FD" )
      instantiateObject(ioc.fDI, name, id,  param );
    else if( name == "IN" )
      instantiateObject(ioc.in, name, id,  param );
    else if( name == "OP" )
      instantiateObject(ioc.op, name, id,  param );
    else if( name == "OUT" )
      instantiateObject(ioc.out, name, id,  param );
    else if( name == "QA" )
      instantiateObject(ioc.qA, name, id,  param );
    else if( name == "TC" )
      instantiateObject(ioc.tC, name, id,  param );
  }

  // append the obj pointers to the list of IObj-casted pointers
  for( size_t i=0 ; i < ioc.an.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.an[i]) );
  for( size_t i=0 ; i < ioc.cS.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.cS[i]) );
  for( size_t i=0 ; i < ioc.cF.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.cF[i]) );
  for( size_t i=0 ; i < ioc.fDI.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.fDI[i]) );
  for( size_t i=0 ; i < ioc.in.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.in[i]) );
  for( size_t i=0 ; i < ioc.op.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.op[i]) );
  for( size_t i=0 ; i < ioc.out.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.out[i]) );
  for( size_t i=0 ; i < ioc.qA.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.qA[i]) );
  for( size_t i=0 ; i < ioc.tC.size() ; ++i )
    ioc.vIObj.push_back( &(ioc.tC[i]) );

  return;
}

void
parseOptions(int argc, char *org_argv[], IObjContainer &ioc)
{
  GetOpt opt;
  opt.opterr=0;
  int copt;
  std::string str0("null");
  std::string str1;

//  std::vector<std::string> inputFilename;
  std::vector<std::string> outputFilename;

  // working with std::string is preferred
  std::vector<std::string> argv;

  for( int i=0 ; i < argc ; ++i)
    argv.push_back( org_argv[i] );

  // read options from file?
  readOptions(argv);
  int sz=argv.size();

  // printing is split, so remember
  bool isPrintTest=false;

  // getopt expects char*[].
  char **pargv = new char* [sz+1] ;
  for( int i=0 ; i < sz ; ++i )
    pargv[i] = const_cast<char*>(argv[i].c_str());

  while( (copt = opt.getopt(sz, pargv,
   "f:p:t:v:<--cpu-time><--help><--post><--printTest>")) != -1 )
  {
    if( opt.longOption > 0 )
      str0=opt.longOption;

    if( opt.longOption && str0 == "--help" )
    {
      Base::help();
//      CellStatistics::help();
      FD_interface::help();
      InFile::help();
//      Oper::help();
//      OutFile::help();
      QA::help();
//      TimeControl::help();
      exit(1);
    }

    if( opt.longOption && str0 == "--cpu-time" )
    {
      isCPU_time=true;
      continue;
    }

    if( opt.longOption && str0 == "--post" )
    {
      isPostProc=true;
      continue;
    }

    if( opt.longOption && str0 == "--printTest" )
    {
      isPrintTest=true;
      continue;
    }

    switch ( copt )
    {
      case 'f':
        ::NC_FILENAME = opt.optarg ;
        break;
      case 'p':
        ::NC_PATH = opt.optarg ;
        break;
      case 't':
        ::TABLE_PATH = opt.optarg ;
        break;
      default:
//      description();
        break;
    }
  }

  // Note: pargv are pointers to argv items.
  std::vector<std::string> aargv;
  for( int i=opt.optind ; i < sz ; ++i)
    aargv.push_back( pargv[i] );

  argv = aargv ;

  delete[] pargv ;

  // parse argv for instructions of making objects
  // and their linkage.
  Parse parse(argv);

  std::vector<std::vector<std::string> > linkList
       = parse.getLinkList();

  if( isPrintTest )
    parse.printList();  // exits

  // objects are created
  makeObject(linkList, ioc);

  // apply parameters to objects
  setObjProperties(linkList, ioc);

  linkObj(linkList, ioc);

  scanGdRelation(linkList, ioc);

  return ;
}

void
readOptions(std::vector<std::string> &argv)
{
  ReadLine rL;
  std::string file;

  bool isCinProvided=false;

  // find filename of parameter-file.
  for( size_t i=0 ; i<argv.size() ; ++i)
  {
     // find "--file"
     if( argv[i].find("--file") < std::string::npos )
     {
       if( argv[i] == "--file" && (i+1) < argv.size() )
       {
         file = argv[i+1];

         // remove: --file's argument
         argv.erase(argv.begin() + i +1 );
       }
       else
         file = argv[i].substr(7);

       // remove: --file
       argv.erase(argv.begin() + i);
       break;
     }

     if( argv[i].find("--input") < std::string::npos )
     {
       isCinProvided=true;

       // remove argument
       argv.erase(argv.begin() + i);
       break;
     }
  }

  if( file.size() > 0 )
  {
     rL.open(file); // from file

     // read instructions
     while( ! rL.readLine() )
     {
        if( rL.at(0) == '#')
          continue;

        for( size_t j=0 ; j < rL.size() ; ++j)
          argv.push_back( rL.getItem(j) ) ;
     }

    rL.close();
  }

  if( !isCinProvided)
    return;

  // now switch to std::cin
  rL.connect_cin();  // std::cin

  // read instructions
  while( ! rL.readLine() )
  {
     if( rL.at(0) == '#' )
       continue;

     for( size_t j=0 ; j < rL.size() ; ++j)
       argv.push_back( rL.getItem(j) ) ;
  }

  return;
}

void
scanGdRelation(std::vector<std::vector<std::string> > &list, IObjContainer &ioc)
{
  std::string s0;
  std::string name;
  std::string name1;
  int id;

  // scan for definitions which declare an operation that requires
  // a data field of its own, or preferably, would modify data of
  // one of the reference fields (but, only if this would not be
  // required by a following operation).
  // Modification of given data is only admissible, if there is no
  // identical reference to follow.

  std::vector<bool> isUsedLater;
  std::vector<std::string> useGeoDataObjStr;

  for( size_t i0=0 ; i0 < list.size() ; ++i0 )
  {
    // here as placeholder. Insertion to be done at the end of scope
    useGeoDataObjStr.push_back( "" );

/*
    // true if an OP definition modifies data
    if( list[i0][0].find("<") < std::string::npos )
      continue;
*/

    isUsedLater.clear();

    // check for each reference: one by one
    for( size_t i1=1 ; i1 < list[i0].size() ; ++i1 )
    {
      isUsedLater.push_back( false ) ;  // at the start

      (void) Parse::getListObj( list[i0][i1], name, id);
      if ( name != "OP" )
        continue;

      // OP always passes such an object
      if ( name == "IN" )
        continue;


      // if identical references are given on curr row --> useLater
      // for the two references
      for( size_t i2=1 ; i2 < list[i0].size() ; ++i2 )
      {
        if( i1 == i2 )
          continue;

        // references on current row
        (void) Parse::getListObj( list[i0][i2], name1, id);
        if( name == name1 )
          isUsedLater.back()=true ;
      }
      if( isUsedLater.back() )
        continue;  //try next reference

      // search the current ref in  subsequent definitions
      for( size_t j0=i0+1 ; j0 < list.size() ; ++j0 )
      {
        // name of definition
        (void) Parse::getListObj( list[j0][0], name1, id);
        if ( name1 == "OUT" )
          continue; // out does not modify

        // take only the refs into account
        for( size_t j1=1 ; j1 < list[j0].size() ; ++j1 )
        {
          // name of reference
          (void) Parse::getListObj( list[j0][j1], name1, id);

          if( name == name1 )
            isUsedLater.back()=true ;
        }
      }
    }

    bool isNoDataAllocation=false;
    size_t index;
    for( size_t k=0 ; k < isUsedLater.size() ; ++k )
    {
      if( ! isUsedLater[k] )
      {
        isNoDataAllocation=true;
        index=k;
        break;
      }
    }

    if( isNoDataAllocation )
       // plus 1, because list starts with definition
       useGeoDataObjStr.back() = list[i0][index+1] ;
  }

  // Allocate or link obj:
  // useGeoDataObjStr indicates whether a new own GeoData obj has
  // to be allocated or if an already existing one can be
  // used (and modified) further.
  for( size_t i0=0 ; i0 < list.size() ; ++i0 )
  {
    // Note: name is for the definition
    (void) Parse::getListObj( list[i0][0], name, id);

    if( name == "IN" )
      continue;  // this has already been allocated

    if( useGeoDataObjStr[i0].size() == 0 )
    {
      // Allocate memory until the end of the run
      if( name == "OP" )
        ioc.op[id].isAllocate=true;

      // No allocation for CS FD and OUT objects; they use
      // specialised versions.
    }
  }

  return;
}

void
setObjProperties(std::vector<std::vector<std::string> > &list, IObjContainer &ioc)
{
  std::string name;
  int id;

  for( size_t i=0 ; i < ioc.vIObj.size() ; ++i )
  {
    name = ioc.vIObj[i]->getObjName();
    id   = ioc.vIObj[i]->getObjID();

    IObj *ip = ioc.vIObj[i] ;

    if( name == "X" )
    {
      Annotation *p = dynamic_cast<Annotation*>(ip);
      p->setTablePath(::TABLE_PATH);

      if( id == 0 )
        notes = &ioc.an[i] ;
    }
    else if( name == "CS" )
      ;
    else if( name == "CF" )
    {
      CF *p = dynamic_cast<CF*>(ip);
      p->setTablePath(::TABLE_PATH);
      p->enableCheck();
    }
    else if( name == "FD" )
      ;
    else if( name == "IN" )
    {
      InFile *p = dynamic_cast<InFile*>(ip);
      p->setFilename(::NC_PATH + "/" + ::NC_FILENAME);
    }
    else if( name == "OP" )
      ;
    else if( name == "OUT" )
      ;
    else if( name == "QA" )
    {
      QA *p = dynamic_cast<QA*>(ip);
      p->setTablePath(::TABLE_PATH);
    }
    else if( name == "TC" )
      ;
  }

  return;
}

bool
updateIn(std::vector<InFile> &in)
{
  // init once for each single file or
  // build filenames from pattern and sync with date

/* not in the QA context
  for( size_t j=0 ; j < in.size() ; ++j )
  {
    // Build a filename from date.
    // Program exits, if beyond the time frame.
    if( in[j].tC > 0 && in[j].tC->isBuildFilename() )
    {
      in[j].filename = in[j].tC->getFilename() ;
      if( in[j].filename.size() == 0 )
        return true;  // stop
    }
  }
*/

  // start reading and processing
  for( size_t j=0 ; j < in.size() ; ++j )
  {
    in[j].init();  // Note: error detection and exit in the method

    // synchronise start and end records of the nc-file
    // to optional time limits.
    if( in[j].tC > 0 )
        in[j].tC->init();
  }

  return false;
}

void getWallClockSeconds(void)
{
  static clock_t start=0;
  static bool isFirst=true;

  /* a struct timeval to hold the current time. */
  /* timeval is defined in sys/time.h */
  struct timeval time;

  double t;

  /* get the current time and store value in 'time' */
  gettimeofday(&time, NULL);

  /* convert the time to a fraction */

  t = time.tv_sec + (time.tv_usec/1000000.0);

  if( isFirst )
  {
     isFirst=false;
     start = t ;
     return ;
  }

  t -= start;

  std::cout << "REAL-BEG" << t << "REAL-END" << std::endl;
  std::cout << ";"  << std::endl;

  return ;
}
