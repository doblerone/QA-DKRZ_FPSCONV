#ifndef _QA_H
#define _QA_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"
#include "json/json-forwards.h"
#include "json/json.h"
#include "qa_data.h"
#include "qa_drs_cv_table.h"
#include "qa_time.h"
#include "qa_cnsty.h"

#if defined CORDEX
  #include "qa_CORDEX.h"
#elif defined CMIP5
  #include "qa_CMIP5.h"
#elif defined CMIP6
  #include "qa_CMIP6.h"
#endif


//! Quality Control Program Unit for CORDEX.
/*! All the QA considerations are covered by this class.\n
Features specific to an experiment are 'imported' by qa_EXPNAME.h,
where EXPNAME indicates the experiment, e.g. CMIP5.\n
The netCDF data-file is linked by a pointer to the InFile class
instance. Results of the QA are written to a netCDF file
(to the directory where the main program was started) with filename
qa_<data-filename>.nc. Outlier test and such for replicated records
are performed. Annotations are supplied via the Annotation class
linked by a pointer.
*/

//! Struct containing dimensional properties to cross-check with table information.
class QA : public IObj
{
  public:

  //! Default constructor.
  QA();
  ~QA();

  //! coresponding to virtual methods in IObj

  //! Check field properties of the variable.
  /*! Designed for multiple usage for sub-layers of a data block.
      Each multiple-set must be invoked by calling method 'clearStatistics'.*/
  bool   entry(void);

  //! Initialisation of the QA object.
  /*! Open the qa-result.nc file, when available or create
   it from scratch. Meta data checks are performed.
   Initialise time testing, time boundary testing, and cycles
   within a time step. At the end  entry() is called to test
   the data of fields.*/
  bool   init(void) ;
  void   linkObject(IObj *);

  // special: from InFile with path and period stripped.
  void   setFilename(hdhC::FileSplit&);
  void   setTablePath(std::string p){tablePath=p;}

  void   applyOptions(bool isPost=false);

  //! Comparison of dimensions between file and standard table
  /*! Cross-checks with the standard table are performed only once for
   each variable at first time encounter in the CORDEX Project ensuring
   conformance.*/

  bool   checkDataBody(std::string vName="");

  //!    true requires a check against the project meta data
  bool   checkConsistency(InFile &in, std::vector<std::string> &opts,
                          std::string& tPath);

  /*! Close records for time and data.*/
  void   closeEntry(void);

  //! Make VarMetaData objects.
  void   createVarMetaData(void);

  //! Check the path to the tables;
  void   defaultPrjTableName(void);

  //! The final operations.
  /*! An exit code is returned.*/
  int    finally(int errCode=0);

  //! The final qa data operations.
  /*! Called from finall(). An exit code is returned.*/
  int    finally_data(int errCode=0);

  std::string
         getAttValue(size_t v_ix, size_t a_ix);

  int    getExitState(int e=-1);

  //! Get path componenents.
  /*! mode: "total": filename with total path, "file": filename,
      "base": filename without extension, "ext": extension without '.',
      "path": the path component without trailing '/'.*/
  std::string
         getPath(std::string& f, std::string mode="total");

  //! Brief description of options
  static void
         help(void);

  void   initCheckStatus(void);

  //! Initialisation of flushing gathered results to netCDF file.
  /*! Parameter indicates the number of variables. */
  void   initDataOutputBuffer(void);

  //! Set default values.
  void   initDefaults(void);

  //! Global attributes of the qa-netCDF file.
  /*! Partly reflecting global attributes from the sources. */
  void   initGlobalAtts(InFile &);

  //! Initialisiation of a resumed session.
  /*! Happens for non-atomic data sets and those that are yet incomplete. */
  void   initResumeSession(void);

  bool   isProgress(void){ return ! qaTime.isNoProgress ; }

  //! Get coordinates of grid-cells where an error occurred
  /*! Does not work for tripolar coordinates */
//  bool   locate( GeoMetaT<float>*, double *lat, double *lon, const char* );

  //! Open a qa_result file for creation or appending data.
  /*! CopY time variable from input-nc file.
   Collect some properties of the in-netcdf-file in
   struct varMeDa. Also check properties against tables.
  */
  void   openQA_Nc(InFile&);

  //! Perform only post-processing
  bool   postProc(void);
  bool   postProc_outlierTest(void);

  void   resumeSession(void);

  //! Connect this with the object to be checked
  void   setInFilePointer(InFile *p) { pIn = p; }

  //! Unused.
  /*! Needed to be conform to a specific Base class functionality */
  void   setSrcStr(std::string s)
             {srcStr.push_back(s); return;}

  //! Store results in the internal buffer
  /*! The buffer is flushed to file every 'flushCountMax' time steps.*/
  void   store(std::vector<hdhC::FieldData> &fA);
  void   storeData(VariableMetaData&, hdhC::FieldData& );
  void   storeTime(void);

  //! Name of the netCDF file with results of the quality control
  std::string tablePath;
  hdhC::FileSplit qaFile;
  hdhC::FileSplit consistencyFile;

  std::string qaNcfileFlags;

  int exitState;
  bool is_exit;

  Annotation* notes;
  DRS_CV_Table drs_cv_table;
  NcAPI*      nc;
  QA_Exp      qaExp;
  QA_Time     qaTime;

  int thisId;

  size_t currQARec;
  size_t importedRecFromPrevQA; // initial num of recs in the write-to-nc-file
  MtrxArr<double> tmp_mv;

  // the same buf-size for all buffer is required for testing replicated records
  size_t bufferSize;

  // init for test about times
  bool enablePostProc;
  bool enableVersionInHistory;
  bool isClearBits;
  bool isFileComplete;
  bool isFirstFile;
  bool isNotFirstRecord;
  bool isOnlyCF;
  bool isPrintTimeBoundDates;
  bool isResumeSession;

  size_t nextRecords;

  bool isCheckCF;
  bool isCheckCV;
  bool isCheckCNSTY;
  bool isCheckData;
  bool isCheckDRS_F;
  bool isCheckDRS_P;
  bool isCheckTime;
  bool isCheckTimeValues;

  bool isRequiredTime;
  bool isRequiredVariable;
  bool isRequiredGlobal;

  std::vector<std::string> excludedAttribute;
  std::vector<std::string> overruleAllFlagsOption;

  std::vector<std::string> constValueOption;
  std::vector<std::string> fillValueOption;
  std::vector<std::string> outlierOpts;
  std::vector<std::string> replicationOpts;
  std::vector<std::string> requiredAttributesOption;

  int identNum;
  char        fileSequenceState;
  std::string prevVersionFile;
  std::vector<std::string> srcStr;
  std::string revision;

  static std::string tableSheet;
  static std::string tableSheetSub;

  std::string n_data;
  std::string n_time;
  std::string n_disabled;
  std::string n_pass;
  std::string n_fail;
  std::string fileStr;
  std::string notAvailable;
  std::string s_global;
  std::string s_mismatch;

  std::string n_axis;
  std::string n_cell_methods;
  std::string n_frequency;
  std::string n_long_name;
  std::string n_outputVarName;
  std::string n_positive;
  std::string n_standard_name;
  std::string n_units;

  std::string getCaptIntroDim(VariableMetaData &vMD,
                   struct DimensionMetaData &nc_entry,
                   struct DimensionMetaData &tbl_entry, std::string att="");
  void        appendToHistory();
  bool        isExit(void);
  std::string getSubjectsIntroDim(VariableMetaData &vMD,
                   struct DimensionMetaData &nc_entry,
                   struct DimensionMetaData &tbl_entry, bool isColon=true);
  bool        not_equal(double x1, double x2, double epsilon);
  void        pushBackVarMeDa(Variable*);
  void        setExitState(int);
  void        setCheckMode(std::string);
  void        setProcessing(void);
};

std::string QA::tableSheet=hdhC::empty;
std::string QA::tableSheetSub=hdhC::empty;

#endif

