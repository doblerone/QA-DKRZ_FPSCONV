#ifndef _IN_FILE_H
#define _IN_FILE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "base.h"

/*! \file in_file.h
 \brief Interface to netCDF meta-data and data: reading and writing.
*/

//! Access to netCDF files (derived from class Base).

/*! Purpose: open a netCDF file, get meta-data of variables (in struct Base::Variable),
    and store data in a GeoMeta object with the respective netCDF type. InFile may
    have embedded pointers to the objects of FD_interface, QA, and/or TimeControl.
    A pointer links the Annotation instance.*/

class InFile : public Base
{
  // Layout (attributes) of a variable
  //! meta data of variable representations of dimensions

public:

  //! Default constructor
  InFile() ;
  ~InFile(){nc.close();}

  //! coresponding to virtual methods in IObj
  bool   entry(void);
  bool   init(void) ;
  void   linkObject(IObj *);

  void   applyOptions(void);
  void   applyOptions(std::vector<std::string> &v);
  void   clearRecProperties(void);
  void   excludeVars(void);

  std::string
         getAbsoluteFilename(void);

//! Get values of an attribute
  std::string
         getAttValue(std::string attName, std::string varName="");

  //! Get and store records of netCDF

  // used externally; read for all
  void   getData(int rec);  // for all
  void   getData(int rec, std::string);  //for s

//! Get those variable names not depending on the unlimited dimension.
/*! If no unlimited variable is defined but time is provided, then
    try for all variables, not depending on time.*/
    std::vector<std::string>
         getLimitedVarName(void);

//! Get the number of records.
/*! If there is no unlimited variable, but time is defined, then
    try for all variables, depending on time. Still not found, then
    return 0 */
    size_t
         getNumOfRecords(void);

  //! Get the path to the netcdf file.
  std::string
         getPath(void){return path;}

  //! Get the units attribute of time
  /*! In effect, returns the units of the variable representation
      of the unlimited dimensions; empty string, if not found. */
  std::string
         getTimeUnit(void);

  //! Get the units attribute of time
  /*! Stores also the name of the var-repres. of the unlimited dim.*/
  std::string
         getTimeUnit(std::string &vName);

//! Get names of all variables depending on the unlimited dimension.
/*! If no unlimited variable is defined, but time is provided, then
    try for all variables, depending on time.*/
    std::vector<std::string>
         getUnlimitedVars(void);

//! Get the index of a variable
/*! Return -1 for a mismatch*/
  int    getVarIndex(std::string);

  //! Scan netcdf header
  /*! Variables and attributes of unlimited dimension are analysed
      and corresponding GeoMeta objects are generated..*/
  void   getVarSpecs(void);

  static void
         help(void);

  void   initDefault(void);
  void   initRecProperties(int e=-1);

//! Return true, if variable 'vName' depends on unlimited dimension.
/*! If no unlimited variable is defined, but time is provided, then
    try for all variables, not depending on time.*/
  bool isVarUnlimited(std::string);

  /*! Open a file and fill struct Variable.*/
  bool   openNc( bool isNew=true );

  void   pullMetaData(char mode='\0');

  //! Set current record.
  /*! Purpose: Adjust the current record of a for-loop in a method.*/
  void   setCurrRec(size_t rec) {currRec=rec;}

  void   setRecBeg(size_t rec) {currRec=ncRecBeg=rec;}
  void   setRecEnd(size_t rec) {ncRecEnd=rec;}

  //! Store the path to the netcdf file.
  void   setFilename(std::string f) {file.setFile(f);}
  void   setTablePath(std::string p) { ; }

  hdhC::FileSplit file;

  bool   enableEntry;
  bool   isInfNan;
  bool   isInit;
  bool   isPrintGMT;
  bool   isRecEndSet;
  bool   isRecSet; // true for individual rec properties
  bool   isRecSingle;
  bool   isUnlimited;
  size_t currRec;
  size_t externRecBeg;
  size_t externRecEnd;  // see option -R
  size_t ncRecBeg;
  size_t ncRecEnd;
  size_t recStride;
  size_t recOffset;
  std::string unlimitedName;

  std::map<std::string, int> vIx;
  std::vector<std::string> excludedVariable;

  std::vector<size_t> dataVarIndex;
  bool                isTime;
  size_t  varSz;  // note: without a trailing NC_GLOBAL
  std::vector<Variable> variable;  // the last item for global attribute
  NcAPI   nc;

private:
  int    getDimPos(std::string &dName);
  bool   getNcAttVal(std::string vName,
              std::string aName, std::vector<double> &back);
  void   getVariableAtt(Variable &);
  // mode: 0: variables && global, 1: variables, 2: global
  void   getVariableMD(std::vector<Variable>& variable, char mode='\0');
  void   getVariableMD(Variable &);
  void   makeVariable (std::string name, Variable&, int id=-1);
  void   setGeoCellShape(Variable &var);
  void   setGeoLayer(Variable &var);
  void   setGeoParam(Variable & );
  void   setVerticesFromGridBounds(Variable &,
            std::string &lat_bnd_name, std::string &lon_bnd_name,
            size_t lat_dim_sz, size_t lon_dim_sz);
  void   setVerticesFromGridCellCentres(Variable &,
            std::string &lat_bnd_name, std::string &lon_bnd_name,
            size_t lat_dim_sz, size_t lon_dim_sz);

  // netCDF files.
  std::string path;
  std::string strLonLatResol;
  std::string fixedFieldVars;
  std::string explicitVariable;

};

#endif
