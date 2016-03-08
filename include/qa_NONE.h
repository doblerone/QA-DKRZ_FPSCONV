#ifndef _QA_NONE_H
#define _QA_NONE_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"
#include "qa.h"
#include "qa_data.h"
#include "qa_time.h"
#include "qa_PT.h"

//! Quality Control Program Unit for project NONE.
/*! All the QA considerations are covered by this class.\n
The QA_NONE.cpp and qa_NONE.h files have to be linked to
QA.cpp and qa.h, respectively.\n
The netCDF data-file is linked by a pointer to the InFile class
instance. Results of the QA are written to a netCDF file
(to the directory where the main program was started) with filename
qa_<data-filename>.nc. Outlier test and such for replicated records
are performed. Annotations are supplied via the Annotation class
linked by a pointer.
*/

struct DRS_CV
{
  DRS_CV(QA*);

  void   applyOptions(std::vector<std::string>&);
  void   checkModelName(std::string &aN, std::string &aV,
            char model, std::string iN="", std::string iV="");

  //! Match filename components and global attributes of the file.
  void   checkFilename(std::string& fName, struct DRS_CV_Table&);
  void   checkFilenameEncoding(Split&, struct DRS_CV_Table& );

  //! Test optional global attribute
  void   checkDrivingExperiment(void);

  //! Is it NetCDF-4, is it compressed?
  void   checkNetCDF(void);
  void   checkPath(std::string&, struct DRS_CV_Table&);
  void   checkProductName(std::string& drs_product,
                   std::string prod_choice,
                   std::map<std::string, std::string>& gM);
 void    findFN_faults(Split&, Split&,
                   std::map<std::string, std::string>&,
                   std::string& text);
  void   findPath_faults(Split&, Split&,
                   std::map<std::string, std::string>&,
                   std::string& text);
  int    getPathBegIndex( Split& drs, Split& x_e,
            std::map<std::string, std::string>& gM );

  void run(void);

  //! Test the time-period of the input file.
  /*! If the end-date in the filename and the last time value
      match within the uncertainty of 0.75% of the time-step, then
      the file is assumed to be completely qa-processed.
      Syntax of date ranges as given in CORDEX  DRS Syntax.*/
  bool   testPeriod(Split&);
  bool   testPeriodAlignment(std::vector<std::string> &sd, Date** pDates)  ;
  void   testPeriodCut(std::vector<std::string> &sd) ;
  bool   testPeriodCut_CMOR_isGOD(std::vector<std::string> &sd, Date**);
  void   testPeriodCutRegular(std::vector<std::string> &sd,
              std::vector<std::string>& text);
  bool   testPeriodDatesFormat(std::vector<std::string> &sd) ;
  bool   testPeriodFormat(Split&, std::vector<std::string> &sd) ;

  struct hdhC::FileSplit GCM_ModelnameTable;
  struct hdhC::FileSplit RCM_ModelnameTable;

  bool enabledCompletenessCheck;

  Annotation* notes;
  QA*         pQA;
};


//! Struct containing dimensional properties to cross-check with table information.
struct DimensionMetaData
{
  // first item is set by 'time'
  std::string  cmor_name;
  std::string  outname;
  std::string  stndname;
  std::string  longname;
  std::string  type;
  std::string  units;
  bool         isUnitsDefined;
  std::string  index_axis;
  std::string  axis;
  std::string  coordsAtt;
  std::string  bounds;
  uint32_t     checksum;  //fletcher32
  size_t       size;
};

  //! QA related variable information.
class VariableMetaData
{
  public:
  VariableMetaData(QA*, Variable *var=0);
  ~VariableMetaData();

  // index of variable obj
  std::vector<size_t>  dimVarRep;

  std::string standardName;
  std::string longName;
  std::string units;
  std::string cellMethods;
  std::string cellMethodsOpt;
  std::string positive;

  bool        isUnitsDefined;

  std::string name;
  std::string stdTableFreq;
  std::string stdSubTable;
  std::string type;
  std::string dims;
  std::string time_units;
  std::string unlimitedDim;

  Annotation     *notes;
  Variable *var;
  QA             *pQA ;
  QA_Data         qaData;

  int  finally(int errCode=0);
  void forkAnnotation(Annotation *p);
  void setAnnotation(Annotation *p);
  void setParent(QA *p);

  //! Verify units % or 1 by data range
  void verifyPercent(void);
};

class QA_Exp
{
  public:

   //! Default constructor.
  QA_Exp();

  void   applyOptions(std::vector<std::string>&);

  //! Make VarMetaData objects.
  void   createVarMetaData(void);

  std::string
         getAttValue(size_t v_ix, size_t a_ix);

  std::string
         getFrequency(void);

  std::string
         getTableEntryID(std::string vName);

  void   init(std::vector<std::string>&);

  //! Initialisation of flushing gathered results to netCDF file.
  /*! Parameter indicates the number of variables. */
  void   initDataOutputBuffer(void);

  //! Set default values.
  void   initDefaults(void);

  //! Initialisiation of a resumed session.
  /*! Happens for non-atomic data sets and those that are yet incomplete. */
  void   initResumeSession(std::vector<std::string>& prevTargets);

  //! Check the path to the tables;
  void   inqTables(void){return;}

  void   run(void);

  void   setParent(QA*);

  bool   testPeriod(void); //{return false;}

  std::vector<VariableMetaData> varMeDa;

  Annotation* notes;
  NcAPI *nc;
  QA* pQA;

  MtrxArr<double> tmp_mv;

  // the same buf-size for all buffer is required for testing replicated records
  size_t bufferSize;

  // init for test about times
  bool isCaseInsensitiveVarName;
  bool isClearBits;
  bool isUseStrict;  // dummy

  std::vector<std::string> excludedAttribute;
  std::vector<std::string> overruleAllFlagsOption;

  std::vector<std::string> constValueOption;
  std::vector<std::string> fillValueOption;
  std::vector<std::string> outlierOpts;
  std::vector<std::string> replicationOpts;

  std::string frequency;
  std::string fVarname;

  std::string getVarnameFromFilename(void){return hdhC::empty;}
  std::string getVarnameFromFilename(std::string str){return hdhC::empty;}
  void        pushBackVarMeDa(Variable*);
};

#endif
