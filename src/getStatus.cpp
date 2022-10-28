#include <iostream>
#include <cmath>
#include <string>

#include "hdhC.h"

#include "hdhC.cpp"
#include "Split.cpp"

std::string getHumanTime(double);
std::string getHumanStorage(double);

int main(int argc, const char* argv[])
{
 /*
   getStatus.x ${checkTime} ${usedStorage} ${totalStorage}

   argv[1]     checkTime
   argv[2]     usedStorage
   argv[3]     totalStorage
   argv[4]     elapsedTime
 */

  double checkTime, usedStorage, totalStorage;
  double elapsedTime;

  if( ! hdhC::isDigit(argv[1]) )
    return 1;
  else
    checkTime=hdhC::string2Double(argv[1]);

  if( ! hdhC::isDigit(argv[2]) )
    return 1;
  else
    usedStorage=hdhC::string2Double(argv[2]);

  if( ! hdhC::isDigit(argv[3]) )
    return 1;
  else
    totalStorage=hdhC::string2Double(argv[3]);

  if( ! hdhC::isDigit(argv[4]) )
    return 1;
  else
    elapsedTime=hdhC::string2Double(argv[4]);

  std::string usedStorageStr;
  std::string guessedTotalTime;

  if( usedStorage == 0. )
  {
     usedStorageStr="0";
     guessedTotalTime="--";
  }
  else
  {
    double eT    = checkTime * totalStorage / usedStorage ;
    guessedTotalTime = getHumanTime(eT);
    usedStorageStr  = getHumanStorage(usedStorage);
  }

  std::string str( getHumanTime(checkTime) );
  str += " ";
  str += guessedTotalTime ;
  str += " ";

  str += usedStorageStr;
  str += " ";
  str += getHumanStorage(totalStorage);

  str += " ";
  str += getHumanTime(elapsedTime) ;

  std::cout << str << std::endl ;

  return 0 ;
}


std::string getHumanStorage(double val)
{
  char u[]={ 'b', 'K', 'M', 'G', 'T'};

  size_t i=0;
  for( ; val > 1024. ; ++i)
    val /= 1024. ;

  std::string str( hdhC::double2String(val,0) );
  str += u[i];

  return str;
}

std::string getHumanTime(double val)
{
  double s, m, h, d ;
  s = m = h = d = 0. ;
  int i;
  double v0;
  std::string str;

  if( val >= 60. )
  {
    v0 = val/60.;
    i = static_cast<int>(val) - static_cast<int>(v0) * 60 ;
    s=static_cast<double>(i);
    val = v0;
  }
  else
  {
    str += hdhC::double2String(val,0);
    str +="s";
    return str;
  }

  if( val >= 60. )
  {
    v0 = val/60.;
    i = static_cast<int>(val) - static_cast<int>(v0) * 60 ;
    m=static_cast<double>(i);
    val = v0;
  }
  else
  {
    val = static_cast<double>( static_cast<int>(val) ) ;
    str += hdhC::double2String(val,"p0");
    str +="m";
    str += hdhC::double2String(s,"p0");
    str +="s";
    return str;
  }

  if( val >= 24. )
  {
    v0 = val/24.;
    i = static_cast<int>(val) - static_cast<int>(v0) * 24 ;
    h=static_cast<double>(i);
    val = v0;
  }
  else
  {
    val = static_cast<double>( static_cast<int>(val) ) ;
    str += hdhC::double2String(val,"p0");
    str +="h";
    str += hdhC::double2String(m,"p0");
    str +="m";
    str += hdhC::double2String(s,"p0");
    str +="s";
    return str;
  }

  val = static_cast<double>( static_cast<int>(val) ) ;
  str += hdhC::double2String(val,"p0");
  str +="d";
  str += hdhC::double2String(h,"p0");
  str +="h";
  str += hdhC::double2String(m,"p0");
  str +="m";
  str += hdhC::double2String(s,"p0");
  str +="s";

  return str;
}
