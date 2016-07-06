// Designed for the float type. Arbitrary template
// functionality is a TODO


#include "cf_main.h"

// All sources are included for the shipping version.
// ------------------

// Most compilers don't know to build libs with templates.

#include "hdhC.cpp"

#include "BraceOP.cpp"
#include "Date.cpp"
#include "GetOpt_hdh.cpp"
#include "NcAPI.cpp"
#include "ReadLine.cpp"
#include "Split.cpp"
#include "Statistics.cpp"

#include "Variable.cpp"
#include "Base.cpp"
#include "CF.cpp"
#include "Annotation.cpp"
#include "InFile.cpp"
#include "Parse.cpp"

// ------------------

int main(int argc,char *argv[])
{
  InFile in;
  CF cf;
  Annotation notes;

  in.linkObject((IObj*) &notes);
  in.linkObject((IObj*) &cf);
  cf.linkObject((IObj*) &in);
  cf.linkObject((IObj*) &notes);

  parseOptions(argc, argv, in, cf, notes);

  // runs also the cf checker
  in.init();

  in.pullMetaData('V');
  in.pullMetaData('T');
  in.pullMetaData('G');

  cf.setFilename(in.file);
  (void) cf.run();

  cf.notes->printFlags();

//  return cf.notes->getExitState() ;
  return 0 ;
}

void
parseOptions(int argc, char *org_argv[],
   InFile &in, CF &cf, Annotation &an)
{
  GetOpt opt;

  opt.opterr=0;
  int copt;
  std::string str0("null");
  std::string str1;

  // working with std::string is preferred
  std::vector<std::string> argv;

  for( int i=0 ; i < argc ; ++i)
    argv.push_back( org_argv[i] );

  // read options from file?
  readOptions(argv);
  int sz=argv.size();

  // getopt expects char*[].
  char **pargv = new char* [argv.size()+1] ;
  for( size_t i=0 ; i < argv.size() ; ++i )
    pargv[i] = const_cast<char*>(argv[i].c_str());

  std::string tablePath;

  while( (copt = opt.getopt(sz, pargv,
   "f:p:t:<--help>")) != -1 )
  {
    if( opt.longOption > 0 )
      str0=opt.longOption;

    if( opt.longOption && str0 == "--help" )
    {
      Base::help();
//      CellStatistics::help();
      InFile::help();
//      Oper::help();
//      OutFile::help();
//      TimeControl::help();
      exit(1);
    }

    switch ( copt )
    {
      case 'f':
        in.setFilename(opt.optarg) ;
        break;
      case 'p':
        in.file.path=opt.optarg ;
        break;
      case 't':
        an.setTablePath(opt.optarg);
        cf.setTablePath(opt.optarg);
        break;
      default:
//      description();
        break;
    }
  }

  Split x_argv;
  x_argv.setSeparator(":");
  std::vector<std::string> optStr;
  int osz = static_cast<int>(argv.size());

  for( int i=opt.optind ; i < osz ; ++i )
  {
     x_argv = argv[i];

     if( x_argv.size() )
     {
        for( size_t l=1 ; l < x_argv.size() ; ++l )
          optStr.push_back( x_argv[l] );

        if( x_argv[0] == "X" )
           an.setOptions(optStr);
        else if( x_argv[0] == "CF" )
           cf.setOptions(optStr);
        else if( x_argv[0] == "IN" )
           in.setOptions(optStr);

        optStr.clear();
     }
  }

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
        for( size_t j=0 ; j < rL.size() ; ++j)
        {
          if( rL.at(0) == '#')
            continue; // i.e. also for embedded Split.sep-separated #
          argv.push_back( rL.getItem(j) ) ;
        }
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
