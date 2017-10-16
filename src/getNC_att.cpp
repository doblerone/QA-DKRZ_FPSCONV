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

std::string getAttVal(NcAPI&, std::string attName, std::string varName="");

void  exceptionError(std::string );
void  usage(void);

int main(int argc,char *argv[])
{
  NcAPI nc;

  bool isOnlyValue=false;
  bool isNewline=true;
  bool skipInvalid=false;
  std::string delim("\n");
  std::string record_num("record_num");

  std::vector<std::string> vs_att;
  std::vector<std::string> vs_val;

  for( int i=1 ; i < argc ; ++i )
  {
    std::string att=argv[i];

    if( att == "-h" || att == "--help" )
    {
      usage();
      return 0;
    }

    if( i==1 )
    {
      if( nc.open( att, "", false) )
         continue;
      else
      {
        std::ostringstream ostr(std::ios::app);
        ostr << "getNC_att: netCDF NcAPI::open()\n" ;
        ostr << "Could not open file " << argv[1] ;
        exceptionError( ostr.str() );  // exits
      }
    }

    if( att == "--only-value" )
    {
        isOnlyValue=true;
        continue;
    }

    if( att == "--no-newline" )
    {
        isNewline=false;
        continue;
    }

    if( att.substr(0,7) == "--delim" )
    {
        delim=att.substr(8);

        continue;
    }

    if( att == record_num || att == ("--" + record_num) )
    {
      vs_att.push_back(record_num);
      continue;
    }

    if( att == "--skip-invalid" )
    {
        skipInvalid=true;
        continue;
    }

    vs_att.push_back(att);
  }

  int retValue=0 ;
  std::ostringstream ostr;

  if( vs_att.size() )
  {
     for( size_t i=0 ; i < vs_att.size() ; ++i )
     {
        std::string att(vs_att[i]);

        if( att == record_num )
        {
           vs_val.push_back( hdhC::double2String(nc.getNumOfRecords()) ) ;
           continue;
        }

        // variable or global attribute?
        Split x_att(att,':');
        std::string aName;
        std::string vName;

        if( x_att.size() == 1 )
          aName = att;
        else
        {
          vName=x_att[0];
          aName=x_att[1];
        }

        if( nc.isAttValid(aName, vName) )
           vs_val.push_back( getAttVal(nc, aName, vName) ) ;
        else
        {
            if( ! skipInvalid )
            {
               std::cerr << "invalid attribute " << att << std::endl;
               exit(1);
            }

            vs_val.push_back("");
            retValue=1;
        }
     }
  }
  else
     retValue=1;

  size_t last=vs_att.size() -1 ;
  std::string out;

  for( size_t i=0 ; i < vs_att.size() ; ++i )
  {
     out.clear();

   if( !isOnlyValue )
      out=vs_att[i] + "=" ;

   if( vs_val[i].size() )
      out += vs_val[i] ;
   else
      continue;


   if( i != last )
       out += delim ;

   std::cout << out ;
  }

  if( isNewline )
    std::cout << std::endl ;

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

void
usage(void)
{
   std::string hlp("usage: getNC_att.x [opts] att-name [, att-name, [...]]\n");
   hlp += "  --only-value    Do no print the name of the attribute\n" ;
   hlp += "  --no-newline    End output eventually without a newline\n" ;
   hlp += "  --delim         Delimiter between distinct att=value occurrences;\n" ;
   hlp += "                  by default each item on a new line.\n";
   hlp += " [--]record_num   Number of records, i.e. the unlimited variable\n" ;
   hlp += "  --skip_invalid  Skip invalid attributes; exit by default\n" ;
   hlp += " [variable:]attribute   When variable: oimiitted, then the global attribute\n";

   std::cout << hlp << std::endl;
}

std::string
getAttVal(NcAPI& nc, std::string aName, std::string vName)
{
    std::string str;
    std::vector<std::string> vs_str ;

    nc.getAttValues(vs_str, aName, vName);

    for( size_t j=0 ; j < vs_str.size() ; ++j )
    {
       if(j)
           str += ',';

       str += vs_str[j];
    }

    return str;
}

