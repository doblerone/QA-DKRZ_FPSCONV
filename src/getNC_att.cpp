#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "hdhC.h"

#include "iobj.h"
#include "brace_op.h"
//#include "date.h"
//#include "matrix_array.h"
#include "nc_api.h"
#include "annotation.h"
#include "split.h"
#include "readline.h"

#include "hdhC.cpp"
#include "BraceOP.cpp"
#include "NcAPI.cpp"
#include "Annotation.cpp"
#include "Split.cpp"
#include "ReadLine.cpp"

/*! \file getNC_att.cpp
 \brief Get value of global attributes.

 Purpose: get values of global attributes.\n
 A netCDF file is mandatory as first argument.
 The next arguments select global attributes.\n
 Output format (on separate lines): 'attribute=value'.
 Additionally, the key-word 'record_num' may be given as parameter.
 When an argument is not a valid attribute (or empty), then
 'attribute=' appears in the output.*/

void  exceptionError(std::string );

int main(int argc,char *argv[])
{
  NcAPI nc;

  if( ! nc.open( argv[1], "", false) )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "getNC_att: netCDF NcAPI::open()\n" ;
    ostr << "Could not open file " << argv[1] ;
    exceptionError( ostr.str() );  // exits
  }

  std::string att;
  bool isOnlyValue=false;

  std::vector<std::string> vs_att;
  std::vector<std::string> vs_val;

  for( int i=2 ; i < argc ; ++i )
  {
    att=argv[i];

    if( att == "-h" || att == "--help" )
    {
        std::cout << "usage: getNC_att.x [--help] [--only-value] att-name [att-name ...]";
        std::cout << std::endl ;
        exit(1);
    }

    if( att == "--only-value" )
    {
        isOnlyValue=true;
        continue;
    }

    if( att == "record_num" )
    {
      std::ostringstream ostr;
      ostr << nc.getNumOfRecords() ;

      vs_att.push_back("record_num");
      vs_val.push_back(ostr.str());

      continue;
    }

    if( ! nc.isAttValid(att) )
    {
      if( !isOnlyValue )
      {
        vs_att.push_back(att);
        vs_val.push_back("");
      }

      continue;
    }

    // is it a number or a char?
    if( nc.layout.globalAttType[nc.layout.globalAttMap[att]] == NC_CHAR )
    {
      vs_att.push_back(att);
      vs_val.push_back(nc.getAttString(att));
    }
    else
    {
       // is a number
       std::vector<double> v;
       nc.getAttValues(v, att);
       if( v.size() )
       {
         std::ostringstream ostr;
         ostr << v[0] ;
         for( size_t j=1 ; j < v.size() ; ++j )
            ostr << "," << v[j] ;

         vs_att.push_back(att);
         vs_val.push_back(ostr.str());
       }
    }
  }

  int retValue=0 ;

  if( vs_att.size() )
  {
     for( size_t i=0 ; i < vs_att.size() ; ++i )
     {
       if(i)
           std::cout << " " ;

       if( !isOnlyValue )
           std::cout << vs_att[i] << "=" ;

       if( vs_val[i].size() )
         std::cout << vs_val[i] ;
       else
         retValue=1;
     }
  }
  else
     retValue=1;

  return retValue;
}

void
exceptionError(std::string str)
{
  std::string name = "error_getNC_att.txt";

  // open file for writing
  std::ofstream ofsError(name.c_str());

  ofsError << str << std::endl;

  exit(1);
  return ;
}
