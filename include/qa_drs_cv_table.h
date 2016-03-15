#ifndef _QA_DRS_CV_TABLE
#define _QA_DRS_CV_TABLE

// Tables of DRS and CV rules and policies. Information of table
// PROJECT_DRS_VC.cvs can be parsed into the members of this struct.
// Proceedings have to be coded in DRS_CV which is called by QA_PROJECT.run()
class DRS_CV_Table
{
public:

   DRS_CV_Table(){;}
   void applyOptions(std::vector<std::string>&);
   void read(void);
   void setParent(QA*);
   void setPath(std::string& p){ tablePath=p;}

   std::map<std::string, std::string> cvMap;
   std::vector<std::string> fileEncoding;
   std::vector<std::string> fileEncodingName;
   std::vector<std::string> pathEncoding;
   std::vector<std::string> pathEncodingName;

   std::vector<std::string> varName;
   std::vector<std::vector<std::string> > attName;
   std::vector<std::vector<std::string> > attValue;

   std::vector<std::string> section;
   std::vector<std::vector<std::string> > line;

   struct hdhC::FileSplit table_DRS_CV;
   std::string tablePath;

   QA* pQA;
   Annotation* notes;

};

#endif
