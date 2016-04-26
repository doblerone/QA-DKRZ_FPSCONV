#ifndef _FREQ_DIST_INTERFACE_H
#define _FREQ_DIST_INTERFACE_H

#include <dirent.h>
#include <values.h>

#include "iobj.h"
#include "hdhC.h"
#include "date.h"
#include "freqDist.h"
#include "geo_meta.h"

/*! \file fd_interface.h
 \brief Interface to build frequency distributions of QA results.
*/

//! Calculate frequency distributions out of netCF records.
/*! The pointer of this object is embedded in a Base-derived object.
The data of a GeoMeta obj there is used to build the frequency distribution.
A freqDist may be saved as ascii file that starts with a header (including
the start and end times of the data block taken into account).
A saved freqDist may be restored and a
session be resumed. Automatic time-window slicing is an option.
A saved freqDist with filename extension '.build' may be converted to a final shape (also with merging) by the program hist.cpp.*/

class FD_interface : public IObj
{
  struct ExceptionStruct
  {
     std::ofstream *ofsError;
     std::ofstream *ofsWarning;

     std::string strError;
     std::string strWarning;
  } ;

public:
  //! Default constructor
  FD_interface();
  ~FD_interface(){;}

  //! coresponding to virtual methods in IObj

  //! Executed for the current record.
  /*! Add GeoMeta field to the freqDist. Time-window slicing is
      controlled by evaluating the date corresponding the current
      record. */
  bool         entry(void);

  /*! A previous session might be resumed.*/
  bool         init(void);
  void         linkObject(IObj *);

  void         setFilename(std::string f){file.setFile(f);}
  void         setTablePath(std::string p){ ; }

  //! Settings by options.
  void applyOptions(void);

  void exceptionError(std::string );
  void exceptionWarning(std::string );
  //! Finalisation. Write the file.
  void finally(int errCode=0);
  int  getSourceID(void){return identNum;}

//! Description of options.
  static void
       help(void);

  void initDefaults(void);

  //! Time-window slicing.
  /*! Start, resume, or finish of a frequency distribution file.
      Update header information.*/
  bool initTimeWindow(void);
  void print(std::string fromDate, std::string toDate);
  void print(void);
  //! Obsolete.
  void pushFreqDist(size_t seas);

  //! Rebuild a freqDist from a saved build-file.
  /*! Priorities:\n
      1) build-file is named in option 'r=name' and exists, then use it \n
      2) build-file is named and does not exist, \n
         a) but, a file  named "frequencDist.[gD_VarUnlim.name].prop" \n
            is in the path, then use it \n
         b) but, a file  named "frequencDist.prop" is \n
            in the path, then use it \n
      3) a build-file is named and does not exist nor does \n
         "frequency.prop", but properties \n
         are given by options, then use the options \n*/
  bool rebuild( std::string rbFilename);

  //! get access to the global exception and annotation handling
  void setNotes(Annotation *n) {notes = n; }

  //! Connect this with the object to be checked
  void   setInFilePointer(InFile *p)
              { pIn = p; }
  void setSourceID(int i){identNum=i;}
  void setSrcStr(std::string s)
           {srcStr.push_back(s); return;}

  int identNum;
  hdhC::FileSplit file;

  std::string vName;
  std::vector<std::string> srcStr;

  // vector for regions, except that with index==0
  std::vector<FreqDist<float> > fD;

  std::string regionFile;
  std::vector<std::string> regioStr;

  bool isAutoResizeDisabled;
  bool isCentric;
  bool isFirstInit;
  bool isInitValue;
  bool isPrintBars;
  bool isPrintPlain;
  bool isReadProperties;
  bool isSaveBuild;
  bool isSetByOption;
  bool isStdOut;
  bool isUseAreaWeight;
  bool isUseFrequencyWeight;

  double binWidth;
  double initValue;

  Date refDate;
  Date currDate;
  Date beginDate;
  Date endDate;
  double currTime;
  std::string beginDateStr, currDateStr, endDateStr;
  std::string timeWindowWidth;

  std::string rebuildFilename;

  GeoMetaBase *pGD;

  // messaging in case of exceptions.
  struct ExceptionStruct xcptn;
};

#endif
