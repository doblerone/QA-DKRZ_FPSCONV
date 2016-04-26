#ifndef _GEO_META_H
#define _GEO_META_H

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <cmath>

#include <limits.h>
#include <values.h>

//#include "hdhC.h"
//#include "data_properties.h"
#include "getopt_hdh.h"
#include "matrix_array.h"
#include "readline.h"
#include "statistics.h"

/*! \file geo_meta.h
 \brief Management of geo-located data.
*/

//! Data management of type T on a geographical grid.

/*! This class is still heavily under construction. A change
 from grid-based to cell-based representation of geographical
 storage is going on. At present, an object of this class will
 get data from a netCDF file. The type of data
 is identical to the one of the netCDF file.
 The data field is stored in objects derived from the MtrxArr
 class, i.e. there is access via the MtrxArr class for efficiency
 (reference counting) as well as for a matrix-like representation for convenience.\n
 Slicing a 3D data block into 2D layers is an option.\n
 Handling of missing|filling values is not as efficient one would like.\n

 Note: Comments here do not really reflect, if a method is obsolete.\n*/

class GeoMetaBase
{
public:
//! Default constructor.
//    GeoMetaBase();

//! copy "constructor"
    virtual GeoMetaBase* clone(void) =0;

//! Destructor.
    virtual ~GeoMetaBase(){;}

//! Assign referenced object's data to *this.
//    virtual GeoMetaBase& operator  = ( const GeoMetaBase& ) =0;

/*
//! Add 'val' to the grid-cell which contains coordinates 'lat','lon'.
  virtual void   add( void*, double lat, double lon)=0;

//! Add 'val' to the cell[iLat][iLon].
  virtual void   add( void*, int iLat, int iLon)=0;
*/
/*
//! Assign 'val' to the cell which contains coordinates 'lat','lon'.
  virtual void   assign( void*, double lat, double lon)=0;

//! Assign 'val' to the cell[iLat][iLon].
  virtual void   assign( void*, int iLat, int iLon)=0;
*/

  virtual void clear(void) =0;

//! Copy grid-cell values and boolean marker from object.
/*! The numbers of cells over longitude and latitude are checked.
    It is assumed that identical numbers represent the same grid.*/
//  virtual void   cpCellValue( void*) =0;

//! Switch layer
/*! Link cellValue to the i-th layer of a 3-D object. If i equals -1,
    then the curent level is incremented unitl it exceeds the top-most
    layer. Then it starts all over again.*/
  virtual bool cycleLayers(void) =0;

//! Disable/enable geographic coordinates.
/*! Set sum of weights to unit.*/
  virtual void disableCellShape(void) =0;
  virtual void enableCellShape(void) =0 ;

//! Set mutable layer layout.
  virtual void enableMutableLayerDims(void) =0;

  virtual void enableExceptionValue(void*, size_t ,
              std::vector<char>* mode=0, std::string* s=0 ) =0;

/*
//! Error messages are written to a file. Exits immediately.
  void    exceptionError(std::string);
*/

//! Get reference to the grid-cell values (valarray).
  virtual MtrxArr<double>&
         getArea(void) =0 ;

//! Get i-th grid-cell area (valarray).
  virtual double getArea(size_t i) =0;

//! Get areas of grid-cells in matrix representation.
//  virtual double** const
//         getAreaM(void) =0;

//! Get val of i-th grid-cell.
//  virtual void* getCellValue(size_t, void*)=0;

//! Get value for the grid-cell containing 'lon', 'lat'.
//  virtual bool getCellValue( void*, double lon, double lat) =0;

//! Get cell values of current layer.
  virtual void*  getCellValue(void) =0;

//! Get weights of the areas of the grid-cells.
  virtual MtrxArr<double>*
         getCellWeight(void) =0;

//! Get weights of the areas of the grid-cells (matrix representation).
  virtual double** const
         getCellWeightM(void) =0;

//! Get grid-indexes.
/*! Return the indexes for 'lat', 'lon'. 'shift' inidicates
    a shift of lat, lon about half a grid-cell size. Modes of shift:
    empty for no shift, "pos" for a positve shift and "neg" for
    a negative shift. By default (backRotation==false), lat, lon
    and corresponding indixes belong to the same coordinate system.
    If set true, then lat, lon are given in the usual system, but
    the indices belong to a rotated system.*/
/*
  virtual void getCoordIndex(double lat, double lon,
            int *latIndx, int *lonIndx,
            std::string shift="" ,bool backRotation=false) =0;
*/

  virtual void* getData(void) =0;

//! Get area of the earth surface.
  //sphere with equal-surface to the geoid[m]
  virtual double getEarthSurface(void) =0;

//! Get area of cells with valid values
  virtual double getValidArea(void) =0;

//! Get index of total data in relation to layer coordinates.
  virtual size_t getIndex(size_t ilon, size_t ilat) =0;

//! Get area only of cells marked as missing|filling.
  virtual double getInvalidArea(void) =0;

//! Get start and end index of a layer of data
  virtual void getLayerIndex(size_t *beg, size_t *end) =0;

//! Get total area of the domain.
  virtual double getTotalArea(void) =0;

//! Any cells with defined area?
  virtual bool isLayer(void) =0;

//! Is the shape of cells from a regular grid?
  virtual bool inqRegularGrid(void) =0;

//! Is the layout of a layer mutable?
  virtual bool isLayerMutable(void) =0;

//! Reset to the first layer
  virtual void resetLayer(void) =0;

//! Calculate areas of cells
//  virtual void setArea(bool isGridOrig=false);

//! The coordinate system is back-rotated by default.
  virtual void setBackRotation(void) =0;

//! Set value of i-th cell to 'v'.
//  virtual void setCellValue(size_t i, void*) =0;

//! Set cell vertices.
/*! String is either lat|y or lon|x.
    If dimLat == dimLon == 0, then these dimensions must be
    already be known to the object. Else: there is a reset to
    the supplied dimensions.*/
  virtual void setVertices(std::string, void*, std::string type,
      size_t vrtSz, size_t vrtPos, size_t lat_sz=0, size_t lat_pos=0,
      size_t lon_sz=0, size_t lon_pos=0 ) =0;

//! Set a pointer to a portion (layer) of data (MtrxArr obj)
  virtual void setData( void*) =0;

//! Set radius of earth;
/*! Default: 6371007.176 m of equal-surface geoid*/
  virtual void setEarthRadius(double) =0;

//! Initialise cell properties.
/*! Create 2D layout for data,
    cellArea, cellAreaWeight.\n */
//  virtual void setLayer(std::string s, size_t sz, size_t pos=0) =0;

//! Set 2D layer properties.
  virtual void setLayer(std::vector<std::string> &nm,
             std::vector<size_t> &pos, std::vector<size_t> &sz) =0;

//! Set those dimensions that don't define a layer.
  virtual void setNonLayerDim(std::vector<size_t> sz, std::vector<size_t> pos) =0;


//  virtual void setOutputFormat(std::string format) =0;
//  virtual void setPrintFilename(std::string file) =0;

//! Set, if the vertices originate from a regular grid.
/*! Used to calculate areas and weights.*/
  virtual void setRegularGrid(bool is=true) =0;

//! Get number of grid-cells.
  virtual size_t size(void) =0;

/*
//! -------- Latitudes and longitude section--------- \n
//! Get latitudes of the bounds of a grid-cell.
/ *! Also for regions and also for back-rotation ('isBR'), i.e. the index
    is valid in the rotated system and the bounds in the back-rotated one.
    Default is: both index and latitude in the same system.
  MtrxArr<double>
         getCellLatVertices(size_t index, bool isBR=false)
           { return getCellLatLonVertices(true, index, isBR); }

//! Get indexes of latitude for the cells of a regular grid.
  MtrxArr<int>
         getCellLatIndex(void)
           { return getCellIndexLatLon(true); }

//! Get latitude of 'index'ed cell.
// / *! Also for regions and back-rotation.
  double getCellLatitude(size_t index,bool isBRot=false)
           { return getCellLatLon(true, index, isBRot); }

//! Get latitudes of cells in grid.
  MtrxArr<double>
         getCellLatitudes(bool isBRot=false)
           { return getCellLatLon(true, isBRot); }

//! Get longitudes of the bounds of the grid-cell with lon-'index'.
  MtrxArr<double>
         getCellLonVertices(size_t index, bool isBRot=false)
           { return getCellLatLonVertices(false, index, isBRot); }

//! Get index of longitude for a regular grid.
  MtrxArr<int>
         getCellLonIndex(void)
           { return getCellIndexLatLon(false);}

//! Get longitude of the bounds of a grid-cell.
  double getCellLongitude(size_t index, bool isBRot=false)
           { return getCellLatLon(false, index, isBRot);}

//! Get longitudes of cells in grid.
  MtrxArr<double>
         getCellLongitude(bool isBRot=false)
           { return getCellLatLon(false, isBRot); }
*/

//  void    accessData(T *arr);
//  void    accessData(MtrxArr<T>&);

//  void    printWithBorders(void);
//  void    printWithGeoCoord(void);
//  virtual bool testIdenticalMapping( const GeoMetaBase &) =0;

};

// -----------------------

template<typename T>
class GeoMeta : public GeoMetaBase
{
public:
//! Default constructor.
  GeoMeta();

//! Copy "constructor".
  GeoMeta( const GeoMeta& ); // copies all

  GeoMetaBase* clone(void)
    { return new GeoMeta(*this); }

//! Assign referenced object's data to *this.
  GeoMeta& operator  = ( const GeoMeta &);

//! Destructor.
  ~GeoMeta();


   struct ExceptionStruct
   {
      std::ofstream *ofsError;
      std::string strError;
   } ;

/*
//! Assign referenced object's data.
  void     operator  = ( const MtrxArr<T> &);
*/

/*
//! Add 'val' to the grid-cell with coordinates 'lat','lon'.
  void    add( void* p, double lat, double lon)
              { add(*reinterpret_cast<T*>(p), lat, lon); }
  void    add( T val, double lat, double lon);

//! Add 'val' to the cell[iLat][iLon].
  void    add( void* p, int ilat, int ilon)
              { add(*reinterpret_cast<T*>(p), ilat, ilon); }
  void    add( T val, int iLat, int iLon)
            {size_t rT=iLat;size_t cT=iLon;add(val,rT,cT);}

//! Add 'val' to the cell[iLat][iLon].
  void    add( T val, size_t iLat, size_t iLon);
*/

//! Get effective area based weights
/*! Return A_total / A_effective; zero if A_effective=0*/
  double  adaptWeight(void);

  void    adjustWeights(void);
  double  areaSphericalTriangle(size_t ilat, double arc, double rr);

/*
//! Assign 'val' to the cell which contains coordinates 'lat','lon'.
  void    assign( void* p, double lat, double lon)
              { assign(*reinterpret_cast<T*>(p), lat, lon); }
  void    assign( T val, double lat, double lon);

//! Assign 'val' to the cell[iLat][iLon].
  void    assign( void* p, int ilat, int ilon)
              { assign(*reinterpret_cast<T*>(p), ilat, ilon); }
  void    assign( T val, int iLat, int iLon)
            {size_t rT=iLat;size_t cT=iLon;assign(val,rT,cT);}
//! Assign 'val' to the cell[iLat][iLon].
  void    assign( T val, size_t rLat, size_t cLon);
*/

  void    calcRegularGridArea(void);
  void    calcAreaFromVertices(void);
  void    calcCellEquiWeights(void);
  void    calcCellWeights(void);

//! Clear previous settings and re-init.
  void    clear(void);

//! Clear previous settings.
  void    clearMapping();

  void    coordBackRotation(double fromLat, double fromLon,
             double *toLat, double *toLon);
  void    coordForwardRotation(double fromLat, double fromLon,
             double *toLat, double *toLon);

//  void    cpObject( const void* p )
//            { cpObject( * reinterpret_cast<GeoMeta<T>* >(p) );}
  void    cpObject( const GeoMeta<T>& );

//  void    cpState( const void* p )
//            { cpState( * reinterpret_cast<GeoMeta<T>* >(p) );}
  void    cpState( const GeoMeta<T>& );


//! Switch layer
/*! Link cellValue to the i-th layer of a 3-D object. If i exceeds
the top-most layer, then it is reset to the first one.*/
  bool    cycleLayers(void);

//! Disable/enable geographic cell area properties.
/*! Set sum of weights to unit, i.e. equal weights.*/
  void    disableCellShape(void){hasShape=true;}
  void    enableCellShape(void){hasShape=false;}

  void    enableExceptionValue(void* p, size_t sz,
              std::vector<char>* mode=0, std::string* s=0)
              { cellValue.valExcp->enableExceptionValue
                    (reinterpret_cast<T*>(p), sz, mode, s );}

//! Mutable layer layout.
  void    enableMutableLayerDims(void) {isImmutableLayer=false;}

  void    exceptionError(std::string);

//! Maximum/minimum, weighted ave, stnd dev., number of filling value
/*! occurrences of the current layer(s).
  Initialisation, processing, and getting the results are split.
  Usage: clear, add [, add, ...], get.*/
/*
  void fieldDataAdd(bool isWeighted=true)
            { dataProps.add(this, isWeighted);}
  void fieldDataClear(void)
            { dataProps.clear(this);}
  void fieldDataDisableChecksum(void)
            { dataProps.disableChecksum();}
  void fieldDataEnableChecksumWithClearedBits(size_t n)
            { dataProps.enableChecksumWithClearedBits(n);}
  hdhC::FieldData
       fieldDataGet(void)
            { return dataProps.get(this);}
*/
  MtrxArr<double>&
          getArea(void);
  double  getArea(size_t);

//  double** const
//          getAreaM(void);
/*
  MtrxArr<int>
          getCellIndexLatLon(bool isLat);
  double
          getCellLatLon(bool isLat, size_t index, bool isBRot);
  MtrxArr<double>
          getCellLatLonVertices(bool isLat, size_t index,
            bool isBRot) ;
  MtrxArr<double>
          getCellLatLon(bool isLat, bool isBRot);
*/

/*
//! Access to the grid-cell values.
  bool    getCellValue( T *val, double lat, double lon);
  void*   getCellValue(size_t i, void* p)
            { return reinterpret_cast<void*>(
              getCellValue(size_t i, reinterpret_cast<T*>(p) ) );}
  T       getCellValue(size_t i, T* p )
            { return data[ getIndex

  MtrxArr<T>&
          getCellValue(void);
*/

  void*   getCellValue(void)
             { return (void*) &cellValue ; }

  double** const
          getCellWeightM(void) ;
  MtrxArr<double>*
          getCellWeight(void);

/*
  void    getCoordIndex(double latRequest, double lonRequest,
             int *latIndx, int *lonIndx, std::string shift,
             bool myIsBackRot);
*/
  void*   getData(void)
             { return reinterpret_cast<void*>(data); }

  double  getEarthSurface(void);

//! Get index of total data in relation to layer coordinates.
  size_t  getIndex(size_t ilon, size_t ilat)
             { return layerOffset + ilat*lonSz + ilon ;}

  double  getInvalidArea(void);

//! Get start and end index of a layer of data
  void    getLayerIndex(size_t *beg, size_t *end)
             { *beg=layerOffset;
               *end=layerOffset+size_2D; return;}

//! Get area of cells with valid values
  double  getValidArea(void);

  double  getTotalArea(void);

  void    init(void);

//! Any cells with defined area?
  bool    isLayer(void){ return hasLayers;}

//! Is the shape of cells from a regular grid?
  bool    inqRegularGrid(void){ return isRegularGrid;}

//! Is the layout of a layer immutable?
  bool    isLayerMutable(void) { return !isImmutableLayer;}

/*
//! Assign referenced object's data to cellValue.
  void     operator  = ( const MtrxArr<T> &);

//! Assign the 'double' value to each grid-cell.
  GeoMeta<T>& operator  = ( T );

//! Adds data of the referenced object to *this.
  GeoMeta<T>& operator += ( GeoMeta<T> &);

//! Subtract data of the referenced object from *this.
  GeoMeta<T>& operator -= ( GeoMeta<T> &);

//! Multiply data of the referenced object to *this.
  GeoMeta<T>& operator *= ( GeoMeta<T> &);

//! Divide *this data by that of the referenced object.
  GeoMeta<T>& operator /= ( GeoMeta<T> &);

//! Add 'double' to each grid-cell.
  GeoMeta<T>& operator += ( T );

//! Subtract 'double' from each grid-cell.
  GeoMeta<T>& operator -= ( T );

//! Mutliply 'double' to each grid-cell.
  GeoMeta<T>& operator *= ( T );

//! Divide each grid-cell value by 'double'.
  GeoMeta<T>& operator /= ( T );
*/

/*
  void    print(void);
  void    print(std::vector<std::string>);
  void    printWithBorders(void);
  void    printWithGeoCoord(void);
*/

//! Calculate areas of cells
  void    setArea(bool isGridOrig=false);

//! The coordinate system is back-rotated by default.
  void    setBackRotation(void){isBackRotation=true;}

//! Set value of i-th cell to 'v'.
/*
  void setCellValue(size_t i, void* p)
         { setCellValue(i, *reinterpret_cast<T*>(p) );}
  void setCellValue(size_t i, T v)
         { cellValue[ getIndex(v,i) ] ;}
*/

  void    resetLayer( void);

  void    setData( void* p)
            { data = reinterpret_cast<MtrxArr<T> *>(p) ;}

//! Set radius of earth;
/*! Default: 6371007.176 m of equal-surface geoid*/
  void    setEarthRadius(double r){earthRadius=r;}

//! Set 2D layer properties.
  void    setLayer(std::vector<std::string> &nm,
             std::vector<size_t> &pos, std::vector<size_t> &sz);

//! Initialise cell properties.
/*! Create 2D layout for data,
    cellArea, cellAreaWeight.\n */
  void    setLayer(std::string, size_t sz, size_t pos=0);

//! Set those dimensions that don't define a layer.
  void    setNonLayerDim(std::vector<size_t> sz,
                          std::vector<size_t> pos);
/*
  void    setOutputFormat(std::string format);
  void    setPrintFilename(std::string file);
*/

//! Set, if the vertices originate from a regular grid.
/*! Used to calculate areas and weights.*/
  void setRegularGrid(bool is=true){ isRegularGrid=is;}

//! Set cell vertices.
/*! String is either lat|y or lon|x.
    If dimLat == dimLon == 0, then these dimensions must be
    already be known to the object. Else: there is a reset to
    the supplied dimensions.*/
  void    setVertices(std::string, void *v, std::string type,
             size_t vrtSz, size_t vrtPos, size_t lat_sz=0,
             size_t lat_pos=0, size_t lon_sz=0, size_t lon_pos=0 );

/*! Requires the derived name.*/
template <typename X>
  void    setVertices(std::string, MtrxArr<X> &v,
             size_t vrtSz, size_t vrtPos, size_t lat_sz=0,
             size_t lat_pos=0, size_t lon_sz=0, size_t lon_pos=0 );

/*
  void    setXtractDomain(std::string domain);
*/

  //! Get number of grid-cells.
  size_t size(void){ return sZ; }

  bool    testIdenticalMapping( const GeoMeta<T> &);

  double NUMERICAL_TOLERANCE ;

  bool isBackRotation;
  bool isImmutableLayer;
//  bool isGMT_MultiSegment;
  bool enabledPrint;
/*
  bool isPrintToFile;
  bool isPrintWithBorders;
  bool isPrintWithGeoCoord;
*/
  bool hasLayers;
  bool hasShape;
  bool isRegularGrid;
//  bool isStripMissingValue;

  size_t sZ ;  // size of data
  size_t size_2D; // size of layer (1D or 2D)
  size_t latSz, lonSz, vrtcSz ;
  size_t prevLatSz, prevLonSz;
  size_t latPos, lonPos, vrtcPos;
  std::vector<size_t> layerLevelDim;
  std::vector<size_t> layerLevelPos;

//  bool isScientific;
//  int  formatWidth;
//  int  formatPrecis;

  MtrxArr<double> cellArea ;
  MtrxArr<double> cellLatitude ;
  MtrxArr<double> cellLongitude ;
  MtrxArr<double> cellWeight ;  // weighted by total area

  // different sizes of vertices possible
  MtrxArr<double> cellLatVertices;
  MtrxArr<double> cellLonVertices;

  double prevEffAreaRatio;
  double effectiveArea;
  double totalArea;
  double earthRadius ;

  // in case of coordinate rotation
  double lonRotatedNorthPole;  // default for CLM
  double latRotatedNorthPole;

  // messaging in case of exceptions.
  struct ExceptionStruct xcptn;

  MtrxArr<T>* data;
  MtrxArr<T>  cellValue; // layer: rows x cols

  size_t               layerOffset;  // start of layer within data
  std::vector<size_t>  currLayer;
};

// SOURCES

template <typename T>
GeoMeta<T>::GeoMeta()
  : cellArea(), cellWeight(),
    cellLatVertices(),  cellLonVertices()
{
  init();
}

template <typename T>
GeoMeta<T>::~GeoMeta(void)
{
  clearMapping();
}


// copy constructor
template <typename T>
GeoMeta<T>::GeoMeta( const GeoMeta<T> &g )
  : cellArea(), cellWeight(),
    cellLatVertices(),  cellLonVertices()
{
  cpObject(g);
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator = ( const GeoMeta &g)
{
  if( this == &g )
    return *this;

  GeoMeta *p = const_cast<GeoMeta*>(&g);
  cpObject(*reinterpret_cast<GeoMeta<T> *>(p) );

  return *this;
}

template<typename T>
double
GeoMeta<T>::adaptWeight(void)
{
  double aw=1.;

  if( cellValue.isValid() )
  {
    aw = prevEffAreaRatio;
    prevEffAreaRatio = 1.;
    return aw;
  }

  effectiveArea=0.;

  size_t lim=cellValue.validRangeBegin.size();
  for( size_t k=0 ; k < lim ; k++ )
     for(size_t i=cellValue.validRangeBegin[k] ;
            i < cellValue.validRangeEnd[k] ; ++i)
       effectiveArea += cellArea[i] ;

  if( effectiveArea > 0. )
  {
    aw = prevEffAreaRatio * totalArea / effectiveArea ;
    prevEffAreaRatio = effectiveArea / totalArea;
  }

  return aw;
}

/*
template<typename T>
void
GeoData<T>::add( T val, double lat, double lon)
{
  // period
  if( lon == lonGeoEnd )
    lon = lonGeoBeg ;

  int myLatIndx=static_cast<int>(
            (lat - latGeoBegDataSet)/latStep - latBegIndx ) ;
  int myLonIndx=static_cast<int>(
            (lon - lonGeoBegDataSet)/lonStep - lonBegIndx ) ;

  // outside of considered lon-lat-range
  if( myLatIndx >= latSz || myLonIndx >= lonSz )
    return ;
  if( myLatIndx < 0 || myLonIndx < 0 )
    return ;

  if( isFillingValueFound )
    if( ! isFillingValue( cellValue->index(myLatIndx, myLonIndx) ) )
      cellValue->[ cellValue->index(myLatIndx, myLonIndx) ] += val ;
  else
    cellValue->[ cellValue->index(myLatIndx, myLonIndx) ] += val ;

  return;
}
*/

template<typename T>
void
GeoMeta<T>::adjustWeights(void)
{
  if( cellWeight.size() == 0 || !isImmutableLayer )
  {
    if( hasShape )
      calcCellWeights();
    else
      calcCellEquiWeights();

    prevEffAreaRatio=1;
  }

  cellWeight *= adaptWeight();

  return;
}

template<typename T>
double
GeoMeta<T>::areaSphericalTriangle(size_t ix,
    double arc, double rr)
{
  // arc = pi / 180.
  // rr is square of earth radius

  double A=0.;  //accumulated area of a polygon

  size_t iv0, iv1;
  double phi[vrtcSz+1];
  double alf[vrtcSz+1];
  double gca[vrtcSz];  // piece of a great circle arc

  // Only for the first triangle of the polygon.
  phi[0]=cellLatVertices.get(ix, 0) * arc ;
  alf[0]=cellLonVertices.get(ix, 0) * arc ;
  phi[2]=cellLatVertices.get(ix, 1) * arc ;
  alf[2]=cellLonVertices.get(ix, 1) * arc ;

  double dphiSin=sin( (phi[2] - phi[0])*0.5 );
  dphiSin *= dphiSin;
  double dalfSin=sin( (alf[2] - alf[0])*0.5 );
  dalfSin *= dalfSin;

  // Each triangle of the polygon starts from the same coordinate.
  phi[3] = phi[0];
  alf[3] = alf[0];

  // the last side of a triangle will become the first
  // of the next triangle
  gca[2] = dphiSin + cos(phi[0])*cos(phi[2]) * dalfSin;
  gca[2] = 2.*atan2( sqrt(gca[2]), sqrt(1.-gca[2]) );

  for( size_t j=1 ; j < vrtcSz-1 ; ++j)
  {
    // area for a spherical triangle
    phi[1]=phi[2];
    alf[1]=alf[2];
    phi[2]=cellLatVertices.get(ix, j+1) * arc ;
    alf[2]=cellLonVertices.get(ix, j+1) * arc ;

    double s = gca[0] = gca[2] ;
    for( iv0=1 ; iv0 < 3 ; ++iv0)
    {
       // using the Haversine formula
       iv1=iv0+1;
       dphiSin=sin( (phi[iv1] - phi[iv0])*0.5 );
       dphiSin *= dphiSin;
       dalfSin=sin( (alf[iv1] - alf[iv0])*0.5 );
       dalfSin *= dalfSin;

       gca[iv0] = dphiSin + cos(phi[iv0])*cos(phi[iv1]) * dalfSin;
       gca[iv0] = 2.*atan2( sqrt(gca[iv0]), sqrt(1.-gca[iv0]) );
       s += gca[iv0] ;
    }
    s *= 0.5 ;

    double excess=tan(s*0.5);
    for( iv0=0 ; iv0 < 3 ; ++iv0)
       excess *= tan((s-gca[iv0])*0.5) ;

    excess = 4. * atan( sqrt( excess ) );
    A += excess * rr ;
  }

  return A ;
}

/*
template<typename T>
void
GeoData<T>::assign( T val, double lat, double lon)
{
  if( lon == lonGeoEnd )
    lon = lonGeoBeg ;

  int myLatIndx=static_cast<int>(
            (lat - latGeoBegDataSet)/latStep - latBegIndx ) ;
  int myLonIndx=static_cast<int>(
            (lon - lonGeoBegDataSet)/lonStep - lonBegIndx ) ;

  // outside of considered lon-lat-range
  if( myLatIndx >= latSz || myLonIndx >= lonSz )
    return ;
  if( myLatIndx < 0 || myLonIndx < 0 )
    return ;

  size_t index=cellValue->index(myLatIndx, myLonIndx);

  if( isFillingValueFound && isFillingValue( val ) )
    cellValue->[ index ] = 0. ;
  else
    cellValue->[ index ] = val ;

  return;
}
*/

/*
template<typename T>
void
GeoData<T>::assign( T val, size_t tLat, size_t tLon)
{
  cellValue->[ cellValue->index(tLat, tLon) ] = val;

  return ;
}
*/

template<typename T>
void
GeoMeta<T>::calcRegularGridArea(void)
{
  cellArea.resize(latSz, lonSz) ;

  // calculate the area of each grid box by the difference between
  // spherical caps at lat0 and lat1 (normalised by
  // longitude_grid_size / 360).
  double rr2pi=2.*M_PI*earthRadius*earthRadius ;  // helper variable

  totalArea = 0.;

  double arc=M_PI/180. ;
  double lat0;
  double lat1 = cellLatVertices.get(0,0,0);
  double lonStep
        = cellLonVertices.get(0,0,1) - cellLonVertices.get(0,0,0);

  for( size_t ltx=0 ; ltx < latSz ; ++ltx)
  {
    lat0 = lat1;
    lat1 = cellLatVertices.get(ltx,0,3);
    double A=rr2pi * fabs( sin(lat0*arc) - sin(lat1*arc) );

    A *= (lonStep / 360.);

    // sum area
    for( size_t lngx=0 ; lngx < lonSz ; ++lngx)
    {
      cellArea.put(A, ltx,lngx) ;
      totalArea += A ;
    }
  }

  return ;
}

template<typename T>
void
GeoMeta<T>::calcAreaFromVertices(void)
{
  cellArea.resize(size_2D) ;

  // calculate the area of each spherical polygon cell
  totalArea = 0.;

  // calculate once and pass as parameter
  double arc=M_PI/180. ;
  double rr=earthRadius*earthRadius ;  // helper variable

  for( size_t i=0 ; i < size_2D ; ++i)
  {
    double A = areaSphericalTriangle(i, arc, rr);

    cellArea.put(A, i) ;
    totalArea += A ;
  }

  return ;
}

template<typename T>
void
GeoMeta<T>::calcCellEquiWeights(void)
{
    if( prevLatSz == latSz && prevLonSz == lonSz )
      return;

    cellWeight.resize(latSz, lonSz) ;

    double w = 1. / static_cast<double>(size_2D);

    cellWeight = w ;

    return;
}

template<typename T>
void
GeoMeta<T>::calcCellWeights(void)
{
  // calculation of areas
  if( totalArea == 0. || !isImmutableLayer )
    setArea( isRegularGrid );

  cellWeight.resize(latSz, lonSz) ;

  for( size_t i=0 ; i < size_2D ; ++i)
     cellWeight.arr[i] = cellArea.arr[i] / totalArea ;

  return ;
}

template<typename T>
void
GeoMeta<T>::clear(void)
{
  clearMapping();
  init();

  return;
}

template<typename T>
void
GeoMeta<T>::clearMapping(void)
{
  cellArea.clear();
  cellWeight.clear();
  cellLatVertices.clear();
  cellLonVertices.clear();

  return;
}

template<typename T>
void
GeoMeta<T>::coordBackRotation(double fromLat, double fromLon,
          double *toLat, double *toLon)
{
   static const double toArc=M_PI / 180.;
   static const double fromArc=180. / M_PI;

   static const double sinLatNP=sin( latRotatedNorthPole );
   static const double cosLatNP=cos( latRotatedNorthPole );
   static const double sinLonNP=sin( lonRotatedNorthPole );
   static const double cosLonNP=cos( lonRotatedNorthPole );

   fromLat *= toArc;
   fromLon *= toArc;

   *toLat = asin (
      cosLatNP*cos(fromLat)*cos(fromLon) + sinLatNP*sin(fromLat)
                 ) * fromArc ;

   *toLon = atan2(
      sinLonNP*( cosLatNP*sin(fromLat)
                  - sinLatNP*cos(fromLon)*cos(fromLat) )
             - cosLonNP*sin(fromLon)*cos(fromLat)
      ,
      cosLonNP*( cosLatNP*sin(fromLat)
                  -  sinLatNP*cos(fromLon)*cos(fromLat) )
             +  sinLonNP*sin(fromLon)*cos(fromLat)
                 ) * fromArc ;
  return;
}

template<typename T>
void
GeoMeta<T>::coordForwardRotation(double fromLat, double fromLon,
          double *toLat, double *toLon)
{
   static const double toArc=M_PI / 180.;
   static const double fromArc=180. / M_PI;

   static const double sinLatNP=sin( latRotatedNorthPole );
   static const double cosLatNP=cos( latRotatedNorthPole );
   static const double sinLonNP=sin( lonRotatedNorthPole );
   static const double cosLonNP=cos( lonRotatedNorthPole );

   fromLat *= toArc;
   fromLon *= toArc;

   double tmpCos=cos(fromLon - lonRotatedNorthPole);

   *toLat = asin (
      sinLatNP*sin(fromLat) + cosLatNP*cos(fromLat)*tmpCos
                 ) * fromArc ;

   *toLon = atan(
      (cos(fromLat)*sin(fromLon - lonRotatedNorthPole))/
        (cos(fromLat)*sinLatNP*tmpCos - sin(fromLat)*cosLatNP)
                 ) * fromArc ;
  return;
}

template<typename T>
void
GeoMeta<T>::cpObject( const GeoMeta<T> &g )
{
  cpState( g );

  if( g.cellArea.size() )
  {
    cellArea.resize(latSz, lonSz) ;
    cellArea = g.cellArea ;
  }

  if( g.cellWeight.size() )
  {
    cellWeight.resize(latSz, lonSz) ;
    cellWeight = g.cellWeight ;
  }

  if( g.cellLatVertices.size() )
     cellLatVertices =  g.cellLatVertices;

  if( g.cellLonVertices.size() )
     cellLonVertices =  g.cellLonVertices;

  return ;
}

template<typename T>
void
GeoMeta<T>::cpState( const GeoMeta<T> &g )
{
  size_2D = g.size_2D;
  cellValue = g.cellValue;
  data = g.data;

  NUMERICAL_TOLERANCE=g.NUMERICAL_TOLERANCE;

  isBackRotation=           g.isBackRotation;
/*
  isGMT_MultiSegment=       g.isGMT_MultiSegment;
  enabledPrint=             g.enabledPrint;
  isPrintWithBorders=       g.isPrintWithBorders;
  isPrintWithGeoCoord=      g.isPrintWithGeoCoord;
  isPrintToFile=            g.isPrintToFile;
  isScientific=             g.isScientific;
*/
  earthRadius=              g.earthRadius;
//  formatWidth=              g.formatWidth;
//  formatPrecis=             g.formatPrecis;

  latSz=                    g.latSz;
  lonSz=                    g.lonSz;
  latPos=                   g.latPos;
  lonPos=                   g.lonPos;
  vrtcSz=                   g.vrtcSz;
  vrtcPos=                  g.vrtcPos;
  sZ=                       g.sZ;

  lonRotatedNorthPole=      g.lonRotatedNorthPole;
  latRotatedNorthPole=      g.latRotatedNorthPole;

//  outFile=                  g.outFile;
  totalArea=                g.totalArea;

  return;
}

template <typename T>
bool
GeoMeta<T>::cycleLayers(void)
{
/*
  if( sZ == 0 )
    return; // error handling
*/

/*
  if( currLayer.size() == 0 )
  {
error
  }
*/

  // Dimensions must be ordered from left to right aligned
  // to the layer dim(s).

  // return == false, when a full circle was reached.

  layerOffset=0;

  // When there is just a single layer at all, then
  // the size of dims == 0
  if( ! layerLevelDim.size() )
  {
     // full cycle; do it again
     resetLayer() ;
     return false;
  }

  // find, from right to left, the first non-layer dimension
  // that didn't reach maximum, yet.
  for( size_t i=currLayer.size()-1 ;  ; --i )
  {
    ++currLayer[i] ;

    if( currLayer[i] < layerLevelDim[i] )
      break;

    // maximum was exceeded; thus
    // reset this and the subsequent dimensions
    for( size_t j=i ; j < currLayer.size() ; ++j )
      currLayer[j] = 0 ;

    if( i == 0 )
    {
       // full cycle; do it again
       resetLayer() ;
       return false;
    }
  }

  // get the offset
  size_t fact  ;
  for( size_t j=0; j < currLayer.size() ; ++j )
  {
    fact=1;
    // note: a virtual layerLevelDim == 1 was appended
    for( size_t k=j+1; k < layerLevelDim.size() ; ++k )
       fact *= layerLevelDim[k];

    layerOffset += currLayer[j] * fact ;
  }

  layerOffset *= size_2D;

  cellValue.frame(*data, layerOffset, size_2D) ;

  return true ;
}

template<typename T>
void
GeoMeta<T>::exceptionError(std::string str)
{
  // Occurrence of an error stops the run immediately.
  xcptn.ofsError=0;
  xcptn.strError = "error_GEO_META.txt" ;

  // open file for appending data
  if( ! xcptn.ofsError )
    xcptn.ofsError
       = new std::ofstream(xcptn.strError.c_str(), std::ios::app);

  *xcptn.ofsError << str << std::endl;

  exit(1);
  return ;
}

template<typename T>

MtrxArr<double>&
GeoMeta<T>::getArea(void)
{
  if( size_2D )
    return cellArea ;

  std::ostringstream ostr(std::ios::app);
  ostr << "GeoMeta<T>::getArea()"
       << "\nno data set was read" ;
  exceptionError( ostr.str() );

  return cellArea ;
}

template<typename T>
double
GeoMeta<T>::getArea(size_t i)
{
  if( cellArea.size() == 0 )
    setArea( isRegularGrid );

  if( i )
    return cellArea.arr[i] ;
  else
    return totalArea ;
}

/*
template<typename T>
double** const
GeoMeta<T>::getAreaM(void)
{
  if( cellArea.size() == 0 )
    setArea( isRegularGrid );

  return cellArea.getM();
}
*/

/*
MtrxArr<int>
GeoMeta<T>::getCellIndexLatLon(bool isLat)
{
  MtrxArr<int> mv;

  return mv;  // not for the domain
}
*/

/*
double
GeoMeta<T>::getCellLatLon(bool isLat, size_t index, bool isBRot)
{
  size_t domainIndex=index;
  double lat, lon;

  // domain
  lat=cellLatVertices[index];
  lon=cellLonVertices[index] ;

  if( isBRot )
  {
    double lon=0.;
    coordBackRotation( lat, lon, &lat, &lon );
  }

  if( isBRot )
    coordBackRotation( lat, lon, &lat, &lon );

  if( isLat )
    return lat;
  else
    return lon;
}
*/

/*
MtrxArr<double>
GeoMeta<T>::getCellLatLonVertices(bool isLat,
        size_t index, bool isBRot)
{
  MtrxArr<double> cLatB(2);
  MtrxArr<double> cLonB(2);

  cLatB[0]=cellLatVertices[index] - latGridSize/2.;
  cLatB[1]=cellLatVertices[index] + latGridSize/2.;
  cLonB[0]=cellLonVertices[index] - lonGridSize/2.;
  cLonB[1]=cellLonVertices[index] + lonGridSize/2.;

  // return back-rotated coords.
  if( isBRot )
  {
    coordBackRotation( cLatB[0], cLonB[0], &cLatB[0], &cLonB[0] );
    coordBackRotation( cLatB[1], cLonB[1], &cLatB[1], &cLonB[1] );
  }

  if( isLat )
    return cLatB ;
  else
    return cLonB ;
}
*/

/*
MtrxArr<double>
GeoMeta<T>::getCellLatLon(bool isLat, bool isBRot)
{
  if( isBRot )
  {
    MtrxArr<double> lat(cellLatVertices);
    MtrxArr<double> lon(cellLonVertices);

    for(size_t i=0 ; i < lat.size() ; ++i)
      coordBackRotation( lat[i], lon[i], &lat[i], &lon[i] );

    if( isLat )
     return lat;
    else
     return lon;
  }
  else
  {
    if( isLat )
      return cellLatVertices ;
    else
      return cellLonVertices ;
  }
}
*/

/*
template<typename T>
bool
GeoMeta<T>::getCellValue( T *val, double lat, double lon)
{
  // Get value from cell with lat/lon inside.
  // It is not checked whether edges between vertices cross
  // (must not). Also, lat/lon epsilon-close to edges or
  // vertices is no treated robust.

  size_t k;
  bool isInside;

  for( k=0 ; k < size_2D ; ++k )
  {
    std::vector<double> &vlat = cellLatVertices[k];
    std::vector<double> &vlon = cellLonVertices[k];
    size_t szL=vlat.size();

    isInside=false;
    size_t i, j;
    for ( i = 0, j = szL-1; i < szL; j = i++)
      // Next line: Copyright (c) 1970-2003, Wm. Randolph Franklin
      if ( ((vlat[i] > lat) != (vlon[j] > lat))
             && (lon < (vlon[j]-vlon[i]) * (lat-vlat[i])
                   / (vlat[j]-vlat[i]) + vlon[i]) )
         isInside = !isInside;

     if( isInside )
       *val = data[ layerOffset+k ] ;
  }

  return false ;  // no cell found
}
*/

/*
template<typename T>
MtrxArr<T>&
GeoMeta<T>::getCellValue(void)
{
  if( data->size() )
    return data ;

  std::ostringstream ostr(std::ios::app);
  ostr << "GeoData::getCellValue()"
       << "\nno data set was read" ;
  exceptionError( ostr.str() );

  return data ;
}
*/

template<typename T>
double** const
GeoMeta<T>::getCellWeightM(void)
{
  adjustWeights();

  return cellWeight.getM();
}

template<typename T>
MtrxArr<double>*
GeoMeta<T>::getCellWeight(void)
{
  if( ! hasShape )
    return 0;

  adjustWeights();

  return &cellWeight ;
}

/*
template<typename T>
void
GeoMeta<T>::getCoordIndex(double latRequest, double lonRequest,
    int *latIndx, int *lonIndx, std::string shift, bool myIsBackRot)
{
  // This method might be called with coordinates in a
  // back rotated coord-system, wherein data of *this
  // is rotated. Then, the indexes
  // in the (forward)rotated system are returned to the caller.
  // The requested lon and lat coordinates might be shifted by
  // the half-width of a cell. If shift=="pos", then
  // with a positive value, if shift =="neg", then opposed, and
  // an empty string indicates "no shift"

  double lon, lat;
  if( myIsBackRot )
     coordForwardRotation( latRequest, lonRequest, &lat, &lon);
  else
  {
     lat=latRequest;
     lon=lonRequest;
  }

  if( shift == "neg" )
  {
    lat -= 0.5 * latGridSize;
    lon -= 0.5 * lonGridSize;
  }
  else if( shift == "pos" )
  {
    lat += 0.5 * latGridSize;
    lon += 0.5 * lonGridSize;
  }

  if( lon == lonGeoEnd )
    lon = lonGeoBeg ;

  *latIndx=static_cast<int>(
            (lat - latGeoBegDataSet)/latStep - latBegIndx ) ;
  *lonIndx=static_cast<int>(
            (lon - lonGeoBegDataSet)/lonStep - lonBegIndx ) ;

  // outside of considered lon-lat-range
  if( *latIndx >= latSz || *latIndx < 0 )
  {
    *latIndx=-1;
    return ;
  }
  if( *lonIndx >= lonSz || *lonIndx < 0 )
  {
    *lonIndx=-1;
    return ;
  }

  return;
}
*/

template<typename T>
double
GeoMeta<T>::getEarthSurface(void)
{
  return 4.*M_PI*earthRadius*earthRadius; //equal-surface [m]
}

template<typename T>
double
GeoMeta<T>::getInvalidArea(void)
{
  if( cellArea.size() == 0 )
    setArea( isRegularGrid );

  return (totalArea - getValidArea() ) ;
}


template<typename T>
double
GeoMeta<T>::getTotalArea(void)
{
  if( cellArea.size() == 0 )
    setArea( isRegularGrid );

  return totalArea ;
}

template<typename T>
double
GeoMeta<T>::getValidArea(void)
{
  if( cellArea.size() == 0 )
    setArea( isRegularGrid );

  if( cellValue.isValid() )
    return totalArea ;

  double validArea = 0.;

  size_t lim=cellValue.validRangeBegin.size();
  for( size_t k=0 ; k < lim ; k++ )
     for(size_t i=cellValue.validRangeBegin[k] ;
             i < cellValue.validRangeEnd[k] ; ++i)
       validArea += cellArea[i] ;

  return validArea ;
}

template<typename T>
void
GeoMeta<T>::init(void)
{
//  NUMERICAL_TOLERANCE=hdhC::NUMERICAL_TOLERANCE ;
  NUMERICAL_TOLERANCE=.0001 ;

  enabledPrint=false;
  hasShape=false;
  hasLayers=false;
  isBackRotation=false;
  isImmutableLayer=true;
  isRegularGrid=false;

//  isGMT_MultiSegment=false;
/*
  isPrintWithBorders=false;
  isPrintWithGeoCoord=false;
  isPrintToFile=false;
*/

/*
  isScientific=true;
  formatWidth=14;
  formatPrecis=6;
*/

  earthRadius=6371007.176; // [m] sphere with equal-surface to geoid
  totalArea=0.;

  lonSz=0;
  latSz=0;
  lonPos=0;
  latPos=0;
  vrtcSz=0;
  vrtcPos=0;
  sZ=0;

  layerOffset=0;
  currLayer.push_back(0);

  prevLatSz = prevLonSz = 0;

  lonRotatedNorthPole=-162. * M_PI/180.;  // default for CLM
  latRotatedNorthPole=39.25  * M_PI/180. ;

  size_2D=0;

  prevEffAreaRatio = 1.;

  return ;
}

/*
template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator = ( T val)
{
  for(size_t i=0 ; i < sZ ; ++i )
    cellValue->arr[i] = val ;

  return *this;
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator += ( GeoMeta<T> &g)
{
  if( ! testIdenticalMapping(g))
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::operator +=()"
         << "\nthe mapping is not identical" ;
    exceptionError( ostr.str() );
  }

  size_t sz=cellValue->size();

  if( isFillingValueFound || g.isFillingValueFound )
  {
    for( size_t i=0 ; i < sz ; ++i)
    {
      if( (cellValue->arr[i] == fillingValue)
             || ( g.cellValue->arr[i] == g.fillingValue ) )
        continue;

      cellValue->arr[i] += g.cellValue->arr[i] ;
    }
  }
  else
    for( size_t i=0 ; i < sz ; ++i)
      cellValue->arr[i] += g.cellValue->arr[i] ;

  return *this;
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator -= ( GeoMeta<T> &g)
{
  if( ! testIdenticalMapping(g))
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::operator -=()"
         << "\nthe mapping is not identical" ;
    exceptionError( ostr.str() );
  }

  size_t sz=cellValue->size();

  if( isFillingValueFound )
  {
    for( size_t i=0 ; i < sz ; ++i)
    {
        if( isFillingValue( cellValue->arr[i] )
             || g.isFillingValue( g.cellValue->arr[i] ) )
          continue;

        cellValue->arr[i] -= g.cellValue->arr[i] ;
    }
  }
  else
    for( size_t i=0 ; i < sz ; ++i)
        cellValue->arr[i] -= g.cellValue->arr[i] ;

  return *this;
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator *= ( GeoMeta<T> &g)
{
  if( ! testIdenticalMapping(g))
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::operator *=()"
         << "\nthe mapping is not identical" ;
    exceptionError( ostr.str() );
  }

  size_t sz=cellValue->size();

  if( isFillingValueFound )
  {
    for( size_t i=0 ; i < sz ; ++i)
    {
        if( isFillingValue( cellValue->arr[i] )
             || g.isFillingValue( g.cellValue->arr[i] ) )
          continue;

        cellValue->arr[i] *= g.cellValue->arr[i] ;
    }
  }
  else
    for( size_t i=0 ; i < sz ; ++i)
        cellValue->arr[i] *= g.cellValue->arr[i] ;

  return *this;
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator /= ( GeoMeta<T> &g)
{
  if( ! testIdenticalMapping(g))
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::operator /=()"
         << "\nthe mapping is not identical" ;
    exceptionError( ostr.str() );
  }

  size_t sz=cellValue->size();

  if( isFillingValueFound )
  {
    for( size_t i=0 ; i < sz ; ++i)
    {
        if( (cellValue->arr[i] == fillingValue)
             || (g.cellValue->arr[i] == g.fillingValue ) )
          continue;

        if( g.cellValue->arr[i] == 0. )
          cellValue->arr[i] = fillingValue;
        else
          cellValue->arr[i] /= g.cellValue->arr[i] ;
    }
  }
  else
  {
    for( size_t i=0 ; i < sz ; ++i)
    {
       if( g.cellValue->arr[i] == 0. )
       {
         cellValue->arr[i] = fillingValue;
         isFillingValueFound=true;
       }
       else
         cellValue->arr[i] /= g.cellValue->arr[i] ;
    }
  }

  return *this;
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator += ( T val)
{
  if( val == 0. )
    return *this;

  size_t sz=cellValue->size();

  for( size_t i=0 ; i < sz ; ++i)
    if( cellValue->arr[i] != fillingValue )
      cellValue->arr[i] += val ;

  return *this;
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator -= ( T val)
{
  if( val == 0. )
    return *this;

  size_t sz=cellValue->size();

  for( size_t i=0 ; i < sz ; ++i)
    if( cellValue->arr[i] != fillingValue )
      cellValue->arr[i] -= val ;

  return *this;
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator *= ( T val)
{
  if( val == 1. )
    return *this;

  size_t sz=cellValue->size();

  for( size_t i=0 ; i < sz ; ++i)
    if( cellValue->arr[i] != fillingValue )
      cellValue->arr[i] *= val ;

  return *this;
}

template<typename T>
GeoMeta<T>&
GeoMeta<T>::operator /= ( T val)
{
  if( val == 1. )
    return *this;

  if( val == 0. )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::operator /=()"
         << "\ndivision by zero" ;
    exceptionError( ostr.str() );
  }

  size_t sz=cellValue->size();

  if( isFillingValueFound )
  {
    for( size_t i=0 ; i < sz ; ++i)
    {
      if( (cellValue->arr[i] == fillingValue)
             || ( val == static_cast<T>(0) ) )
        continue;

      cellValue->arr[i] /= val ;
    }
  }
  else
    for( size_t i=0 ; i < sz ; ++i)
      if( val != static_cast<T>(0) )
        cellValue->arr[i] /= val ;

  return *this;
}
*/

/*
template<typename T>
void
GeoMeta<T>::print(void)
{
  if( cellValue->size() == 0 )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::print()"
         << "\nread data set at first";
    exceptionError( ostr.str() );
  }

  if( isPrintWithBorders && isPrintWithGeoCoord )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::print()"
         << "\nconflict: isPrintWithBorders && isPrintWithGeoCoord == true";
    exceptionError( ostr.str() );
  }
  else if( isPrintWithBorders )
    printWithBorders();
  else if( isPrintWithGeoCoord || isGMT_MultiSegment )
    printWithGeoCoord();

  return;
}
*/

///   GMT_coastline, GMT_multi_segment, with_borders,
///   with_coordinates, filename=name-of-file, and
///   output_format=format-string.
/// Syntax of format-string: width.precision[X].
/// If X is d or D,scientific format is disabled.

/*
template<typename T>
void
GeoMeta<T>::print(std::vector<std::string> s)
{
  // save current settings
  bool save_isPrintWithBorders   = isPrintWithBorders;
  bool save_isPrintWithGeoCoord  = isPrintWithGeoCoord;
  bool save_isPrintGMT_CoastLine = isPrintGMT_CoastLine;
  bool save_isGMT_MultiSegment   = isGMT_MultiSegment;

  std::string save_outFile = outFile;
  bool save_isPrintToFile  = isPrintToFile;
  bool save_enabledPrint       = enabledPrint;

  bool save_isScientific = isScientific;
  int save_formatWidth   = formatWidth;
  int save_formatPrecis  = formatPrecis;

  if( cellValue->size() == 0 )
  {
      std::ostringstream ostr(std::ios::app);
      ostr << "GeoData::print()";
      ostr << "\nread data first";

      exceptionError( ostr.str() ); // exits
  }

  for( size_t i=0 ; i < s.size() ; ++i)
  {
     if( s[i] == "with_borders" )
     {
       isPrintWithBorders=true;
       enabledPrint=true;
       continue;
     }
     if( s[i] == "with_coordinates" )
     {
       isPrintWithGeoCoord=true;
       enabledPrint=true;
       continue;
     }
     if( s[i] == "GMT_coastline" )
     {
       isPrintGMT_CoastLine=true;
       enabledPrint=true;
       continue;
     }
     if( s[i] == "GMT_multi_segment" )
     {
       isGMT_MultiSegment=true;
       enabledPrint=true;
       continue;
     }
     if( s[i][0] == 'f' )  // filename
     {
       size_t pos=s[i].find('=');
       if( pos == std::string::npos )
       {
         std::ostringstream ostr(std::ios::app);
         ostr << "GeoData::print()";
         ostr << "\nmissing filename in "
              << s[i] ;

         exceptionError( ostr.str() ); // exits
       }
       setPrintFilename(s[i].substr(pos+1));
       continue;
     }
     if( s[i][0] == 'o' ) // output_format
     {
       size_t pos=s[i].find('=');
       if( pos == std::string::npos )
       {
         std::ostringstream ostr(std::ios::app);
         ostr << "GeoData::print()";
         ostr << "\nmissing output format";

         exceptionError( ostr.str() ); // exits
       }
       setOutputFormat(s[i]);
       continue;
     }

     // unknown option
     std::ostringstream ostr(std::ios::app);
     ostr << "GeoData::print()";
     ostr << "\nunknown arg: " << s[i] ;

     exceptionError( ostr.str() ); // exits
  }

  // this block performs printing.
  if( isPrintWithBorders && isPrintWithGeoCoord )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::print()"
         << "\nconflict: isPrintWithBorders && isPrintWithGeoCoord == true";
    exceptionError( ostr.str() );
  }
//  else if( isPrintWithBorders )
//    printWithBorders();
  else if( isPrintWithGeoCoord || isGMT_MultiSegment )
    printWithGeoCoord();

  // reset
  isPrintWithBorders   =save_isPrintWithBorders;
  isPrintWithGeoCoord  =save_isPrintWithGeoCoord;
  isPrintGMT_CoastLine =save_isPrintGMT_CoastLine;
  isGMT_MultiSegment   =save_isGMT_MultiSegment;

  outFile=save_outFile;
  isPrintToFile=save_isPrintToFile;
  enabledPrint=save_enabledPrint;

  isScientific  = save_isScientific;
  formatWidth   = save_formatWidth;
  formatPrecis  = save_formatPrecis;

  return;
}
*/

/*
template<typename T>
void
GeoMeta<T>::printWithBorders(void)
{
  // Output Umleitung
  // der std::cout Buffer wird am Ende von main wiederhergestellt

  std::streambuf* cout_sbuf = std::cout.rdbuf();
  std::ofstream *out ;

  if( isPrintToFile )
  {
    out = new std::ofstream ;
    out->open( outFile.c_str() );
    std::cout.rdbuf(out->rdbuf());
  }
  // print frame: rows: w -> e, cols: n -> s

  if( isScientific )
    std::cout.setf( std::ios::scientific, std::ios::floatfield);
  else
    std::cout.setf( std::ios::fixed, std::ios::floatfield);

  // print entire border values for longitudes
    std::cout << "\n        " ;
    for( int lngx=0 ; lngx < lonSz ; ++lngx)
    {
      std::cout << std::setw(formatWidth)<<std::setprecision(formatPrecis);
      std::cout << std::setw(1)<< ' ' ;
      std::cout << std::setw(formatWidth)
                << (cellLonVertices[lngx]-0.5*lonStep) << " |" ;
    }

    std::cout << std::setw(1)<< ' ' ;
    std::cout <<  std::setw(formatWidth);
    std::cout << (cellLonVertices[lonSz-1]+0.5*lonStep) << "\n" ;
    std::cout << std::setw(1) << "        " ;

    for( size_t lngx=0 ; lngx <= lonSz ; ++lngx)
      std::cout << "++++++++++" ;
    std::cout << "\n" ;
    std::cout << std::setw(formatWidth)
              << (cellLatVertices[0]-0.5*latStep);
    std::cout << std::setw(1) << " +\n" ;

    for( size_t ltx=0 ; ltx < latSz ; ++ltx)
    {
       std::cout << "        +   " ;
       for( size_t lngx=0 ; lngx < lonSz ; ++lngx)
       {
         isFillingValue( cellValue->index(ltx, lngx) ) ?
             std::cout << std::setw(formatWidth) << "     -    "
                : std::cout << ' '
                            << std::setw(formatWidth)
                            << cellValue->[ cellValue->index(ltx, lngx) ]
                            << std::setw(1) << "  " ;
       }
       std::cout << "\n"
                 << std::setw(formatWidth)
                 <<(cellLatVertices[ltx]+0.5*latStep)
                 << std::setw(1) << " +\n" ;
    }

    std::cout << std::endl ;

  if( isPrintToFile )
  {
    std::cout.rdbuf(cout_sbuf); // restore the original stream buffer
    delete out;
  }

  return ;
}
*/

/*
template<typename T>
void
GeoMeta<T>::printWithGeoCoord(void)
{
  // Output Umleitung
  // der std::cout Buffer wird am Ende von main wiederhergestellt

  std::streambuf* cout_sbuf = std::cout.rdbuf();
  std::ofstream *out ;

  if( isPrintToFile )
  {
    out = new std::ofstream ;
    out->open( outFile.c_str() );
    std::cout.rdbuf(out->rdbuf());
  }

  // print xy frame: rows(x): w -> e, cols(y): n -> s
  if( isScientific )
    std::cout.setf( std::ios::scientific, std::ios::floatfield);
  else
    std::cout.setf( std::ios::fixed, std::ios::floatfield);


// XXXXXXXXXXXXXXXXXX
//
//   double threshold=0.5;
//   for( size_t ltx=0 ; ltx < latSz ; ++ltx)
//   {
//     for( size_t lngx=0 ; lngx < lonSz ; ++lngx)
//     {
//       if( isFillingValue( cellValue->[ltx][lngx] ) )
//         continue;

//       if( fabs(cellValue->[ltx][lngx] - threshold) < .1 )
//         cellValue->[ltx][lngx] = 0.5; // shore
//       else if( cellValue->[ltx][lngx] < (threshold -.1) )
//         cellValue->[ltx][lngx] = 0.; // water
//       else
//         cellValue->[ltx][lngx] = 1.; // land
//     }
//   }
//
// XXXXXXXXXXXXXXXXXX

  // print entire border values for longitudes
  std::cout << std::setw(formatWidth)<<std::setprecision(formatPrecis);

  if( isGMT_MultiSegment )
  {
    // determine min/max values and write into a header line
    double gVMin=MAXDOUBLE;
    double gVMax=-MINDOUBLE;
    for( size_t i=0 ; i < cellValue->size() ; ++i)
    {
        if( isStripMissingValue && isFillingValue( cellValue->[i] ) )
          continue;

         if( cellValue->[i] < gVMin )
           gVMin = cellValue->[i];
         if( cellValue->[i] > gVMax )
           gVMax = cellValue->[i];
    }
    std::cout << std::setw(0) << "Domain: "
              << std::setw(0) << gVMin << ' '
              << std::setw(0) << gVMax << '\n' ;

    // helper vector. size depends on output format.
    // GMT_Segment: the 4 corners of a grid cell
    std::vector<double> dd_lon;
    std::vector<double> dd_lat;

//
//    dd_lon.push_back(0.); // in fact: +0.5 - 0.5, where
//                          //+0.5 is the centre of the grid cell
//    dd_lon.push_back(1.0);// +0.5 +0.5
//    dd_lon.push_back(1.0);
//    dd_lon.push_back(0.);
//    dd_lat.push_back(0.);
//    dd_lat.push_back(0.);
//    dd_lat.push_back(1.0);
//    dd_lat.push_back(1.0);

    dd_lon.push_back(-0.5*lonStep);
    dd_lon.push_back(0.5*lonStep);
    dd_lon.push_back(0.5*lonStep);
    dd_lon.push_back(-0.5*lonStep);
    dd_lat.push_back(-0.5*latStep);
    dd_lat.push_back(-0.5*latStep);
    dd_lat.push_back(0.5*latStep);
    dd_lat.push_back(0.5*latStep);

    // declare two fields for the coord of a box
    // for efficiency there is in and out for lat
    double bc_lat_in[4], bc_lat_out[4], bc_lon[4];

    for( size_t ltx=0 ; ltx < latSz ; ++ltx)
    {
      // set model box coord.
      for(size_t i=0 ; i < 4 ; ++i)
        bc_lat_in[i]= cellLatVertices[ltx] + dd_lat[i] ;

      for( size_t lngx=0 ; lngx < lonSz ; ++lngx)
      {
        size_t index= cellArea.index(ltx, lngx) ;

        if( isStripMissingValue && isFillingValue( cellValue->[ index ]))
          continue;

        std::cout << std::setw(0) << "> -Z"
                  << std::setw(0) << cellValue->[ index ] <<'\n' ;

        // set model box coord.
        for(int i=0 ; i < 4 ; ++i)
          bc_lon[i]=cellLonVertices[lngx] + dd_lon[i] ;

        // model coordinates are of a rotated grid, so if
        if( isBackRotation )
        {
          for(int i=0 ; i < 4 ; ++i)
            coordBackRotation(bc_lat_in[i], bc_lon[i],
                              &bc_lat_out[i], &bc_lon[i]);
        }

        for(int i=0 ; i < 4 ; ++i)
        {
            std::cout << std::setw(formatWidth)
                      << bc_lon[i] << ' ' ;
            std::cout << std::setw(formatWidth)
                      << bc_lat_out[i] << '\n' ;
        }
      }
    }
  }
  else
  {
    for( size_t ltx=0 ; ltx < latSz ; ++ltx)
    {
      for( size_t lngx=0 ; lngx < lonSz ; ++lngx)
      {
        size_t index= cellArea.index(ltx, lngx) ;

        if( isStripMissingValue && isFillingValue( cellValue->[ index ] ))
          continue;

        double myLon=cellLonVertices[lngx];
        double myLat=cellLatVertices[ltx];

        // model coordinates are of a rotated grid, so if
        if( isBackRotation )
           coordBackRotation(myLat, myLon, &myLat, &myLon);

        std::cout << std::setw(formatWidth) << myLon<< ' ' ;
        std::cout << std::setw(formatWidth) << myLat<< ' ' ;
        std::cout << std::setw(formatWidth)
                  << cellValue->[ index ] <<'\n' ;
      }
    }
  }

  std::cout << std::flush ;

  if( isPrintToFile )
  {
    std::cout.rdbuf(cout_sbuf); // restore the original stream buffer
    delete out;
  }

  return ;
}
*/

template<typename T>
void
GeoMeta<T>::resetLayer(void)
{
  cellValue.frame(*data, 0, size_2D) ;

  for( size_t j=0 ; j < currLayer.size() ; ++j )
    currLayer[j] = 0 ;

  layerOffset = 0;
  return;
}

template<typename T>
void
GeoMeta<T>::setArea(bool isRegGrid)
{
  if( isRegGrid )
    calcRegularGridArea();
  else
    calcAreaFromVertices();

  return;
}

template<typename T>
void
GeoMeta<T>::setLayer(std::vector<std::string> &nm,
             std::vector<size_t> &pos, std::vector<size_t> &sz)
{
  for( size_t i=0 ; i < nm.size() ; ++i )
    setLayer(nm[i], sz[i], pos[i]);

  hasLayers=true;

  if( nm.size() == 1 )
    cellValue.resize( size_2D) ;
  else if( nm.size() == 2 )
    cellValue.resize( latSz, lonSz) ;

  return;
}

template<typename T>
void
GeoMeta<T>::setLayer(std::string s, size_t sz, size_t pos)
{
  // Initialise layer properties.
  if( s.substr(0,3) == "lon" )
  {
    if( lonSz == sz && lonPos == pos )
      return;

    if( isImmutableLayer && lonSz && (lonSz != sz) )
    {
      std::ostringstream ostr(std::ios::app);
      ostr << "GeoMeta<T>::setLayer():"
         << "changed layer's longitude dimension."
         <<   "previous: " << lonSz
         << "\n current: " << sz ;
      exceptionError( ostr.str() );
    }

    lonSz = sz ;
    lonPos = pos;
  }
  else if( s.substr(0,3) == "lat"
             || s.substr(0,3) == "loc"
                 || s.substr(0,3) == "sit" )
  {
    if( latSz == sz && latPos == pos )
      return;

    if( isImmutableLayer && latSz && (latSz != sz) )
    {
      std::ostringstream ostr(std::ios::app);
      ostr << "GeoMeta<T>::setLayer():"
         << "changed layer's latitude dimension."
         <<   "previous: " << latSz
         << "\n current: " << sz ;
      exceptionError( ostr.str() );
    }

    latSz = sz ;
    latPos = pos;
  }
  else if( s == "vertice" )
  {
    vrtcSz = sz ;
    vrtcPos = pos;
  }
/*
  else
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoMeta<T>::setLayer():"
       << "unknown layer dimension:" << s ;
    exceptionError( ostr.str() );
  }
*/

  if( latSz && lonSz )
    size_2D = latSz * lonSz;
  else if( latSz  )
    size_2D = latSz;
  else if( lonSz  )
    size_2D = lonSz;

  return;
}

template <typename T>
void
GeoMeta<T>::setNonLayerDim(std::vector<size_t> sz,
                          std::vector<size_t> pos)
{ layerLevelDim=sz;
  layerLevelPos=pos;

  // note: first layer dimension is defined, even if there is no one
  for( size_t i=1 ; i < sz.size() ; ++i )
    currLayer.push_back(0);

  layerOffset = 0;
  return;
}

template<typename T>
void
GeoMeta<T>::setVertices(std::string name, void *v,std::string vType,
  size_t vrtSz, size_t vrtPos, size_t lat_sz,
  size_t lat_pos, size_t lon_sz, size_t lon_pos )
{
  if( vType == "double" )
  {
    MtrxArr<double> &mv = *reinterpret_cast<MtrxArr<double> *>(v) ;
    setVertices(name, mv, vrtSz, vrtPos,
                  lat_sz, lat_pos, lon_sz, lon_pos );
  }
  return;
}

template <typename T>
template <typename X>
void
GeoMeta<T>::setVertices(std::string name, MtrxArr<X> &v,
  size_t vrtSz, size_t vrtPos, size_t lat_sz, size_t lat_pos,
  size_t lon_sz, size_t lon_pos )
{
  // vertices can be given in 1D or 2D format. If 2D, then
  // dimension and index position of lat come first.

  // different modes for the dimensional layout:
  // a) dimLat==dimLon==0: the dimensional layout has been set
  //    previously; just the lat|lon values are passed.
  // b) dimLat>0 && dimLon>0: set a 2D layout by passed parameters,
  //    then set values of the vertices for lon or lat
  // c) dimLat>0 && dimLon==0: not explicitly grid.

  // vertices dimension
  vrtcSz=vrtSz;
  vrtcPos=vrtPos;

  if( lat_sz )
  {
      setLayer("lat", lat_sz, lat_pos) ;
  }
  else

  if( lon_sz )
      setLayer("lon", lon_sz, lon_pos) ;

  if( name == "lat" || name == "y" )
    cellLatVertices = v ;
  else if( name == "lon" || name == "x" )
    cellLonVertices = v ;
  else
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "GeoData::setVertices()"
         << "\nprovide either lat|y or lon|x" ;
    exceptionError( ostr.str() );
  }

  hasShape = true;

  return;
}

/*
template<typename T>
void
GeoMeta<T>::setOutputFormat(std::string format)
{
  if( format[format.size()-1] == 'd' || format[format.size()-1] == 'D' )
    isScientific = false;

  int c=format.find('.') ;
  formatWidth = static_cast<int>(hdhC::string2Double( format.substr(0, c).c_str() ,1) );
  formatPrecis= static_cast<int>(hdhC::string2Double( format.substr(c+1,format.size()-c-1).c_str() ,1) );

  return;
}
*/
/*
template<typename T>
void
GeoMeta<T>::setPrintFilename(std::string file)
{
  outFile=file;
  isPrintToFile=true;
  enabledPrint=true;
  return ;
}
*/
/*
template<typename T>
void
GeoMeta<T>::setXtractDomain(std::string domain)
{
   // account for the possibility that this method is called
   // before setAlignment or any other

   double tmp;

   // the user may provide values in arbitrarily order
   lonGeoBeg = hdhC::string2Double(domain,1);
   lonGeoEnd = hdhC::string2Double(domain,2);
   latGeoBeg = hdhC::string2Double(domain,3);
   latGeoEnd = hdhC::string2Double(domain,4);

   if( latGeoBeg > latGeoEnd )
   {
     tmp=latGeoEnd;
     latGeoEnd=latGeoBeg;
     latGeoBeg=tmp;
   }

   if( lonGeoBeg > lonGeoEnd )
   {
     tmp=lonGeoEnd;
     lonGeoEnd=lonGeoBeg;
     lonGeoBeg=tmp;
   }

   isXtractDomain=true;

  return;
}
*/

template<typename T>
bool
GeoMeta<T>::testIdenticalMapping( const GeoMeta<T> &g)
{
// TODO
  if( latSz != g.latSz  )
    return false;
  if( lonSz != g.lonSz  )
    return false;

  return true ;
}

#endif
