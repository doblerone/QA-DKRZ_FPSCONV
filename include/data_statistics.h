#ifndef _DATA_STATISTIC_H
#define _DATA_STATISTIC_H

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <cmath>

#include <limits.h>
#include <values.h>

#include "hdhC.h"
#include "readline.h"
#include "statistics.h"

/*! \file data_statistics.h
 \brief Simple statistics and state of global data.
*/

class DataStatisticsBase
{
public:
//  DataStatisticsBase();

//! copy "constructor"
//  virtual DataStatisticsBase* clone(void) =0;

//! Destructor.
  virtual ~DataStatisticsBase(){}

//! Assign referenced object's data to *this.
//  virtual DataStatisticsBase& operator  = ( const DataStatisticsBase& ) =0;

  //! Add an entire 2D field to the current, inherent statistics.
  /*! Multiple use may add layers of a 3D block. Note: the
      underlying 2D alignment of cells must not change. */
  virtual void add(void*, void* p=0, std::string wType="") =0;
  virtual void add(void*, MtrxArr<double> &w) =0;

  //! Clear the internal statistics.
  virtual void clear(void) =0;

  //! Disable Fletcher16 checksum calculation.
  virtual void disableChecksum(void) =0;

  //! Enable Fletcher16 checksum calculation.
  virtual void enableChecksumWithClearedBits(size_t) =0;

  virtual void enableFillingValue(bool) =0;

  //! Get the area-weighted average of the added field(s).
  /*! Has to be virtual due to templ. T fillingValue*/
  virtual hdhC::FieldData
               get(void) =0;

  //! Get range and counts if Inf, NaN, and _FillValue(s).
  /*! Has to be virtual due to templ. T fillingValue*/
  virtual hdhC::FieldDataMeta
               getMeta(void) =0;

  //! Get the checksum of the field
  /*! Including filling values; invariant against endianess */
  virtual void getFletcher32(void*) =0;

//! Get area-weighted standard deviation.
/*! The result is calculated for a single layer without continuation.*/
  virtual bool getDataStndDev(double *) =0;

  virtual void setFillingValue(void*) =0;
};

// ---------------

template<typename T>
class DataStatistics : public DataStatisticsBase
{
public:
  //! Default constructor.
  DataStatistics();

//! Copy "constructor".
//  DataStatistics( const DataStatistics<T>& ); // copies all

//  DataStatistics<T>* clone(void)
//    { return new  DataStatistics<T>(*this); }

//! Assign referenced object's data to *this.
//  DataStatistics<T>& operator  = ( const DataStatistics<T> &);

//! Destructor.
  ~DataStatistics(){}

  //! Add an entire 2D field to the current, inherent statistics.
  /*! Multiple use may add layers of a 3D block. Note: the
      underlying 2D alignment of cells must not change. */
  void     add(void *v, void *w=0, std::string wType="");
  void     add(void *v, MtrxArr<double> &w);

template<typename W>
  void     add(MtrxArr<T> &val, MtrxArr<W> &weight);
  void     add(MtrxArr<T> &val);

  //! Clear the internal statistics.
  void     clear(void);

  //! Disable Fletcher16 checksum calculation.
  void     disableChecksum(void){isChecksum=false;}

  //! Enable Fletcher16 checksum calculation.
  void     enableChecksumWithClearedBits(size_t n)
              {numOfClearedBits=n;}

  void     enableFillingValue(bool b)
              {isFillingValue = b;}

  //! Get the area-weighted average of the added field(s).
  hdhC::FieldData
           get(void);

  //! Get range and counts if Inf, NaN, and _FillValue(s).
  hdhC::FieldDataMeta
           getMeta(void);

  //! Get the checksum of the field
  /*! Including filling values; invariant against endianess */
  void     getFletcher32(void*p)
               { getFletcher32(*reinterpret_cast<MtrxArr<T> *>(p) ); }
  void     getFletcher32(MtrxArr<T> &);

//! Get area-weighted standard deviation.
/*! The result is calculated for a single layer without continuation.*/
  bool     getDataStndDev(double *);

  void     setFillingValue(void* p)
             { fillingValue = *reinterpret_cast<T*>(p); }
/*
  //! Get the area-weighted standard deviation of the added field(s).
template<typename T>
  bool     getStndDev(GeoData<T> *, double&);
*/

    Statistics<T> statistics;
    uint32_t      checksum;
    bool          isResetChecksum;
    bool          isChecksum;
    size_t        numOfClearedBits;

    bool          isFillingValue;
    T             fillingValue;
};

// SOURCE

template<typename T>
DataStatistics<T>::DataStatistics() : statistics()
{
  isResetChecksum=true;
  isChecksum=true;
  numOfClearedBits=0;
}

template<typename T>
void
DataStatistics<T>::add(void *v, void *w, std::string wType)
{
  MtrxArr<T> &mvv = *reinterpret_cast<MtrxArr<T> *>(v) ;

  if( w )
  {
    if( wType == "double" )
    {
      MtrxArr<double> &mvw = *reinterpret_cast<MtrxArr<double> *>(w) ;
      add(mvv, mvw);
    }
  }
  else
    add(mvv);

  return;
}

template<typename T>
void
DataStatistics<T>::add(void *v, MtrxArr<double> &w)
{
  MtrxArr<T> &mvv = *reinterpret_cast<MtrxArr<T> *>(v) ;

  add(mvv, w);

  return;
}

template<typename T>
template<typename X>
void
DataStatistics<T>::add(MtrxArr<T> &v, MtrxArr<X> &w)
{
   // calculation of standard deviation
   statistics.addWeighted(v, w );

   statistics.addEnsembleCount();

   if( isChecksum )
     getFletcher32( v );

  return;
}

template<typename T>
void
DataStatistics<T>::add(MtrxArr<T> &v)
{
   // calculation of standard deviation
   statistics.add(v);

   statistics.addEnsembleCount();

   if( isChecksum )
     getFletcher32( v );

  return;
}

template<typename T>
void
DataStatistics<T>::clear(void)
{
  // calculation of standard deviation
  statistics.clear();

  if( isFillingValue )
    statistics.enableFillingValueTest(fillingValue);
  else
    statistics.disableFillingValueTest();

  isResetChecksum=true;

  return ;
}

template<typename T>
hdhC::FieldData
DataStatistics<T>::get(void)
{
  hdhC::FieldData a;

  double x0=0.;
  if( statistics.getSampleAverage(&x0) )
  {
    a.areaWeightedAve = x0;

    // areaFract:= actual areaFract times the ensembleCount
    a.areaFract  = statistics.getSampleTotalWeight() ;
    a.areaFract /= statistics.getSampleEnsembleCount();
    a.isValid=true ;
    if( statistics.getSampleStdDev(&x0) )
    {
      getDataStndDev( &(a.stndDev) );
      a.isStndDevValid=true;
    }
    else
      a.isStndDevValid=false;

    a.stndDev = x0 ;
  }
  else
  {
    a.areaWeightedAve = x0 ;
    a.areaFract = x0 ;
    a.isValid=false ;
    a.stndDev = x0 ;
    a.isStndDevValid=false;
  }

  a.max = statistics.getSampleMax();
  a.min = statistics.getSampleMin();
  a.size= static_cast<size_t>(statistics.getSampleSize());
  a.fillingValueNum = statistics.getSampleFillingValueNum("all");
  a.infCount        = statistics.getSampleFillingValueNum("inf");
  a.nanCount        = statistics.getSampleFillingValueNum("nan");

  // Fletche32 checksum
  if( isChecksum )
    a.checksum = checksum ;

  return a;
}

template<typename T>
hdhC::FieldDataMeta
DataStatistics<T>::getMeta(void)
{
  hdhC::FieldDataMeta a;

  a.size= static_cast<size_t>(statistics.getSampleSize());
  if( a.size )
  {
     a.isValid=true;
     a.max = statistics.getSampleMax();
     a.min = statistics.getSampleMin();
     a.size= static_cast<size_t>(statistics.getSampleSize());
     a.fillingValueNum = statistics.getSampleFillingValueNum("all");
     a.infCount        = statistics.getSampleFillingValueNum("inf");
     a.nanCount        = statistics.getSampleFillingValueNum("nan");
  }

  return a;
}

template<typename T>
bool
DataStatistics<T>::getDataStndDev(double *x)
{
  if( statistics.getSampleStdDev(x) )
    return true;
  else
  {
    *x = fillingValue ;
    return false;
  }
}

template<typename T>
void
DataStatistics<T>::getFletcher32(MtrxArr<T> &mv)
{
  checksum =
     hdhC::fletcher32(mv.arr, mv.size(),
                      &isResetChecksum, numOfClearedBits ) ;

  return ;
}

#endif
