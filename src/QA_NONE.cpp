//#include "qa.h"
DRS_CV::DRS_CV(QA* p)
{
  pQA = p;
  notes = pQA->notes;

  enabledCompletenessCheck=true;

  applyOptions(pQA->optStr);
}

void
DRS_CV::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "dIP"
          || split[0] == "dataInProduction"
              || split[0] == "data_in_production" )
     {
        // effects completeness test in testPeriod()
        enabledCompletenessCheck=false;
        continue;
     }

   }

   return;
}

void
DRS_CV::checkDrivingExperiment(void)
{
  InFile& in = *(pQA->pIn);
  std::string str;

  // special: optional driving_experiment could contain
  // driving_model_id, driving_experiment_name, and
  // driving_model_ensemble_member
  str = in.getAttValue("driving_experiment") ;

  if( str.size() == 0 )
    return;  // optional attribute not available

  Split svs;
  svs.setSeparator(',');
  svs.enableEmptyItems();
  svs = str;

  std::vector<std::string> vs;
  for( size_t i=0 ; i < svs.size() ; ++i )
  {
    vs.push_back( svs[i] );
    vs[i] = hdhC::stripSides( vs[i] ) ;
  }

  if( vs.size() != 3 )
  {
    std::string key("2_9");
    if( notes->inq( key, pQA->fileStr ) )
    {
      std::string capt("global " + hdhC::tf_att("driving_experiment") );
      capt += "with wrong number of items, found" ;
      capt += hdhC::tf_val(str) ;

      (void) notes->operate( capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }

    return;
  }

  // index of the pseudo-variable 'NC_GLOBAL'
  if( in.variable.size() == in.varSz )
     return; // no global attributes; checked and notified elsewhere

  Variable &glob = in.variable[in.varSz];

  // att-names corresponding to the three items in drinving_experiment
  std::vector<std::string> rvs;
  rvs.push_back("driving_model_id");
  rvs.push_back("driving_experiment_name");
  rvs.push_back("driving_model_ensemble_member");

  std::string value;

  for( size_t i=0 ; i < rvs.size() ; ++i )
  {
    value = glob.getAttValue( rvs[i] ) ;

    // a missing required att is checked elsewhere
    if( value.size() == 0 )
      continue;

    // allow anything for r0i0p0
    if( value != vs[i] && value != "r0i0p0" && vs[i] != "r0i0p0" )
    {
      std::string key("2_10");
      if( notes->inq( key, pQA->fileStr ) )
      {
        std::string capt("global ");
        capt += hdhC::tf_att("driving_experiment(" + hdhC::itoa(i) + ")" );
        capt += "is in conflict with " + hdhC::tf_att(rvs[i]) ;

        std::string text(rvs[i]) ;
        text += "=" ;
        text += value ;
        text += "\ndriving_experiment=" ;
        if( i==0 )
        {
          text += vs[i] ;
          text += ", ..., ..." ;
        }
        else if( i==1 )
        {
          text += "...," ;
          text += vs[i] ;
          text += ", ..." ;
        }
        else if( i==2 )
        {
          text += ", ..., ..." ;
          text += vs[i] ;
        }

        (void) notes->operate( capt, text) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  return;
}

void
DRS_CV::checkFilename(std::string& fName, struct DRS_CV_Table& drs_cv_table)
{
  Split x_filename;
  x_filename.setSeparator("_");
  x_filename.enableEmptyItems();
  x_filename = fName ;

  // note that this test is not part of the QA_Time class, because
  // coding depends on projects
  std::string frq = pQA->qaExp.getFrequency();

  if( testPeriod(x_filename) && frq.size() && frq != "fx" )
  {
    std::string key("1_6a");
    if( notes->inq( key, pQA->qaTime.name) )
    {
      std::string capt("filename requires a period");

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  checkFilenameEncoding(x_filename, drs_cv_table);

  return ;
}

void
DRS_CV::checkFilenameEncoding(Split& x_filename, struct DRS_CV_Table& drs_cv_table )
{
  // fileEncodingName: name of the encoding type
  // fileEncoding:     sequence of DRS path components
  // encodingMap:      name in encoding vs. name of global attribute (or *)

  if( x_filename.size() == 0 )
    return;

  // components of the current path, these are given in the reverse order.
  Split& drs = x_filename;

  Variable& globalVar = pQA->pIn->variable[ pQA->pIn->varSz ] ;
  std::string n_ast="*";
  size_t enc_sz = drs_cv_table.fileEncoding.size() ;

  Split x_enc[enc_sz];
  std::vector<size_t> countCI(enc_sz, 0);
  std::map<std::string, std::string> globMap[enc_sz] ;
  std::vector<std::vector<size_t> > specialFaultIx ;

  for( size_t ds=0 ; ds < enc_sz ; ++ds)
  {
    Split& x_e = x_enc[ds] ;
    std::map<std::string, std::string>& gM = globMap[ds] ;

    x_e.setSeparator("_");

    std::map<std::string, std::string>& cvMap = drs_cv_table.cvMap ;

    // could have a trailing ".nc" item; if yes, then remove this beforehand
    if( drs_cv_table.fileEncoding[ds].rfind(".nc") < std::string::npos )
    {
      std::string& t = drs_cv_table.fileEncoding[ds];
      // is it trailing?
      if( t.substr(t.size()-3) == ".nc" )
        x_e = t.substr(0, t.size()-3) ;
    }
    else
      x_e = drs_cv_table.fileEncoding[ds] ;

    for(size_t x=0 ; x < x_e.size() ; ++x )
    {
      if( cvMap.count(x_e[x]) == 0 )
      {
        std::string key("7_3");
        std::string capt("Fault in table ");
        capt += pQA->drs_cv_table.table_DRS_CV.getFile();
        capt += ": encoding";
        capt += hdhC::tf_assign("item", x_e[x]);
        capt += " not found in CV";

        if( notes->inq( key, "DRS") )
        {
          (void) notes->operate(capt) ;
          notes->setCheckMetaStr(pQA->fail);
        }
      }

      if( cvMap[x_e[x]] == n_ast )
      {
        if( x_e[x] == "VariableName" )
          gM[x_e[x]] = pQA->qaExp.fVarname ;

        else if( x_e[x] == "StartTime-EndTime" )
          gM[x_e[x]] = n_ast ;
      }
      else
        gM[x_e[x]] = globalVar.getAttValue(cvMap[x_e[x]]) ;
    }

    //count coincidences between filename components and the DRS
    std::string t;
    size_t x_eSz = x_e.size();

    // append * in case of no period
    drs.append("*");

    for( size_t jx=0 ; jx < x_eSz ; ++jx)
    {
      t = gM[ x_e[jx] ];

      if( jx == drs.size() )
        break;

      if( drs[jx] == t || t == n_ast )
        ++countCI[ds];

      // special
      else if( drs[jx] == "r0i0p0" && pQA->qaExp.getFrequency() == "fx" )
      {
        drs.replace(jx, globalVar.getAttValue("driving_model_ensemble_member") );
        ++countCI[ds];
      }
    }

    if( countCI[ds] == x_e.size() )
      return;  // check passed; take into account one optional item
  }

  size_t m=0 ; // because there is only a single DRS-FS item

  // find details of the faults
  Split& x_e = x_enc[m] ;
  std::map<std::string, std::string>& gM = globMap[m] ;

  std::vector<std::string> text;
  std::vector<std::string> keys;

  std::string txt;
  findFN_faults(drs, x_e, gM, txt) ;
  if( txt.size() )
  {
    keys.push_back("1_2");
    text.push_back(txt);
  }

  if( text.size() )
  {
    std::string capt("DRS CV filename:");

    for(size_t i=0 ; i < text.size() ; ++i )
    {
      if( notes->inq( keys[i], "DRS") )
      {
        (void) notes->operate(capt+text[i]) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  return;
}

void
DRS_CV::checkModelName(std::string &aName, std::string &aValue,
   char des, std::string instName, std::string instValue )
{
   hdhC::FileSplit* tbl;

   if( des == 'G' )
     tbl = &GCM_ModelnameTable ;
   else
     tbl = &RCM_ModelnameTable ;

   ReadLine ifs(tbl->getFile(), false);  // no test of existence

   if( ! ifs.isOpen() )
   {
      std::string key("7_4") ;

      if( des == 'G' )
        key += "a" ;
      else
        key += "b" ;

      if( notes->inq(key) )
      {
         std::string capt("could not open " + tbl->getBasename()) ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( pQA->fail );
           pQA->setExit( notes->getExitValue() ) ;
         }
      }

      return;
   }

   std::string line;
   Split x_line;

   // parse table; trailing ':' indicates variable or 'global'
   ifs.skipWhiteLines();
   ifs.skipComment();

   bool isModel=false;
   bool isInst=false;
   bool isModelInst=false;

   bool isRCM = (des == 'R') ? true : false ;

   while( ! ifs.getLine(line) )
   {
     x_line = line ;

     if( aValue == x_line[0] )
       isModel=true;

     if( isRCM )
     {
       if( x_line.size() > 1 && instValue == x_line[1] )
       {
         isInst=true;
         if( isModel )
         {
            isModelInst=true;
            break;
         }
       }
     }
     else if( isModel )
        break;
   }

   ifs.close();

   if( !isModel )
   {
     std::string key;
     if( des == 'G' )
       key = "1_3a" ;
     else
       key = "1_3b" ;

     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt(pQA->s_global);
       capt += hdhC::blank;
       capt += hdhC::tf_att(hdhC::empty, aName, aValue);
       capt += "is not registered in table " ;
       capt += tbl->getBasename();

       if( notes->operate(capt) )
       {
         notes->setCheckMetaStr( pQA->fail );
         pQA->setExit( notes->getExitValue() ) ;
       }
     }
   }

   if( isRCM && !isInst )
   {
     std::string key("1_3c");

     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt(pQA->s_global);
       capt += hdhC::blank;
       capt += hdhC::tf_att(hdhC::empty, instName, instValue);
       capt += "is not registered in table " ;
       capt += tbl->getBasename();

       if( notes->operate(capt) )
       {
         notes->setCheckMetaStr( pQA->fail );
         pQA->setExit( notes->getExitValue() ) ;
       }
     }
   }

   if( isRCM && isModel && isInst && !isModelInst )
   {
     std::string key("1_3d");

     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("combination of ");
       capt += pQA->s_global ;
       capt += hdhC::blank;
       capt += hdhC::tf_att(hdhC::empty, aName, aValue);
       capt += "and " + hdhC::tf_assign(instName, instValue);
       capt += " is unregistered in table ";
       capt += tbl->getBasename();

       if( notes->operate(capt) )
       {
         notes->setCheckMetaStr( pQA->fail );
         pQA->setExit( notes->getExitValue() ) ;
       }
     }
   }

   return;
}

void
DRS_CV::checkNetCDF(void)
{
  // NC_FORMAT_CLASSIC (1)
  // NC_FORMAT_64BIT   (2)
  // NC_FORMAT_NETCDF4 (3)
  // NC_FORMAT_NETCDF4_CLASSIC  (4)

  NcAPI& nc = pQA->pIn->nc;

  int fm = nc.inqNetcdfFormat();
  std::string s;

  if( fm < 3 )
    s = "3";
  else if( fm == 3 )
  {
    s = "4, not classic, ";

    if( ! nc.inqDeflate() )
      s += "not ";

    s+= "deflated (compressed)";
  }

  if( s.size() )
  {
    std::string key("9_4");
    if( notes->inq( key ) )
    {
      std::string capt("NetCDF4 classic deflated (compressed) required") ;
      std::string text("this is NetCDF");
      text += s;

      (void) notes->operate( capt, text ) ;
      notes->setCheckMetaStr( pQA->fail);
    }
  }

  return;
}

void
DRS_CV::checkProductName(std::string& drs_product,
  std::string prod_choice,
  std::map<std::string, std::string>& gM)
{
  Split x_cp(prod_choice, "|");

  for( size_t i=0 ; i < x_cp.size() ; ++i )
  {
    if( drs_product == hdhC::stripSides(x_cp[i]) )
    {
      // adjustment for a test
      gM["product"] = drs_product ;
      return;
    }
  }

  std::string key("1_3");

  if( notes->inq( key, "DRS") )
  {
    std::string capt("DRS CV fault for path component <product>, found") ;
    capt += hdhC::tf_val(drs_product) ;
    capt += ", expected one of";
    capt += hdhC::tf_val(prod_choice);

    (void) notes->operate(capt) ;
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
DRS_CV::checkPath(std::string& path, struct DRS_CV_Table& drs_cv_table)
{
  // pathEncodingName: name of the encoding type
  // pathEncoding:     sequence of DRS path components
  // encodingMap:      name in encoding vs. name of global attribute (or *)

  size_t enc_sz = drs_cv_table.pathEncoding.size() ;

  if( path.size() == 0 || enc_sz == 0 )
    return;

  // components of the current path, these are given in the reverse order.
//  path= "/hdh/data/CMIP5/CMIP5/output/MPI-M/MPI-ESM-LR/hysterical/day/artmos/r1i1p1/tas/";
//  path="/hdh/data/cordex/output/EUR-44/SMHI/ECMWF-ERAINT/evaluation/r1i1p1/SMHI-RCA4/v1/mon/pr";
//  path="/hdh/data/cordex/input/EUR-44/ECMWF-ERAINT/r1i1p1/evaluation/SMHI-RCA4/v1/mon/pr";

  Split drs(path, "/");

  Variable& globalVar = pQA->pIn->variable[ pQA->pIn->varSz ] ;
  std::string n_ast="*";

  Split x_enc[enc_sz];
  std::vector<size_t> countCI(enc_sz, 0);
  std::map<std::string, std::string> globMap[enc_sz] ;

  for( size_t ds=0 ; ds < enc_sz ; ++ds)
  {
    Split& x_e = x_enc[ds] ;
    std::map<std::string, std::string>& gM = globMap[ds] ;

    x_e.setSeparator("/");

    std::map<std::string, std::string>& cvMap = drs_cv_table.cvMap ;

    x_e = drs_cv_table.pathEncoding[ds] ;

    for(size_t x=0 ; x < x_e.size() ; ++x )
    {
      if( cvMap.count(x_e[x]) == 0 )
      {
        std::string key("7_3");
        std::string capt("Fault in table ");
        capt += pQA->drs_cv_table.table_DRS_CV.getFile() ;
        capt += ": encoding " ;
        capt += hdhC::tf_assign("item", x_e[x]) ;
        capt += " not found in CV";

        if( notes->inq( key, "DRS") )
        {
          (void) notes->operate(capt) ;
          notes->setCheckMetaStr(pQA->fail);
        }
      }

      if( cvMap[x_e[x]] == n_ast )
      {
        if( x_e[x] == "VariableName" )
          gM[x_e[x]] = pQA->qaExp.fVarname ;
      }
      else
        gM[x_e[x]] = globalVar.getAttValue(cvMap[x_e[x]]) ;
    }


    // special: at least for the HH ESGF node, which has an additional trailing
    // item for a versioning, e.g. 'v20121231'. This is ignored.
    size_t last = drs.size()-1 ;
    if( drs[last][0] == 'v' && hdhC::isDigit(drs[last].substr(1)) )
        drs.erase(last);

    //special: customisation
    for( size_t i=0 ; i < drs.size() ; ++i )
    {
      if( drs[i] == "r0i0p0" && pQA->qaExp.getFrequency() == "fx" )
        // never thought the CORDEX community about this; at least in the beginning
        drs.replace(i, globalVar.getAttValue("driving_model_ensemble_member") );
    }

    std::string t;
    int ix;
    if( (ix = getPathBegIndex( drs, x_e, gM )) == -1 )
      continue;

    size_t drsBeg = ix;
    size_t i;

    for( size_t j=0 ; j < x_e.size() ; ++j)
    {
      t = gM[ x_e[j] ];

      if( (i = drsBeg+j) < drs.size() )
      {
        //count coincidences between path components and the DRS
        if( drs[i] == t || t == n_ast )
          ++countCI[ds];
        // drs may have a different value, e.g. output1
        else if( x_e[j] == "product" )
        {
          checkProductName(drs[i], cvMap[x_e[j]+"_constr"], gM);
          ++countCI[ds];
        }

        // another type of check; not counting in countCI
        if( x_e[j] == "RCMModelName" )
        {
          std::string inst("Institution");
          checkModelName(x_e[j], t, 'R', inst, gM[inst]) ;
        }
        else if( x_e[j] == "GCMModelName" )
          checkModelName(x_e[j], t, 'G') ;
      }
    }

    if( countCI[ds] == x_e.size() )
      return;  // check passed
  }

  // find the encoding with maximum number of coincidences
  size_t m=0;

/* // necessary if more than a single DRS structure is available
  size_t mx=countCI[0];
  for( size_t ds=1 ; ds < countCI.size() ; ++ds )
  {
    if( countCI[ds] > mx )
    {
      mx = countCI[ds] ;
      m=ds;
    }
  }
*/


  Split& x_e = x_enc[m] ;
  std::map<std::string, std::string>& gM = globMap[m] ;

  std::vector<std::string> text;
  std::vector<std::string> keys;

  std::string txt;
  findPath_faults(drs, x_e, gM, txt) ;
  if( txt.size() )
  {
    keys.push_back("1_1");
    text.push_back(txt);
  }

  if( text.size() )
  {
    std::string capt("DRS CV path:");

    for( size_t i=0 ; i < text.size() ; ++i )
    {
      if( notes->inq( keys[i], pQA->fileStr) )
      {
        (void) notes->operate(capt+text[i]) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  return;
}

void
DRS_CV::findFN_faults(Split& drs, Split& x_e,
                   std::map<std::string,std::string>& gM,
                   std::string& text)
{
  std::string t;
  std::string n_ast="*";
  int x_eSz = x_e.size();
  int drsSz = drs.size() ;

  for( int j=0 ; j < x_eSz ; ++j)
  {
    t = gM[ x_e[j] ];

    if( drsSz == x_eSz )
    {
      text = " check failed, suspicion of a missing item in the filename, found" ;
      text += hdhC::tf_val(drs.getStr()) ;

      return;
    }

    else if( !(drs[j] == t || t == n_ast) )
    {
      text = " check failed, expected " ;
      text += hdhC::tf_assign(x_e[j],t) ;
      text += " found" ;
      text += hdhC::tf_val(drs[j]) ;

      return;
    }
  }

  return ;
}

void
DRS_CV::findPath_faults(Split& drs, Split& x_e,
                   std::map<std::string,std::string>& gM,
                   std::string& text)
{
  std::string t;
  std::string n_ast="*";
  int x_eSz = x_e.size();
  int drsSz = drs.size() ;
  int drsBeg = drsSz - x_eSz ;

  if( drsBeg < 0)
  {
    text = " check failed" ;
    return; // fewer path items than expected
  }

  for( int j=0 ; j < x_eSz ; ++j)
  {
    t = gM[ x_e[j] ];

    int i = drsBeg+j ;

    if( i == -1 )
    {
      text = " suspicion of a missing item in the path, found" ;
      text += hdhC::tf_val(drs.getStr()) ;

      break;
    }

    if( !( drs[i] == t || t == n_ast) )
    {
      if(x_e[j] == "activity" )
      {
        std::string s( hdhC::Upper()(drs[i]) ) ;
        if( s == t  && !notes->inq( "1_3a", pQA->fileStr, "INQ_ONLY") )
          continue;
      }

      text = " global  " ;
      text += hdhC::tf_att(hdhC::empty,x_e[j],t) ;
      text += " vs." ;
      text += hdhC::tf_val(drs[drsBeg+j]) ;

      break;
    }
  }

  return ;
}

int
DRS_CV::getPathBegIndex(
    Split& drs, Split& x_e,
    std::map<std::string, std::string>& gM )
{
  int ix=-1;
  bool isActivity; // applied case-insensivity

  for( size_t i=0 ; i < drs.size() ; ++i)
  {
    std::string s(drs[i]);

    for( size_t j=0 ; j < x_e.size() ; ++j)
    {
      if( x_e[j] == "activity" )
      {
        isActivity=true;
        s = hdhC::Upper()(s);
      }
      else
        isActivity=false;

      if( s == gM[ x_e[j] ]  )
      {
        ix = static_cast<int>(i);

        // the match could be a leading false one, required is the last one
        for( ++i ; i < drs.size() ; ++i)
        {
          s = drs[i];

          if(isActivity)
            s = hdhC::Upper()(s);

          if( s == gM[ x_e[j] ]  )
            ix = static_cast<int>(i);
        }

        return ix;
      }
    }
  }

  return ix;
}

void
DRS_CV::run(void)
{

  return;
}

bool
DRS_CV::testPeriod(Split& x_f)
{
  // return true, if a file is supposed to be not complete.
  // return false, a) if there is no period in the filename
  //               b) if an error was found
  //               c) times of period and file match

  // The time value given in the first/last record is assumed to be
  // in the range of the period of the file, if there is any.

  // If the first/last date in the filename period and the
  // first/last time value match within the uncertainty of the
  // time-step, then the file is complete.
  // If the end of the period exceeds the time data figure,
  // then the nc-file is considered to be not completely processed.

  // Does the filename has a trailing date range?
  // Strip off the extension.
  std::vector<std::string> sd;
  sd.push_back( "" );
  sd.push_back( "" );

  // is it formatted as expected?
  if( testPeriodFormat(x_f, sd) )
    return true;

  // now that we have found two candidates for a date
  // compose ISO-8601 strings
  std::vector<Date> period;
  pQA->qaTime.getDRSformattedDateRange(period, sd);

  // necessary for validity (not sufficient)
  if( period[0] > period[1] )
  {
     std::string key("1_6c");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("invalid period range in the filename, found");
       capt += hdhC::tf_val(sd[0] + "-" + sd[1]);

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr( pQA->fail );
     }

     return false;
  }

  Date* pDates[6];
  pDates[0] = &period[0];  // StartTime in the filename
  pDates[1] = &period[1];  // EndTime in the filename

  // index 2: date of first time value
  // index 3: date of last  time value
  // index 4: date of first time-bound value, if available; else 0
  // index 5: date of last  time-bound value, if available; else 0

  for( size_t i=2 ; i < 6 ; ++i )
    pDates[i] = 0 ;

  if( pQA->qaTime.isTimeBounds)
  {
    pDates[4] = new Date(pQA->qaTime.refDate);
    if( pQA->qaTime.firstTimeBoundsValue[0] != 0 )
      pDates[4]->addTime(pQA->qaTime.firstTimeBoundsValue[0]);

    pDates[5] = new Date(pQA->qaTime.refDate);
    if( pQA->qaTime.lastTimeBoundsValue[1] != 0 )
      pDates[5]->addTime(pQA->qaTime.lastTimeBoundsValue[1]);

    double db_centre=(pQA->qaTime.firstTimeBoundsValue[0]
                        + pQA->qaTime.firstTimeBoundsValue[1])/2. ;
    if( ! hdhC::compare(db_centre, "=", pQA->qaTime.firstTimeValue) )
    {
      std::string key("5_7");
      if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
      {
        std::string capt("Range of variable time_bnds is not centred around time values.");

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
    }
  }
  else
  {
    if( pQA->qaTime.time_ix > -1 &&
        ! pQA->pIn->variable[pQA->qaTime.time_ix].isInstant )
    {
      std::string key("5_8");
      if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
      {
        std::string capt("Variable time_bnds is missing");

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  pDates[2] = new Date(pQA->qaTime.refDate);
  if( pQA->qaTime.firstTimeValue != 0. )
    pDates[2]->addTime(pQA->qaTime.firstTimeValue);

  pDates[3] = new Date(pQA->qaTime.refDate);
  if( pQA->qaTime.lastTimeValue != 0. )
    pDates[3]->addTime(pQA->qaTime.lastTimeValue);

  // the annotations
  if( !testPeriodAlignment(sd, pDates) )
  {
    if( testPeriodDatesFormat(sd) ) // format of period dates.
    {
      // period requires a cut specific to the various frequencies.
      std::vector<std::string> text ;
      testPeriodCutRegular(sd, text) ;

      if( text.size() )
      {
        std::string key("1_6e");
        std::string capt("period in the filename") ;

        for( size_t i=0 ; i < text.size() ; ++i )
        {
          if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
          {
            (void) notes->operate(capt + text[i]) ;

            notes->setCheckMetaStr( pQA->fail );
          }
        }
      }
    }
  }

  // note that indices 0 and 1 belong to a vector
  for(size_t i=2 ; i < 6 ; ++i )
    if( pDates[i] )
      delete pDates[i];

  // complete
  return false;
}

bool
DRS_CV::testPeriodAlignment(std::vector<std::string> &sd, Date** pDates)
{
  // regular test: filename vs. time_bounds
  if( *pDates[0] == *pDates[2] && *pDates[1] == *pDates[3] )
    return true;
  else
    if( testPeriodCut_CMOR_isGOD(sd, pDates) )
      return true;

  // alignment of time bounds and period in the filename
  bool is[] = { true, true, true, true };
  double uncertainty=0.1 ;
  if( pQA->qaExp.getFrequency() != "day" )
    uncertainty = 1.; // because of variable len of months

  // time value: left-side
  Date myDate( *pDates[2] );
  myDate.addTime(-pQA->qaTime.refTimeStep/2.);
  double dDiff = fabs(myDate - *pDates[0]) ;
  is[0] = dDiff < uncertainty ;

  // time value: right-side
  myDate = *pDates[3] ;
  myDate.addTime(pQA->qaTime.refTimeStep/2.);
  dDiff = fabs(myDate - *pDates[1]) ;
  is[1] = dDiff < uncertainty ;

  if(pQA->qaTime.isTimeBounds)
  {
    // time_bounds: left-side
    Date myDate = *pDates[4] ;
//    myDate.addTime(-pQA->qaTime.refTimeStep/2.);
//    dDiff = fabs(myDate - *pDates[0]) ;
    is[2] = myDate == *pDates[4] ;

    // time_bounds: right-side
    myDate = *pDates[5] ;
//    myDate.addTime(pQA->qaTime.refTimeStep/2.);
//    dDiff = fabs(myDate - *pDates[1]) ;
    is[3] = myDate == *pDates[5] ;
  }

  for(size_t i=0 ; i < 2 ; ++i)
  {
    // i == 0: left; 1: right

    // skip for the mode of checking during production
    if( i && !pQA->isFileComplete )
       continue;

    if( !is[0+i] || !is[2+i] )
    {
      std::string key("1_6g");
      if( notes->inq( key, pQA->fileStr) )
      {
        std::string token;

        std::string capt("Misaligned ");
        if( i == 0 )
          capt += "begin" ;
        else
          capt += "end" ;
        capt += " of periods in filename and ";

        size_t ix;

        if( pQA->qaTime.isTimeBounds )
        {
          capt="time bounds: ";
          ix = 4 + i ;
        }
        else
        {
          capt="time values: ";
          ix = 2 + i ;
        }

        capt += sd[i] ;
        capt += " vs. " ;
        capt += pDates[ix]->str();

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
    }
  }

  return false;
}

void
DRS_CV::testPeriodCutRegular(std::vector<std::string> &sd,
                  std::vector<std::string>& text)
{
  // Partitioning of files check are equivalent.
  // Note that the format was tested before.
  // Note that desplaced start/end points, e.g. '02' for monthly data, would
  // lead to a wrong cut.

  bool isInstant = ! pQA->qaTime.isTimeBounds ;

  bool isBegin = pQA->fileSequenceState == 'l' || pQA->fileSequenceState == 's' ;
  bool isEnd   = pQA->fileSequenceState == 'f' || pQA->fileSequenceState == 's' ;

  std::string frequency(pQA->qaExp.getFrequency());

  // period length per file as recommended?
  if( frequency == "3hr" || frequency == "6hr" )
  {
    // same year.
    bool isA = sd[1].substr(4,4) == "1231";
    bool isB = sd[1].substr(4,4) == "0101" && sd[1].substr(8,2) == "00" ;

    // should be the same year
    if( isBegin && isEnd )
    {
      int yBeg=hdhC::string2Double( sd[0].substr(0,4) );
      int yEnd=hdhC::string2Double( sd[1].substr(0,4) );
      if( isB )  // begin of a next year
        --yEnd;

      if( (yEnd-yBeg) )
        text.push_back(": time span of a full year is exceeded");
    }

    // cut of period
    std::string s_sd0( sd[0].substr(4,4) );
    std::string s_sd1( sd[1].substr(4,4) );

    std::string s_hr0( sd[0].substr(8,2) );
    std::string s_hr1( sd[1].substr(8,2) );

    std::string t_found(", found ");
    std::string t_hr0("00");
    std::string t_hr1;

    if( frequency == "3hr" )
      t_hr1 = "21";
    else
      t_hr1 = "18";

    if( isBegin )
    {
      if( s_sd0 != "0101" )
        text.push_back( ": not the begin of the year, found " + s_sd0);

      if( isInstant )
      {
        if( s_hr0 != t_hr0 )
        {
          text.push_back(" (instantaneous + 1st date): expected hr=") ;
          text.back() += t_hr0 + t_found + s_hr0 ;
        }
      }
      else
      {
        if( sd[0].substr(8,2) != t_hr0 )
        {
          text.push_back(" (average + 1st date): expected hr=");
          text.back() += t_hr0 + t_found + s_hr0 ;
        }
      }
    }

    if( isEnd )
    {
      if( !(isA || isB)  )
        text.push_back( ": not the end of the year, found " + s_sd1);

      if( isInstant )
      {
        if( s_hr1 != t_hr1 )
        {
          text.push_back(" (instantaneous + 2nd date): expected hr=");
          text.back() += t_hr1 + t_found + s_hr1 ;
        }
      }
      else
      {
        if( isA && s_hr1 != "24" )
        {
          text.push_back(" (averaged + 1st date): expected hr=");
          text.back() += t_hr1 + t_found + s_hr1 ;
        }
        else if( isB && s_hr1 != "00" )
        {
          text.push_back(" (averaged + 2nd date): expected hr=");
          text.back() += t_hr1 + t_found + s_hr1 ;
        }
      }
    }
  }

  else if( frequency == "day" )
  {
     // 5 years or less
     if( isBegin && isEnd )
     {
      int yBeg=hdhC::string2Double(sd[0].substr(0,4));
      int yEnd=hdhC::string2Double(sd[1].substr(0,4));
      if( (yEnd-yBeg) > 5 )
        text.push_back(": time span of 5 years is exceeded");
     }

     if( isBegin )
     {
       if( ! (sd[0][3] == '1' || sd[0][3] == '6') )
         text.push_back(": StartTime should begin with YYY1 or YYY6");

      if( sd[0].substr(4,4) != "0101" )
         text.push_back(": StartTime should be YYYY0101");
     }

     if( isEnd )
     {
       if( ! (sd[1][3] == '0' || sd[1][3] == '5') )
         text.push_back(": EndTime should begin with YYY0 or YYY5");

       std::string numDec(hdhC::double2String(
                          pQA->qaTime.refDate.regularMonthDays[11]));
       if( sd[1].substr(4,4) != "12"+numDec )
         text.push_back(": EndTime should be YYYY12"+numDec);
     }
  }
  else if( frequency == "mon" )
  {
     if( isBegin && isEnd )
     {
      // 10 years or less
      int yBeg=hdhC::string2Double(sd[0].substr(0,4));
      int yEnd=hdhC::string2Double(sd[1].substr(0,4));
      if( (yEnd-yBeg) > 10 )
        text.push_back(": time span of 10 years is exceeded");
     }

     if( isBegin )
     {
       if( sd[0][3] != '1')
         text.push_back(": StartTime should begin with YYY1");

       if( sd[0].substr(4,4) != "01" )
         text.push_back(": StartTime should be YYYY01");
     }

     if( isEnd )
     {
       if( ! (sd[1][3] == '0' || sd[1][3] == '5') )
         text.push_back(": EndTime should begin with YYY0");

       std::string numDec(hdhC::double2String(
                          pQA->qaTime.refDate.regularMonthDays[11]));
       if( sd[1].substr(4,4) != "12" )
         text.push_back(": EndTime should be YYYY12"+numDec);
     }
  }
  else if( frequency == "sem" )
  {
     if( isBegin && isEnd )
     {
      // 10 years or less
      int yBeg=hdhC::string2Double(sd[0].substr(0,4));
      int yEnd=hdhC::string2Double(sd[1].substr(0,4));
      if( (yEnd-yBeg) > 11 )  // because of winter across two year
        text.push_back(": time span of 10 years is exceeded");
     }

      if( isBegin )
      {
        if( sd[0].substr(4,2) != "12" )
          text.push_back(": StartTime should be YYYY12");
      }

      if( isEnd )
      {
        if( sd[1].substr(4,2) != "11" )
          text.push_back(": EndTime should be YYYY11");
      }
  }

  return;
}

bool
DRS_CV::testPeriodCut_CMOR_isGOD(std::vector<std::string> &sd, Date**)
{
  // some pecularities of CMOR, which will probably not be modified
  // for a behaviour as expected.
  // The CORDEX Archive Design doc states in Appendix C:
  // "..., for using CMOR, the StartTime-EndTime element [in the filename]
  //  is based on the first and last time value included in the file ..."

  // in such cases, saisonal's first and last month, respectively,
  // is shifted by one month in the filename

  bool isBegin = pQA->fileSequenceState == 'l' || pQA->fileSequenceState == 's' ;
  bool isEnd   = pQA->fileSequenceState == 'f' || pQA->fileSequenceState == 's' ;

  std::string frequency(pQA->qaExp.getFrequency());

  bool isRet=false;

  // period length per file as recommended?
  if( frequency == "sem" )
  {
    // CMOR cuts the period on both ends; the resulting months
    // for Start/End time are equal
    int shiftedMon[]={ 1, 4, 7, 10};

    if( isBegin )
    {
      int currMon = hdhC::string2Double(sd[0].substr(4,2));
      for( size_t i=0 ; i < 4 ; ++i )
      {
        if( currMon == shiftedMon[i] )
        {
          isRet=true;
          break;
        }
      }
    }

    if( !isRet && isEnd )
    {
      int currMon = hdhC::string2Double(sd[0].substr(4,2));
      for( size_t i=0 ; i < 4 ; ++i )
      {
        if( currMon == shiftedMon[i] )
        {
          isRet=true;
          break;
        }
      }
    }
  }

  if(isRet)
  {
    std::string key("1_6d");
    std::string capt("period in the filename: ") ;
    capt +="CMOR shifted StartTime-EndTime by one month";

    if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
    {
      (void) notes->operate(capt) ;

      notes->setCheckMetaStr( pQA->fail );
    }
  }

  return isRet;
}

bool
DRS_CV::testPeriodDatesFormat(std::vector<std::string> &sd)
{
  // return: true means go on for testing the period cut
  std::string key("1_6f");
  std::string capt;
  std::string str;
  std::string frequency(pQA->qaExp.getFrequency());

  // partitioning of files
  if( frequency == "3hr" || frequency == "6hr" )
  {
      if( sd[0].size() != 10 || sd[1].size() != 10 )
      {
        str += "YYYYMMDDhh for ";
        if( frequency == "3hr" )
          str += "3";
        else
          str += "6";
        str += "-hourly time step";
      }
  }
  else if( frequency == "day" )
  {
      if( sd[0].size() != 8 || sd[1].size() != 8 )
        str += "YYYYMMDD for daily time step";
  }
  else if( frequency == "mon" || frequency == "sem" )
  {
     if( sd[0].size() != 6 || sd[1].size() != 6 )
     {
        str += "YYYYMM for ";
        if( frequency == "mon" )
          str += "monthly";
        else
          str += "seasonal";
        str += " data";
     }
  }

  if( str.size() )
  {
     if( notes->inq( key, pQA->fileStr) )
     {
        capt = "period in filename of incorrect format";
        capt += ", found " + sd[0] + "-" +  sd[1];
        capt += " expected " + str ;

        (void) notes->operate(capt) ;

        notes->setCheckMetaStr( pQA->fail );
     }
  }

  return true;
}

bool
DRS_CV::testPeriodFormat(Split& x_f, std::vector<std::string> &sd)
{
  int x_fSz=x_f.size();

  if( ! x_fSz )
    return true; // trapped before

  // any geographic subset? Even with wrong separator '_'?
  if( x_f[x_fSz-1].substr(0,2) == "g-" )
    --x_fSz;
  else if( x_f[x_fSz-1] == "g" )
    --x_fSz;
  else if( x_fSz > 1 && x_f[x_fSz-2].substr(0,2) == "g" )
    x_fSz -= 2 ;

  if( x_fSz < 1 )
    return true;

  if( x_f[x_fSz-1] == "clim" || x_f[x_fSz-1] == "ave" )
  {
    // Wrong separator for appendix clim or ave, found '_'
    std::string key = "1_6b" ;
    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt("Wrong separation of filename's period's appendix");
      capt += hdhC::tf_val(x_f[x_fSz-1]);
      capt += ", found underscore";

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr( pQA->fail );
    }

    --x_fSz;
  }

  if( x_fSz < 1 )
    return true;

  Split x_last(x_f[x_fSz-1],'-') ;
  int x_lastSz = x_last.size();

  // minimum size of x_last could never be zero

  // elimination of 'ave' or 'clim' separated by a dash
  if( x_last[x_lastSz-1] == "clim" || x_last[x_lastSz-1] == "ave" )
    --x_lastSz ;

  bool isRegular=true;
  if( x_lastSz == 2 )
  {
    // the regular case
    std::vector<int> ix;
    if( hdhC::isDigit(x_last[0]) )
      sd[0]=x_last[0] ;
    else
    {
      ix.push_back(0);
      isRegular=false;
    }

    if( hdhC::isDigit(x_last[1]) )
      sd[1]=x_last[1] ;
    else
    {
      isRegular=false;
      ix.push_back(1);
    }

    if( ix.size() == 2 )
      return true;  // apparently not a period
    else if( ix.size() == 1 )
    {
      // try for clim or ave without any separator with a wrong one
      size_t pos;
      std::string f(x_last[ix[0]]);
      if( (pos=f.rfind("clim")) < std::string::npos
             || (pos=f.rfind("ave")) < std::string::npos )
      {
        if( pos && hdhC::isDigit( f.substr(0, pos-1) ) )
        {
          if( ix[0] == 1 )
            isRegular = true;

          sd[ix[0]] = f.substr(0,pos-1) ;
        }
        else if( pos && hdhC::isDigit( f.substr(0,pos) ) )
        {
          if( ix[0] == 1 )
            isRegular = true;

          sd[ix[0]] = f.substr(0,pos) ;
        }
        else
          isRegular=false;

        if(ix.size())
        {
          // Wrong separator for appendix clim or ave, found '_'
          std::string key = "1_6b" ;
          if( notes->inq( key, pQA->fileStr) )
          {
            std::string capt("Wrong separation of filename's period's appendix");
            capt += hdhC::tf_val(x_last[ix[0]]);

            (void) notes->operate(capt) ;
            notes->setCheckMetaStr( pQA->fail );
          }
        }
      }
    }
  }
  else
    isRegular=false;

  if( ! isRegular )
  {
    std::string f;
    if( sd[0].size() )
      f = x_last[1];
    else
      f = x_last[0];

    // could be a period with a wrong separator
    std::string sep;
    size_t pos=std::string::npos;

    bool isSep=false;
    for( size_t i=0 ; i < f.size() ; ++i )
    {
      if( !hdhC::isDigit(f[i]) )
      {
        if(isSep)
          return true;  // not a wrong separator, just anything

        isSep=true;
        sep=f[i];
        pos=i;
      }
    }

    if( pos < std::string::npos )
    {
      sd[0]=f.substr(0, pos) ;
      sd[1]=f.substr(pos+1) ;

      // Wrong separator for appendix clim or ave, found '_'
      std::string key = "1_6b" ;
      if( notes->inq( key, pQA->fileStr) )
      {
        std::string capt("Wrong separation of filename's period's dates");
        capt += ", found";
        capt += hdhC::tf_val(sep);

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
    }
  }

  // in case of something really nasty
  if( !( hdhC::isDigit(sd[0]) && hdhC::isDigit(sd[1]) ) )
    return true;

  return false;
}

QA_Exp::QA_Exp()
{
  initDefaults();
}

void
QA_Exp::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "cIVN"
            || split[0] == "CaseInsensitiveVariableName"
                 || split[0] == "case_insensitive_variable_name" )
     {
          isCaseInsensitiveVarName=true;
          continue;
     }

     if( split[0] == "eA" || split[0] == "excludedAttribute"
         || split[0] == "excluded_attribute" )
     {
       Split cvs(split[1],",");
       for( size_t i=0 ; i < cvs.size() ; ++i )
         excludedAttribute.push_back(cvs[i]);

       continue;
     }
   }

   return;
}

void
QA_Exp::createVarMetaData(void)
{
  // create instances of VariableMetaData:375. These have been identified
  // previously at the opening of the nc-file and marked as
  // Variable::VariableMeta(Base)::isDATA == true. The index
  // of identified targets is stored in vector in.dataVarIndex.

  for( size_t i=0 ; i < pQA->pIn->dataVarIndex.size() ; ++i )
  {
    Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[i]];

    //push next instance
    pushBackVarMeDa( &var );
  }

  return;
}

std::string
QA_Exp::getFrequency()
{
  if( frequency.size() )
    return frequency;  // already known

  // get frequency from attribute (it is required)
  frequency = pQA->pIn->nc.getAttString("frequency") ;

  return frequency ;
}

std::string
QA_Exp::getTableEntryID(std::string vName)
{
  vName += "," ;
  vName += getFrequency() ;

  return vName + ",";
}

void
QA_Exp::init(std::vector<std::string>& optStr)
{
   // apply parsed command-line args
   applyOptions(optStr);

   // Create and set VarMetaData objects.
   createVarMetaData() ;

   return ;
}

void
QA_Exp::initDataOutputBuffer(void)
{
    for( size_t i=0 ; i < varMeDa.size() ; ++i)
      varMeDa[i].qaData.initBuffer(pQA, pQA->currQARec, bufferSize);

  return;
}

void
QA_Exp::initDefaults(void)
{
  isCaseInsensitiveVarName=false;
  isUseStrict=false;

  bufferSize=1500;

  return;
}

void
QA_Exp::initResumeSession(std::vector<std::string>& prevTargets)
{
  if( !pQA->isCheckData )
    return;
  
  // a missing variable?
  for( size_t i=0 ; i < prevTargets.size() ; ++i)
  {
    size_t j;
    for( j=0 ; j < varMeDa.size() ; ++j)
      if( prevTargets[i] == varMeDa[j].name )
        break;

    if( j == varMeDa.size() )
    {
       std::string key("33a");
       if( notes->inq( key, prevTargets[i]) )
       {
         std::string capt("variable=");
         capt += prevTargets[i] + " is missing in sub-temporal file" ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( pQA->fail );
         }
       }
    }
  }

  // a new variable?
  for( size_t j=0 ; j < varMeDa.size() ; ++j)
  {
    size_t i;
    for( i=0 ; i < prevTargets.size() ; ++i)
      if( prevTargets[i] == varMeDa[j].name )
        break;

    if( i == prevTargets.size() )
    {
       std::string key("33b");
       if( notes->inq( key, varMeDa[j].name) )
       {
         std::string capt("variable=");
         capt += varMeDa[j].name + " is new in sub-temporal file" ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( pQA->fail );
         }
       }
    }
  }

  return;
}


void
QA_Exp::pushBackVarMeDa(Variable *var)
{
   varMeDa.push_back( VariableMetaData(pQA, var) );

   if( var )
   {
     VariableMetaData &vMD = varMeDa.back();

     vMD.forkAnnotation(notes);

     // disable tests by given options
     vMD.qaData.disableTests(var->name);

     vMD.qaData.init(pQA, var->name);
   }

   return;
}

void
QA_Exp::run(void)
{
   return ;
}

void
QA_Exp::setParent(QA* p)
{
   pQA = p;
   notes = p->notes;
   return;
}

bool
QA_Exp::testPeriod(void)
{
  return false;
}

VariableMetaData::VariableMetaData(QA *p, Variable *v)
{
   pQA = p;
   var = v;
}

VariableMetaData::~VariableMetaData()
{
  qaData.dataOutputBuffer.clear();
}

void
VariableMetaData::verifyPercent(void)
{
   // % range
   if( var->units == "%" )
   {
      if( qaData.statMin.getSampleMin() >= 0.
           && qaData.statMax.getSampleMax() <= 1. )
      {
        std::string key("6_8");
        if( notes->inq( key, var->name) )
        {
          std::string capt( hdhC::tf_var(var->name, ":"));
          capt += "Suspicion of fractional data range for units [%], found range ";
          capt += "[" + hdhC::double2String(qaData.statMin.getSampleMin());
          capt += ", " + hdhC::double2String(qaData.statMax.getSampleMax()) + "]" ;

          (void) notes->operate(capt) ;
          notes->setCheckMetaStr( pQA->fail );
        }
      }
   }

   if( var->units == "1" || var->units.size() == 0 )
   {
      // is it % range? Not all cases are detectable
      if( qaData.statMin.getSampleMin() >= 0. &&
           qaData.statMax.getSampleMax() > 1.
             && qaData.statMax.getSampleMax() <= 100.)
      {
        std::string key("6_9");
        if( notes->inq( key, var->name) )
        {
          std::string capt( "Suspicion of percentage data range for units <1>, found range " ) ;
          capt += "[" + hdhC::double2String(qaData.statMin.getSampleMin());
          capt += ", " + hdhC::double2String(qaData.statMax.getSampleMax()) + "]" ;

          (void) notes->operate(capt) ;
          notes->setCheckMetaStr( pQA->fail );
        }
      }
   }

   return;
}

int
VariableMetaData::finally(int xCode)
{
  // write pending results to qa-file.nc. Modes are considered there
  qaData.flush();

  // annotation obj forked by the parent VMD
  notes->printFlags();

  int rV = notes->getExitValue();
  xCode = ( xCode > rV ) ? xCode : rV ;

  return xCode ;
}

void
VariableMetaData::forkAnnotation(Annotation *n)
{
   notes = new Annotation(n);

   // this is not a mistaken
   qaData.setAnnotation(n);

   return;
}

void
VariableMetaData::setAnnotation(Annotation *n)
{
   notes = n;

   qaData.setAnnotation(n);

   return;
}
