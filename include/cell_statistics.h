#ifndef _CELL_STATISTICS_H
#define _CELL_STATISTICS_H

#include "iobj.h"

/*! \file cell_statistics.h
 \brief Apply simple statistics to an ensemble of grid-cells.
*/

//! Statistics of each grid-cell (derived from class Base).
/*! Not used for the quality control.*/

class CellStatistics : public IObj
{
public:
  CellStatistics();
  ~CellStatistics(){;}

  //! coresponding to virtual methods in IObj
  bool         entry(void){return false;}
  bool         init(void) ;
  void         linkObject(IObj *);

  void         setFilename(std::string f){file.setFile(f);}

  void initDefaults(void);

  hdhC::FileSplit file;
};

#endif
