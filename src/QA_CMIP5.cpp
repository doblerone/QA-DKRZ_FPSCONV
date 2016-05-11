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
DRS_CV::checkFilename(std::string& fName, struct DRS_CV_Table& drs_cv_table)
{
  Split x_filename;
  x_filename.setSeparator("_");
  x_filename.enableEmptyItems();
  x_filename = fName ;

  checkVariableName(x_filename[0]) ;

  checkFilenameGeographic(x_filename);

  std::string frq = pQA->qaExp.getFrequency();

  if( testPeriod(x_filename) && frq.size() && frq != "fx" )
  {
    // no period in the filename
    std::string key("1_6a");

    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt("filename requires a period") ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("DRS", pQA->n_fail);
    }
  }

  checkFilenameEncoding(x_filename, drs_cv_table);

  return;
}

void
DRS_CV::checkFilenameEncoding(Split& x_filename, struct DRS_CV_Table& drs_cv_table)
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
  std::string t;

  size_t enc_sz = drs_cv_table.fileEncoding.size() ;

  Split x_enc[enc_sz];
  std::vector<size_t> countCI(enc_sz, 0);
  std::map<std::string, std::string> globMap[enc_sz] ;
  std::vector<std::vector<size_t> > specialFaultIx ;

  for( size_t ds=0 ; ds < enc_sz ; ++ds)
  {
    Split& x_e = x_enc[ds] ;
    std::map<std::string, std::string>& gM = globMap[ds] ;
    specialFaultIx.push_back( std::vector<size_t>() );

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
        capt += pQA->drs_cv_table.table_DRS_CV.getFile() ;
        capt += ": encoding ";
        capt += hdhC::tf_assign("item", x_e[x]) + " not found in CV";
        if( notes->inq( key, "DRS") )
        {
          (void) notes->operate(capt) ;
          notes->setCheckStatus("DRS", pQA->n_fail);
        }
      }

      if( cvMap[x_e[x]] == n_ast )
      {
        if( x_e[x] == "variable name" )
          gM[x_e[x]] = pQA->qaExp.fVarname ;

        else if( x_e[x] == "ensemble member" )
          gM[x_e[x]] = getEnsembleMember() ;

        else if( x_e[x] == "gridspec" )
          gM[x_e[x]] = "gridspec" ;

        else if( x_e[x] == "temporal subset" )
          gM[x_e[x]] = n_ast ;

        else if( x_e[x] == "geographical info" )
          gM[x_e[x]] = n_ast ;

        else if( x_e[x] == "version" )
          gM[x_e[x]] = n_ast ;
      }
      else if( x_e[x] == "MIP table" )
        gM[x_e[x]] = pQA->qaExp.getMIP_tableName() ;
      else
        gM[x_e[x]] = globalVar.getAttValue(cvMap[x_e[x]]) ;
    }

    // special for gridspec filenames
    if( drs_cv_table.fileEncodingName[ds] == "GRIDSPEC" )
    {
      for( size_t i=0 ; i < drs.size() ; ++i)
      {
        // frequency
        if( x_e[i] == "frequency" )
        {
          t = gM[ x_e[i] ];
          if( t != "fx" || drs[i] != "fx" )
            specialFaultIx[ds].push_back(i);
        }

        // ensemble member
        if( x_e[i] == "ensemble member" )
        {
          t = gM[ x_e[i] ];
          if( t != "r0i0p0" )
            specialFaultIx[ds].push_back(i);
        }
      }
    }


    // append * twice in case of no geo-info nor period;
    // surplusses are safe
    drs.append("*");
    drs.append("*");

    //count coincidences between path components and the DRS
    for( size_t jx=0 ; jx < x_e.size() ; ++jx)
    {
      t = gM[ x_e[jx] ];

      if( jx == drs.size() )
        break;

      if( drs[jx] == t || t == n_ast )
        ++countCI[ds];
    }

    if( ! specialFaultIx[ds].size() )
      if( countCI[ds] == x_e.size() )
        return;  // check passed; take into account two optional items
  }

  // find the encoding with maximum number of coincidences, which
  // is assumed to correspond to the current encoding type.
  size_t m=0;
  size_t mx=countCI[0];
  for( size_t ds=1 ; ds < countCI.size() ; ++ds )
  {
    if( countCI[ds] > mx )
    {
      mx = countCI[ds] ;
      m=ds;
    }
  }

  // find details of the faults
  Split& x_e = x_enc[m] ;
  std::map<std::string, std::string>& gM = globMap[m] ;

  std::vector<std::string> text;
  std::vector<std::string> keys;

  for( size_t i=0 ; i < drs.size() ; ++i)
  {
    t = gM[ x_e[i] ];

    if( hdhC::isAmong(i, specialFaultIx[m]) )
    {
      if( x_e[i] == "frequency" )
      {
        keys.push_back("1_5b");
        text.push_back( "A gridspec file must have frequency fx" );
      }

      if( x_e[i] == "ensemble member" )
      {
        keys.push_back("1_5a");
        text.push_back( "A gridspec file must have ensemble member r0i0p0" );
      }
    }
  }

  std::string txt;
  findFN_faults(drs, x_e, gM, txt) ;
  if( txt.size() )
  {
     text.push_back(txt);
     keys.push_back("1_2");
  }

  if( text.size() )
  {
    std::string capt("DRS CV filename:");
    for(size_t i=0 ; i < text.size() ; ++i )
    {
      if( notes->inq( keys[i], "DRS") )
      {
        (void) notes->operate(capt+text[i]) ;
        notes->setCheckStatus("DRS", pQA->n_fail);
      }
    }
  }

  return;
}

void
DRS_CV::checkFilenameGeographic(Split& x_filename)
{
  // the geographical indicator should be the last item
  int i;
  int last = x_filename.size()-1 ;

  for( i=last ; i >= 0 ; --i )
  {
    if( x_filename[i].size() > 1 )
    {
      if( x_filename[i][0] == 'g' && x_filename[i][1] == '-' )
        break;
    }
  }

  if( i == -1 )
    return ;  // no geo-indicator

  Split x_geo(x_filename[i], "-");

  std::vector<std::string> key;
  std::vector<std::string> capt;
  std::string cName("geographical indicator");
  if( i != last )
  {
    // the geographical indicator should be the last item
    std::string bkey("1_7a");
    if( notes->inq( key[i], pQA->fileStr))
    {
      std::string capt(cName);
      capt += " should appear last in the filename" ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("DRS", pQA->n_fail);
    }
  }

  if( x_geo.size() < 2 )
  {
    // the geographical indicator should be the last item
    std::string bkey("1_7b");

    if( notes->inq( key[i], pQA->fileStr))
    {
      std::string capt(cName);
      capt += " g-XXXX[-YYYY]: syntax fault" ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("DRS", pQA->n_fail);
    }

    return ;
  }

  // valid stand-alone '-' separated words
  std::vector<std::string> kw;
  kw.push_back(CMOR::n_global);
  kw.push_back("lnd");
  kw.push_back("ocn");
  kw.push_back("zonalavg");
  kw.push_back("areaavg");

  size_t kw_pos_zzz=3;

  // entries (1st char): (b)ounding-box, (r)egion, (t)ype, (f)ault
  std::vector<char> seq ;

  for( size_t ix=1 ; ix < x_geo.size() ; ++ix )
  {
    if( ix == 1 )
    {
      // a) bounding box?
      Split x_box;
      x_box.setSeparator("lat");
      x_box.addSeparator("lon");
      x_box.setItemsWithSeparator();
      x_box = x_geo[ix];

      for(size_t n=0 ; n < x_box.size() ; ++n )
      {
        bool isLat=false ;
        bool isLon=false ;
        double val=0.;

        if( x_box[n] == "lat" )
        {
          isLat = true ;
          isLon = false ;
          val=90.;
          if( seq.size() == 0 )
            seq.push_back('b');
          continue;
        }
        else if( x_box[n] == "lon" )
        {
          isLon = true ;
          isLat = false ;
          val=180.;
          if( seq.size() == 0 )
            seq.push_back('b');
          continue;
        }

        Split x_term;
        x_term.setSeparator(":alnum:");
        x_term = x_box[n];

        for( size_t jx=0; jx < x_term.size() ; ++jx )
        {
          // a number is expected
          if( hdhC::isDigit(x_term[jx]) )
          {
            if( x_term[jx].find('.') < std::string::npos )
            {
              key.push_back("1_7c");
              capt.push_back(cName + ": numbers should be rounded to the nearest integer");
            }

            if( hdhC::string2Double(x_term[jx]) > val )
            {
              key.push_back("1_7d");
              capt.push_back(cName);
              if(isLat)
                capt.back() += ": latitude value should not exceed 90 degr";
              else if(isLon)
                capt.back() += ": longitude value should not exceed 180 degr";
            }
          }

          // range indicator
          ++jx;
          bool is_SNWE_fault=false;

          if( isLat && ! (x_term[jx] == "S" || x_term[jx] == "N") )
            is_SNWE_fault=true;
          else if( isLon && ! (x_term[jx] == "E" || x_term[jx] == "W") )
            is_SNWE_fault=true;

          if( is_SNWE_fault )
          {
            key.push_back("1_7e");
            capt.push_back(cName + ": invalid bounding-box ");
          }
        }
      }
    }  // ix==1

    if( ix ==1 && seq.size() )
      continue;

    size_t k;
    for(k=0 ; k < kw.size() ; ++k )
      if( kw[k] == x_geo[ix] )
        break;

    if( k < kw_pos_zzz )
      seq.push_back('r') ;
    else if( k < kw.size() )
      seq.push_back('t') ;
    else
      seq.push_back('f') ;

  } // geo_ix

  // a type requires a preceding BB or region
  for( size_t i=0 ; i < seq.size() ; ++i )
  {
    if( seq[i] == 't')
    {
      bool isA = !( i && (seq[i-1] == 'b' || seq[i-1] == 'r') ) ;
      bool isB = !( i && (seq[i-1] == 'f'  ) ) ;
      if( isA && isB )
      {
        key.push_back("1_7g");
        capt.push_back(cName + ": invalid specifier ");
        capt.back() += "g-XXXX[-yyy][-zzz]: given zzz, but missing XXXX";
      }
    }
  }

  if( hdhC::isAmong('f', seq) )
  {
    // 1st index for 'g'
    for( size_t i=1 ; i < x_geo.size() ; ++i )
    {
      size_t j;
      for( j=0 ; j < kw.size() ; ++j )
      {
        if( kw[j] == x_geo[i] )
          break;

        std::string t(x_geo[i].substr(0,3));
        if( t == "lat" || t == "lon" )
          break;
      }

      if( j == kw.size() )
      {
        key.push_back("1_7f");
        capt.push_back(cName + ": invalid specifier");
        capt.back() += hdhC::tf_val(x_geo[i]) ;
        capt.back() += ", valid ";
        if( kw.size() > 1 )
          capt.back() += "are";
        else
          capt.back() += "is";
        capt.back() += hdhC::tf_val( hdhC::getVector2Str(kw) );

      }
    }
  }


  for( size_t ii=0 ; ii < key.size() ; ++ii )
  {
    if( notes->inq( key[ii], pQA->fileStr))
    {
      (void) notes->operate(capt[ii]) ;
      notes->setCheckStatus("DRS", pQA->n_fail);
    }
  }

  return ;
}

void
DRS_CV::checkMIPT_tableName(Split& x_filename)
{
  // Note: filename:= name_CMOR-MIP-table_... .nc
  if( x_filename.size() < 2 )
    return ;  //corrupt filename

  // table sheet name from global attributes has been checked

  // compare file entry to the one in global header attribute table_id
  if( x_filename[1] != QA::tableSheet )
  {
    std::string key("7_8");

    if( notes->inq( key, "MIP") )
    {
      std::string capt("Ambiguous MIP table names, found ") ;
      capt += hdhC::tf_assign("MIP-table(file)", x_filename[1]) ;
      capt += " vs. global ";
      capt += hdhC::tf_att(CMOR::n_table_id, QA::tableSheet);

      (void) notes->operate(capt) ;
      notes->setCheckStatus("DRS", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    // try the filename's MIP table name
    if( QA::tableSheet.size() == 0 )
      QA::tableSheet = pQA->qaExp.getMIP_tableName(x_filename[1]) ;
  }

  return ;
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
      gM[CMOR::n_product] = drs_product ;
      return;
    }
  }

  std::string key("1_3");

  if( notes->inq( key, "DRS") )
  {
    std::string capt("DRS CV fault for path component");
    capt += hdhC::tf_val(CMOR::n_product);
    capt += ", found" ;
    capt += hdhC::tf_val(drs_product) ;
    capt += ", expected one of";
    capt += hdhC::tf_val(prod_choice);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("DRS", pQA->n_fail);
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
        capt += ": encoding not available in CV, found " ;
        capt += hdhC::tf_assign("item", x_e[x]) ;

        if( notes->inq( key, "DRS") )
        {
          (void) notes->operate(capt) ;
          notes->setCheckStatus("DRS", pQA->n_fail);
        }
      }

      if( cvMap[x_e[x]] == n_ast )
      {
        if( x_e[x] == "variable name" )
          gM[x_e[x]] = pQA->qaExp.fVarname ;

        else if( x_e[x] == "ensemble member" )
          gM[x_e[x]] = getEnsembleMember() ;

        else if( x_e[x] == "MIP table" )
          gM[x_e[x]] = pQA->qaExp.getMIP_tableName() ;

        else if( x_e[x] == "gridspec" )
          gM[x_e[x]] = "gridspec" ;
      }
      else
      {
        gM[x_e[x]] = globalVar.getAttValue(cvMap[x_e[x]]) ;

        if( x_e[x] == "MIP table" && gM[x_e[x]].substr(0,6) == "Table " )
        {
          size_t pos=6;
          if( (pos=gM[x_e[x]].find(" ", pos)) < std::string::npos )
            gM[x_e[x]] = gM[x_e[x]].substr(6, pos-6) ;
        }
      }
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
        if( drs[i] == t || t == n_ast )
          ++countCI[ds];
        else if( x_e[j] == "version number" && hdhC::isDigit(drs[i].substr(1)) )
          ++countCI[ds];

        // drs may have a different value, e.g. output1
        else if( x_e[j] == CMOR::n_product )
        {
          checkProductName(drs[i], cvMap[CMOR::n_product+"_constr"], gM);
          ++countCI[ds];
        }
      }
      else
        drs.append(n_ast); // who knows
    }

    if( countCI[ds] == x_e.size() )
      return;  // check passed
  }

  // find the encoding with maximum number of coincidences
  size_t m=0;
  size_t mx=countCI[0];
  for( size_t ds=1 ; ds < countCI.size() ; ++ds )
  {
    if( countCI[ds] > mx )
    {
      mx = countCI[ds] ;
      m=ds;
    }
  }

  std::vector<std::string> text;
  std::vector<std::string> keys;

  Split& x_e = x_enc[m] ;
  std::map<std::string, std::string>& gM = globMap[m] ;

  std::string txt;
  findPath_faults(drs, x_e, gM, txt) ;
  if( txt.size() )
  {
    text.push_back(txt);
    keys.push_back("1_2");
  }

  if( text.size() )
  {
    std::string capt("DRS CV path:");

    for(size_t i=0 ; i < text.size() ; ++i )
    {
      if( notes->inq( keys[i], pQA->fileStr) )
      {
        (void) notes->operate(capt+text[i]) ;
        notes->setCheckStatus("DRS", pQA->n_fail);
      }
    }
  }

  return;
}

void
DRS_CV::checkVariableName(std::string& f_vName)
{
  if( f_vName.find('-') < std::string::npos )
  {
    std::string key("1_4c");

    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt(hdhC::tf_var(f_vName));
      capt += "in the filename should not contain a hyphen" ;

      (void) notes->operate(capt) ;
      pQA->setExit( notes->getExitValue() ) ;
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

    if( drs[j] == CMOR::n_output )
    if( !(drs[j] == t || t == n_ast) )
    {
      text = " check failed, found " ;
      text += hdhC::tf_assign(x_e[j],drs[j]) ;
      text += ", expected" ;
      text += hdhC::tf_val(t) ;

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

  int drsBeg=-1;

  if( drsSz > x_eSz )
  {
    std::vector<int> count(drsSz, 0);

    for( int i=0 ; i < drsSz ; ++i )
    {
      for( int j=0 ; j < x_eSz ; ++j )
      {
        int k = i+j;
        if( k < drsSz && drs[k] == gM[ x_e[j] ] )
          ++count[i];
      }
    }

    // highest count determines drsBeg
    int countMax=count[0];
    drsBeg=0;
    for( size_t i=1 ; i < count.size() ; ++i )
    {
      if( countMax < count[i] )
      {
        countMax = count[i];
        drsBeg = i;
      }
    }
  }

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
        if( notes->inq( "1_1a", pQA->fileStr, "INQ_ONLY") )
        {
          std::string s( hdhC::Upper()(drs[i]) ) ;
          if( s == t )
            continue;
        }
      }

      if(x_e[j] == "version number" )
      {
        if( drs[i][0] == 'v' && hdhC::isDigit(drs[i].substr(1)) )
          continue;
      }

      text = " check failed, path component " ;
      text += hdhC::tf_assign(x_e[j],drs[drsBeg+j]) ;
      text += " does not match global attribute value " ;
      text += hdhC::tf_val(t) ;
      break;
    }
  }

  return ;
}

std::string
DRS_CV::getEnsembleMember(void)
{
  if( ensembleMember.size() )
    return ensembleMember;

  // ensemble member
  int ix;
  size_t g_ix = pQA->pIn->varSz ;  // index to the global atts

  ensembleMember = "r" ;
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("realization")) > -1 )
    ensembleMember += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  ensembleMember += 'i' ;
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("initialization_method")) > -1 )
    ensembleMember += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  ensembleMember += 'p' ;
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("physics_version")) > -1 )
    ensembleMember += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  return ensembleMember ;
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
      if( x_e[j] == CMOR::n_activity )
      {
        isActivity=true;
        s = hdhC::Upper()(s);
      }
      else
        isActivity=false;

      if( s == gM[ x_e[j] ]  )
      {
        ix = static_cast<int>(i);

        // the match could be a leading false one, requested is the last one
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

bool
DRS_CV::isInstantTime(void)
{

  if( pQA->qaExp.getFrequency() == "6hr" )
    return true;

  bool is=true;
  for( size_t i=0 ; i < pQA->qaExp.varMeDa.size() ; ++i )
    if( pQA->qaExp.varMeDa[i].attMap[CMOR::n_cell_methods].size()
          && pQA->qaExp.varMeDa[i].attMap[CMOR::n_cell_methods] != "time: point" )
      is=false;

  return is;
}

void
DRS_CV::run(void)
{
  DRS_CV_Table& drs_cv_table = pQA->drs_cv_table ;

  // check the path
  checkPath(pQA->pIn->file.path, drs_cv_table) ;

  // compare filename to netCDF global attributes
  checkFilename(pQA->pIn->file.basename, drs_cv_table);

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

  // Does the filename have a trailing date range?
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
       std::string capt("invalid period in the filename, found ");
       capt += hdhC::tf_val(sd[0] + "-" + sd[1]);

       (void) notes->operate(capt) ;
       notes->setCheckStatus("DRS", pQA->n_fail );
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
  }
  else
  {
    if( pQA->qaTime.time_ix > -1 &&
        ! pQA->pIn->variable[pQA->qaTime.time_ix].isInstant )
    {
      if( !isInstantTime() )
      {
        std::string key("3_17");
        if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
        {
          std::string capt(hdhC::tf_var("time_bnds"));
          capt += "is missing" ;

          (void) notes->operate(capt) ;
          notes->setCheckStatus("DRS", pQA->n_fail);
        }
      }
    }
  }

  pDates[2] = new Date(pQA->qaTime.refDate);
  if( pQA->qaTime.firstTimeValue != 0. )
    pDates[2]->addTime(pQA->qaTime.firstTimeValue);

  pDates[3] = new Date(pQA->qaTime.refDate);
  if( pQA->qaTime.lastTimeValue != 0. )
    pDates[3]->addTime(pQA->qaTime.lastTimeValue);

  // alignment of of contained dates and those in the filename
  // the booleanx indicate faults
//  bool is_t_beg = is_t_end = is_tb_beg = is_tb_end = false;
  bool isFault[4];
  for(size_t i=0 ; i < 4 ; ++i )
    isFault[i]=false;

  // time value: left-side
  Date myDate( *pDates[2] );
  myDate.addTime(-pQA->qaTime.refTimeStep);
  isFault[0] = myDate == *pDates[0] ;

  // time value: right-side
  myDate = *pDates[3] ;
  myDate.addTime(pQA->qaTime.refTimeStep);
  isFault[1] = myDate ==*pDates[1] ;

  if(pQA->qaTime.isTimeBounds)
  {
    // time_bounds: left-side
    myDate = *pDates[4] ;
    myDate.addTime(pQA->qaTime.refTimeStep);
    isFault[2] = myDate == *pDates[0] ;

    // time_bounds: right-side
    myDate = *pDates[5] ;
    myDate.addTime(-pQA->qaTime.refTimeStep);
    isFault[3] = myDate == *pDates[1] ;
  }

  // the annotations
  if( testPeriodAlignment(sd, pDates, isFault) )
  {
    std::string key("1_6g");

    if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
    {
      std::string capt("Warn: Filename's period: No time_bounds, ");
      capt += "StartTime-EndTime rounded to time values";

      (void) notes->operate(capt) ;
      notes->setCheckStatus("DRS", pQA->n_fail );
    }
  }
  else if( testPeriodDatesFormat(sd) ) // format of period dates.
  {
    // period requires a cut specific to the various frequencies.
    std::vector<std::string> text ;
    testPeriodPrecision(sd, text) ;

    if( text.size() )
    {
      std::string key("1_6d");
      std::string capt("period in the filename with wrong precision") ;

      for( size_t i=0 ; i < text.size() ; ++i )
      {
        if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
        {
          (void) notes->operate(capt + text[i]) ;
          notes->setCheckStatus("DRS", pQA->n_fail );
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
DRS_CV::testPeriodAlignment(std::vector<std::string>& sd, Date** pDates, bool b[])
{
  // some pecularities of CMOR, which will probably not be modified
  // for a behaviour as expected.
  // The CORDEX Archive Design doc states in Appendix C:
  // "..., for using CMOR, the StartTime-EndTime element [in the filename]
  //  is based on the first and last time value included in the file ..."

  // if a test for a CMOR setting fails, testing goes on
  if( *pDates[0] == *pDates[2] && *pDates[1] == *pDates[3] )
      return true;

  for(size_t i=0 ; i < 2 ; ++i)
  {
    // i == 0: left; 1: right

    // skip for the mode of checking during production
    if( i && !pQA->isFileComplete )
       continue;

    if( !( !b[0+i] || !b[2+i] ) )
    {
      std::string key("1_6f");
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
        notes->setCheckStatus("DRS", pQA->n_fail );
      }
    }
  }

  return false;
}

void
DRS_CV::testPeriodPrecision(std::vector<std::string>& sd,
                  std::vector<std::string>& text)
{
  // Partitioning of files check are equivalent.
  // Note that the format was tested before.
  // Note that desplaced start/end points, e.g. '02' for monthly data, would
  // lead to a wrong cut.

  if( sd[0].size() != sd[1].size() )
  {
    text.push_back(" 1st and 2nd date are different") ;
    return;
  }

  // precision corresponds to the MIP table
  std::vector<size_t> ix;

  // a) yyyy
  if( QA::tableSheet == QA_Exp::MIP_tableNames[1] )
  {
    if( sd[0].size() != 4 )
      text.push_back(", expected yyyy, found " + sd[0] + "-" + sd[1]) ;

    return;
  }

  // b) ...mon, aero, Oclim, and cfOff
  ix.push_back(2);
  ix.push_back(3);
  ix.push_back(4);
  ix.push_back(5);
  ix.push_back(6);
  ix.push_back(7);
  ix.push_back(8);
  ix.push_back(13);
  ix.push_back(17);

  for(size_t i=0 ; i < ix.size() ; ++i )
  {
    if( QA::tableSheet == QA_Exp::MIP_tableNames[ix[i]] )
    {
      if( sd[0].size() != 6 )
        text.push_back(", expected yyyyMM, found " + sd[0] + "-" + sd[1]) ;

      return;
    }
  }

  // c) day, cfDay
  if( QA::tableSheet == QA_Exp::MIP_tableNames[9]
        || QA::tableSheet == QA_Exp::MIP_tableNames[14] )
  {
    if( sd[0].size() != 8 )
      text.push_back(", expected yyyyMMdd, found " + sd[0] + "-" + sd[1]) ;

    return;
  }

  // d) 6hr..., 3hr, and cf3hr
  ix.clear();
  ix.push_back(10);
  ix.push_back(11);
  ix.push_back(12);
  ix.push_back(15);

  for(size_t i=0 ; i < ix.size() ; ++i )
  {
    if( QA::tableSheet == QA_Exp::MIP_tableNames[ix[i]] )
    {
      if( sd[0].size() != 12 )
        text.push_back(", expected yyyyMMddhhmm, found " + sd[0] + "-" + sd[1]) ;

      return;
    }
  }

  // e) cfSites
  if( QA::tableSheet == QA_Exp::MIP_tableNames[16] )
  {
    if( sd[0].size() != 14 )
      text.push_back(", expected yyyyMMddhhmmss, found " + sd[0] + "-" + sd[1]) ;
  }

  return;
}

bool
DRS_CV::testPeriodDatesFormat(std::vector<std::string>& sd)
{
  // return: true means go on for testing the period cut
  // partitioning of files
  if( sd.size() != 2 )
    return false;  // no period; is variable time invariant?

  std::string frequency(pQA->qaExp.getFrequency());
  std::string str;

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
    std::string key("1_6e");

     if( notes->inq( key, pQA->fileStr) )
     {
        std::string capt("period in filename of incorrect format, found ");
        capt += sd[0] + "-" +  sd[1];
        capt += " expected " + str ;

        (void) notes->operate(capt) ;
        notes->setCheckStatus("DRS", pQA->n_fail );
     }
  }

  return true;
}

bool
DRS_CV::testPeriodFormat(Split& x_f, std::vector<std::string>& sd)
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
      notes->setCheckStatus("DRS", pQA->n_fail );
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
          std::string key("1_6b") ;
          if( notes->inq( key, pQA->fileStr) )
          {
            std::string capt("Wrong separation of filename's period's appendix");
            capt += hdhC::tf_val(x_last[ix[0]]);

            (void) notes->operate(capt) ;
            notes->setCheckStatus("DRS", pQA->n_fail );
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
      std::string key("1_6b") ;
      if( notes->inq( key, pQA->fileStr) )
      {
        std::string capt("Wrong separation of filename's period's dates");
        capt += ", found ";
        capt += hdhC::tf_val(sep);

        (void) notes->operate(capt) ;
        notes->setCheckStatus("DRS", pQA->n_fail );
      }
    }
  }

  // in case of something really nasty
  if( !( hdhC::isDigit(sd[0]) && hdhC::isDigit(sd[1]) ) )
    return true;

  return false;
}

// class for comparing meta-data from the file to the
// "CMIP5 Model Output Requirements", but global attributes
CMOR::CMOR(QA* p)
{
  pQA = p;
  pExp = &p->qaExp;
  notes = p->notes;

  initDefaults();

  applyOptions(p->optStr);
}

void
CMOR::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "pEI" || split[0] == "parentExperimentID"
         || split[0] == "parent_experiment_id" )
     {
       if( split.size() == 2 )
       {
          parentExpID=split[1];
       }
       continue;
     }

     if( split[0] == "pER" || split[0] == "parentExperimentRIP"
         || split[0] == "parent_experiment_rip" )
     {
       if( split.size() == 2 )
       {
          parentExpRIP=split[1];
       }
       continue;
     }
  }

  return;
}

void
CMOR::checkEnsembleMemItem(std::string& rqName, std::string& attVal)
{
  std::string capt;
  std::string key;

  if( ! hdhC::isDigit(attVal) )
  {
    key = "2_5a";
    capt = n_global + hdhC::blank ;
    capt += hdhC::tf_att(rqName);
    capt += "must be integer, found ";
    capt += attVal;
  }
  else
  {
    int id=atoi(attVal.c_str());

    if( id == 0 && pExp->getFrequency() != "fx" )
    {
      key = "2_5b";
      capt = n_global + hdhC::blank ;
      capt += hdhC::tf_att(rqName);
      capt += "must be an integer > 0 for";
      capt += hdhC::tf_assign(pQA->n_frequency, pExp->getFrequency());
    }

    else if( id > 0 && pExp->getFrequency() == "fx" )
    {
      key = "2_5c";
      capt = n_global + hdhC::blank ;
      capt += hdhC::tf_att(rqName);
      capt += "must be equal 0 for frequency=<fx> ";
    }
  }

  if( capt.size() )
  {
    if( notes->inq( key, n_global) )
    {
      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
CMOR::checkForcing(std::vector<std::string>& vs_rqValue, std::string& aV)
{
  if( aV == hdhC::NA )
    return;

  Split x_aV(aV, ",");

  std::vector<std::string> vs_items(x_aV.getItems());
  vs_items = hdhC::unique(vs_items, hdhC::blank);

  std::string item;
  size_t p0, p1;

  // not a comma-sep list, but separated by blanks?
  if( vs_items.size() == 1 && aV.find(" ") < std::string::npos )
  {
      std::string key("2_6b");
      if( notes->inq( key, pQA->s_global ) )
      {
        std::string capt(pQA->s_global);
        capt += hdhC::blank;
        capt += hdhC::tf_att(hdhC::empty, n_forcing, aV);
        capt += "should be a comma separated list, found blanks";

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV",  pQA->n_fail );
      }

      x_aV.setSeparator(' ');
      x_aV = aV;
      vs_items = x_aV.getItems() ;
      vs_items = hdhC::unique(vs_items, hdhC::blank);
  }

  for( size_t i=0 ; i < vs_items.size() ; ++i )
  {
    if( (p0=vs_items[i].find("(")) < std::string::npos )
    {
      if( p0 )
        item = vs_items[i].substr(0,p0);

      if( (p1=vs_items[i].find(")")) < std::string::npos )
        item += vs_items[i].substr(++p1);
    }
    else
      item=vs_items[i];

    // check
    if( ! hdhC::isAmong(item, vs_rqValue) )
    {
      std::string key("2_6a");
      if( notes->inq( key, pQA->s_global ) )
      {
        std::string capt(pQA->s_global);
        capt += hdhC::blank;
        capt += hdhC::tf_att(hdhC::empty, n_forcing, item);
        capt += "not among DRS-CV requested values";
        capt += hdhC::tf_val( hdhC::catStringVector(vs_rqValue)) ;

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV",  pQA->n_fail );

        break;
      }
    }
  }

  return;
}

bool
CMOR::checkMIP_table(InFile& in, VariableMetaData& vMD,
             std::vector<struct DimensionMetaData>& vs_f_DMD_entries)
{
   // return true for the very special case that a tracer
   // variable was not found in table sheet Omon, because
   // it is defined in Oyr

   if( !pExp->varReqTable.is )  // no standard table
      return false;

   ReadLine ifs(pExp->varReqTable.getFile());

   if( ! ifs.isOpen() )
   {
      std::string key("7_4") ;

      if( notes->inq( key, vMD.var->name) )
      {
         std::string capt("could not open table.") ;
         capt += pExp->varReqTable.basename ;

         (void) notes->operate(capt) ;
         notes->setCheckStatus("CV", pQA->n_fail);
         pQA->setExit( notes->getExitValue() ) ;
      }
   }

  // headings for variables and dimensions
  std::map<std::string, size_t> v_col;
  std::map<std::string, size_t> d_col;

  // read headings from the variable requirements table,
  // lines in the dimensions sub-sheet are stored in vs_dim_sheet_line
  std::vector<std::string> vs_dimSheet;
  std::string str0;

  readHeadline(ifs, vMD, str0, vs_dimSheet, v_col, d_col);

  // find the table sheet, corresponding to the 2nd item
  // in the variable name. The name of the table sheet is
  // present in the first column and begins with "CMOR Table".
  // The remainder is the name.
  // Unfortunately, the variable requirement table is in a shape of a little
  // distance from being perfect.

  VariableMetaData tEntry(pQA);
  VariableMetaData bufEntry(pQA);

  // try to find the name of the table sheet in str0;
  // note that Omon- 3D-tracer requires the corresponding Oyr entry, too.
  bool isCont=true;

  while (isCont)
  {
    if( findVarReqTableSheet(ifs, str0, vMD, tEntry) )
    {
      if( findVarReqTableEntry(ifs, str0, vMD, v_col, tEntry) )
      {
        // get properties of the table entry
        if( ! tEntry.attMap.empty()
              && tableSheetBuf.size() > 1 && tableSheetBuf[0] == "Oyr" )
        {
          // Omon-3D-tracer part II
          getMIPT_var(str0, bufEntry, v_col);

          tEntry.attMap[n_long_name] += " at surface" ;
          if( notes->inq( "7_14", "", "INQ_ONLY") )
            tEntry.attMap[n_long_name] = hdhC::Lower()(tEntry.attMap[n_long_name]) ;

          tEntry.attMap[n_units] = bufEntry.attMap[n_units] ;
          tEntry.attMap[n_cell_methods] = bufEntry.attMap[n_cell_methods] ;
          tEntry.attMap[n_CMOR_dimension] = bufEntry.attMap[n_CMOR_dimension] ;
          tEntry.attMap[n_cell_measures] = bufEntry.attMap[n_cell_measures] ;
        }
        else
          getMIPT_var(str0, tEntry, v_col); // normal; mostly
      }

      ++tableSheetBufIx;
    }

    if( ifs.isEOF() || (tableSheetBufIx == tableSheetBuf.size()) )
      isCont=false;
  }

  ifs.close();

  if( tEntry.attMap.empty() )
  {
    // there was no match, but we try alternatives
    std::string key("3_1") ;
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroVar(
                            pExp->varReqTable.basename, vMD));
      capt += "not found in ";
      capt += hdhC::tf_assign("sheet", QA::tableSheet);

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  // file and table properties are compared to each other
  checkMIPT_var(vMD, tEntry, v_col);

  Split x_tDims(tEntry.attMap[n_CMOR_dimension]);

  for( size_t i=0 ; i < x_tDims.size() ; ++i )
  {
    // check basic properties between the file and
    // requests in the table.
    checkMIPT_dim(vs_dimSheet, vMD, d_col, x_tDims[i]);
  }

  // variable not found in the table.
  return false;
}

void
CMOR::checkMIPT_dim(std::vector<std::string>& vs_dimSheet,
  VariableMetaData& vMD,
  std::map<std::string, size_t>& col, std::string& CMORdim)
{
  // the CMOR-dimension column of the dim-sheet provides a
  // connection to the var entries of the other tables.
  struct DimensionMetaData t_DMD ;

  std::vector<std::string> vs_tOut;

  getDimSheetEntry(CMORdim, vs_dimSheet, col, t_DMD);

  // find corresponding var-representive name f the dim in the file
  size_t ix;
  for( ix=0 ; ix < pQA->pIn->varSz ; ++ix)
  {
    if( pQA->pIn->variable[ix].name == t_DMD.attMap[n_output_dim_name] )
      break;
  }

  if( ix == pQA->pIn->varSz )
  {
    std::string key("4_1a");
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(hdhC::tf_assign(n_CMOR_dimension,CMORdim));
      capt += " is not represented in the file" ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  // instance for a var-rep of a dim in the file
  DimensionMetaData f_DMD_entry;

  // meta-data of variables' dimensions
  if( getDimMetaData(*pQA->pIn, vMD, f_DMD_entry,
                     pQA->pIn->variable[ix].name, ix) )
    // compare findings from the file with those from the table
    checkMIPT_dimEntry(vMD, f_DMD_entry, t_DMD) ;

  return;
}

void
CMOR::checkMIPT_dimEntry(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // Do the dimensional meta-data found in the netCDF file
  // match those in the standard output table?

  if( t_DMD.attMap[n_output_dim_name] != f_DMD.attMap[n_output_dim_name] )
    checkMIPT_dim_outname(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_standard_name] != f_DMD.attMap[n_standard_name] )
    checkMIPT_dim_stdName(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_long_name] != f_DMD.attMap[n_long_name] )
    checkMIPT_dim_longName(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_axis] != f_DMD.attMap[n_axis] )
    checkMIPT_dim_axis(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_units] != f_DMD.attMap[n_units] )
    checkMIPT_dim_units(vMD, f_DMD, t_DMD);

  // nothing to check
//  if( t_DMD.attMap[n_index_axis] == "ok" )
//    checkMIPT_dim_indexAxis(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_coordinates].size() )
    checkMIPT_dim_coordsAtt(vMD, f_DMD, t_DMD);

  // the second condition takes especially into account
  // declaration of a variable across table sheets
  // (Omon, cf3hr, and cfSites)
  if( t_DMD.attMap[n_bounds_quest] == "yes" )
    checkMIPT_dim_boundsQuest(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_valid_min].size() )
    checkMIPT_dim_validMin(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_valid_max].size() )
    checkMIPT_dim_validMax(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_type] != f_DMD.attMap[n_type] )
    checkMIPT_dim_type(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_positive].size() )
    checkMIPT_dim_positive(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_value].size() )
    checkMIPT_dim_value(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_bounds_values].size() )
    checkMIPT_dim_boundsValues(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_requested].size() )
    checkMIPT_dim_requested(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_bounds_requested].size() )
    checkMIPT_dim_boundsRequested(vMD, f_DMD, t_DMD);

  return ;
}

void
CMOR::checkMIPT_dim_axis(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string key("4_4e");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_axis ));

    if( f_DMD.attMap[n_axis].size() )
    {
      capt += "no match with the CMOR table request, found";
      capt += hdhC::tf_val(f_DMD.attMap[n_axis]);
      capt += ", expected";
    }
    else
     capt += "missing, CMOR table requests";

    capt += hdhC::tf_val(t_DMD.attMap[n_axis]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_boundsQuest(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // standard table requires a boundary variable
  if( f_DMD.var->bounds.size() )
    return ;

  std::string key("4_4i");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "bounds?" ));
    capt += "missing bounds-variable as requested by the CMOR table" ;

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_boundsRequested(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
   checkWithTolerance(f_DMD, t_DMD, n_bounds_requested );
   return;
}

void
CMOR::checkMIPT_dim_boundsValues(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // check dimensional values

  // check values of limited dimensions
  if( f_DMD.var->isUnlimited() )
    return;

  checkWithTolerance(f_DMD, t_DMD, n_bounds_values);

  return ;
}

void
CMOR::checkMIPT_dim_coordsAtt(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::vector<std::string> dimVar;
  dimVar.push_back("region");
  dimVar.push_back("passage");
  dimVar.push_back("type_description");

  std::vector<std::string> coAtt;
  coAtt.push_back("basin");
  coAtt.push_back("line");
  coAtt.push_back(n_type);

  std::string& t_DMD_coAtt   = t_DMD.attMap[n_coordinates] ;

  // note: the loop is mostly passed without action
  for(size_t ix=0 ; ix < coAtt.size() ; ++ix )
  {
    if( t_DMD_coAtt == coAtt[ix] )
    {
      // Is coordinates-att set in the corresponding variable?
      if( pQA->pIn->getVarIndex(dimVar[ix]) == -1 );
      {
        std::string key("4_4h");
        if( notes->inq( key, vMD.var->name) )
        {
          std::string capt("missing coordinates variable, expected");
          capt += hdhC::tf_val(dimVar[ix]);

          (void) notes->operate(capt) ;
          notes->setCheckStatus("CV", pQA->n_fail);
          pQA->setExit( notes->getExitValue() ) ;
        }
      }

      break;
    }
  }

  return;
}

void
CMOR::checkMIPT_dim_indexAxis(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  return;
}

void
CMOR::checkMIPT_dim_longName(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // generic level, thus user defined
  if( ! t_DMD.attMap[n_standard_name].size() )
    return;

  if( f_DMD.attMap.count(n_standard_name) &&
          t_DMD.attMap["proposed " + n_standard_name]
            == f_DMD.attMap[n_standard_name] )
    return;

  std::string key("4_4d");

  std::string& t_DMD_long_name = t_DMD.attMap[n_long_name] ;
  std::string& f_long_name = f_DMD.attMap[n_long_name] ;

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "long name" ));
    if( f_long_name.size() )
    {
      capt += "found";
      capt += hdhC::tf_val(f_long_name);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_DMD_long_name) ;

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

bool
CMOR::checkMIPT_dim_outname(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string key("4_4b");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(
        f_DMD, t_DMD, "output dimension name" ));
    capt += "missing output dimensions name in the file";

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return true;
}

void
CMOR::checkMIPT_dim_positive(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string& t_positive = t_DMD.attMap[n_positive] ;
  std::string& f_positive = f_DMD.attMap[n_positive] ;

  if( t_positive == f_positive )
    return;

  std::string key("4_4m");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_positive));
    capt += hdhC::tf_att(n_positive);
    capt += "in the file does not match the request in the CMOR table";
    if( f_DMD.attMap[n_positive].size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_DMD.attMap[n_positive]);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_positive);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_requested(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
   // There could be layers additionally
   // to the 17 mandatory ones for the dimension 'plevs'.
   // Thus, we have to ignore the optional ones.
   // for the comparisons between standard output table and file

  std::string& odn = t_DMD.attMap[n_output_dim_name] ;

  if( odn == n_type || odn == "passage" || odn == "region" )
    checkStringValues(f_DMD, t_DMD, n_requested);
  else
  {
    size_t i=0;
    if( t_DMD.attMap[n_CMOR_dimension] == "plevs" )
      i=17;

    checkWithTolerance(f_DMD, t_DMD, n_requested, i );
  }

  return;
}

void
CMOR::checkMIPT_dim_stdName(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // dimensions which may have various standard names
  std::string& name = t_DMD.attMap[n_output_dim_name] ;

  if( name == "lev" || name == "loc" || name == "site" )
    return;

  std::string key("4_4c");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "standard name" ));
    capt += "not as the CMOR table requested";
    if( f_DMD.attMap[n_standard_name].size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_DMD.attMap[n_standard_name]);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_DMD.attMap[n_standard_name]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_type(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  if( f_DMD.attMap[n_type] == "char" && t_DMD.attMap[n_type] == "character" )
     return;

  std::string key("4_4l");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_type ));
    capt += "CMOR table requests,";
    capt += hdhC::tf_val(t_DMD.attMap[n_type]);
    capt += ", found";
    capt += hdhC::tf_val(f_DMD.attMap[n_type]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_units(
     VariableMetaData& vMD,
     struct DimensionMetaData& f_DMD,
     struct DimensionMetaData& t_DMD)
{
  // Special: units related to any dimension but 'time'

  // Do the dimensional meta-data found in the netCDF file
  // match those in the table (standard or project)?

  // one or more blanks are accepted
  std::string f_units( hdhC::unique(f_DMD.attMap[n_units], ' ') );
  std::string t_units( hdhC::unique(t_DMD.attMap[n_units], ' ') );

  // all time values should be positive, just test the first one
  if( (pQA->qaTime.firstTimeValue - pQA->qaTime.refTimeOffset) < 0 )
  {
    std::string key("3_9b");
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
      capt += " values should be positive";

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  if( t_units == f_units )
    return;

  // dimensions which may have various units
  std::string& name = t_DMD.attMap[n_output_dim_name] ;

  if( name == "lev" || name == "loc" || name == "site" )
    return;

  if( t_units.find('?') < std::string::npos )
  {
    // time

    // Note: units of 'time' in standard table: 'days since ?',
    //       in the file: days since a specific date.

    if( (f_units.find("days since") < std::string::npos)
          != (t_units.find("days since") < std::string::npos) )
    {
      std::string key("3_9");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
        capt += "ill-formatted units, found";
        capt += hdhC::tf_val(f_units);
        capt += ", expected <days since ...>";

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV", pQA->n_fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }

    return;
  }

  bool hasUnits=false;

  // index axis has no units
  if( t_DMD.attMap[n_index_axis] == "ok" )
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  std::string& t_odn = t_DMD.attMap[n_output_dim_name] ;

  // no units
  if( t_odn == "basin" || t_odn == "line" || t_odn == "type")
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  if( t_odn == "site" )
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  std::string key("3_10");

  if( hasUnits )
  {
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
      capt += "expted dim-less, found " + hdhC::tf_assign(n_units, f_units) ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  // regular case: mismatch
  key = "4_4f" ;

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
    capt += "CMOR table requests";
    capt += hdhC::tf_val(t_units);
    if( f_units.size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_units);
    }

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return ;
}

void
CMOR::checkMIPT_dim_validMin(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  MtrxArr<double> ma;
  pQA->pIn->nc.getData(ma, f_DMD.var->name);

  if( ma.size() )
  {
    double min=ma[0];

    for(size_t i=1 ; i< ma.size() ; ++i )
      if( ma[i] < min )
        min=ma[i];

    double t_val( hdhC::string2Double( t_DMD.attMap[n_valid_min]) );

    if( min < t_val )
    {
      std::string key("4_4j");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_valid_min ));
        capt += "data minimum is lower than requested by the CMOR table";
        capt += ", found";
        capt += hdhC::tf_val(hdhC::double2String(min));
        capt += ", expected";
        capt += hdhC::tf_val(t_DMD.attMap[n_valid_min]);

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV", pQA->n_fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }

  return;
}

void
CMOR::checkMIPT_dim_validMax(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  MtrxArr<double> ma;
  pQA->pIn->nc.getData(ma, f_DMD.var->name);

  if( ma.size() )
  {
    double max=ma[0];

    for(size_t i=1 ; i< ma.size() ; ++i )
      if( ma[i] > max )
        max=ma[i];

    double t_val( hdhC::string2Double( t_DMD.attMap[n_valid_max]) );

    if( max > t_val )
    {
      std::string key("4_4k");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_valid_max ));
        capt += "data maximum is higher than requested by the CMOR table";
        capt += ", found";
        capt += hdhC::tf_val(hdhC::double2String(max));
        capt += ", expected";
        capt += hdhC::tf_val(t_DMD.attMap[n_valid_max]);

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV", pQA->n_fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }

  return;
}

void
CMOR::checkMIPT_dim_value(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // check values of limited dimensions
  if( f_DMD.var->isUnlimited() )
    return;


  std::string& odn = t_DMD.attMap[n_output_dim_name] ;

  if( odn == n_type || odn == "passage" || odn == "region" )
    checkStringValues(f_DMD, t_DMD, n_value );
  else
    checkWithTolerance(f_DMD, t_DMD, n_value);

  return ;
}

void
CMOR::checkMIPT_var(
    VariableMetaData& vMD,
    VariableMetaData& tEntry,
    std::map<std::string, size_t>& v_col)
{
  // Do the variable properties found in the netCDF file
  // match those in the MIP table?
  // Note: priority is not defined in the netCDF file.
  std::string t0;

  if( tEntry.attMap[n_cell_measures] != vMD.attMap[n_cell_measures] )
    checkMIPT_var_cellMeasures(vMD, tEntry);

  if( tEntry.attMap[n_cell_methods] != vMD.attMap[n_cell_methods] )
    checkMIPT_var_cellMethods(vMD, tEntry);

  if( tEntry.attMap[n_flag_meanings].size() )
    checkMIPT_var_flagMeanings(vMD, tEntry);

  if( tEntry.attMap[n_flag_values].size() )
    checkMIPT_var_flagValues(vMD, tEntry);

  if( tEntry.attMap[n_frequency].size() )
    checkMIPT_var_frequency(vMD, tEntry);

  if( tEntry.attMap[n_long_name] != vMD.attMap[n_long_name] )
    checkMIPT_var_longName(vMD, tEntry);

  if( tEntry.attMap[n_positive] != vMD.attMap[n_positive] )
    checkMIPT_var_positive(vMD, tEntry);

  if( tEntry.attMap[n_realm] != vMD.attMap[n_realm] )
    checkMIPT_var_realm(vMD, tEntry);

  if( tEntry.attMap[n_standard_name] != vMD.attMap[n_standard_name] )
    checkMIPT_var_stdName(vMD, tEntry);

  checkMIPT_var_type(vMD, tEntry);

  if( tEntry.attMap[n_unformatted_units] != vMD.attMap[n_units] )
    checkMIPT_var_unformattedUnits(vMD, tEntry);

  return ;
}

void
CMOR::checkMIPT_var_cellMeasures(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  // the format was checked in CF

  // get cm items
  std::vector<std::string> vs_cm_te(
      hdhC::itemise(tEntry.attMap[n_cell_measures], ":", "last") ) ;
  std::vector<std::string> vs_cm_ve(
      hdhC::itemise(vMD.attMap[n_cell_measures], ":", "last") ) ;

  bool is=true;
  size_t tSz = vs_cm_te.size();
  size_t vSz = vs_cm_ve.size();

  if( vSz == tSz )
  {
    is=false;

    for( size_t i=0 ; i < vSz ; ++ i )
    {
      size_t j;
      for( j=0 ; j < tSz ; ++ j )
        if( vs_cm_ve[i] == vs_cm_te[j] )
          break;

      if( j == tSz )
      {
        is=true;
        break;
      }
    }
  }

  if(is)
  {
    std::string key("3_3");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string currTable(QA::tableSheet) ;

      std::string capt(QA_Exp::getCaptionIntroVar(
              currTable, vMD, n_cell_measures));

      if( vMD.attMap[n_cell_measures].size() )
      {
        capt += ", found" ;
        capt += hdhC::tf_val(vMD.attMap[n_cell_measures]) ;
      }
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      capt += ", expected";
      if( tEntry.attMap[n_cell_measures].size() )
        capt += hdhC::tf_val(tEntry.attMap[n_cell_measures]) ;
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return ;
}

void
CMOR::checkMIPT_var_cellMethods(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  // the format was checked in CF

  // get cm items
  std::vector<std::string> vs_cm_te(
      hdhC::itemise(tEntry.attMap[n_cell_methods], ":", "last") ) ;
  std::vector<std::string> vs_cm_ve(
      hdhC::itemise(vMD.attMap[n_cell_methods], ":", "last") ) ;

  bool is=true;
  size_t tSz = vs_cm_te.size();
  size_t vSz = vs_cm_ve.size();

  if( vSz == tSz )
  {
    is=false;

    for( size_t i=0 ; i < vSz ; ++ i )
    {
      size_t j;
      for( j=0 ; j < tSz ; ++ j )
        if( vs_cm_ve[i] == vs_cm_te[j] )
          break;

      if( j == tSz )
      {
        is=true;
        break;
      }
    }
  }

  if(is)
  {
    std::string key("3_3");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string currTable(QA::tableSheet) ;

      std::string capt(QA_Exp::getCaptionIntroVar(
              currTable, vMD, n_cell_methods));

      if( vMD.attMap[n_cell_methods].size() )
      {
        capt += ", found" ;
        capt += hdhC::tf_val(vMD.attMap[n_cell_methods]) ;
      }
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      capt += ", expected";
      if( tEntry.attMap[n_cell_methods].size() )
        capt += hdhC::tf_val(tEntry.attMap[n_cell_methods]) ;
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return ;
}

void
CMOR::checkMIPT_var_flagMeanings(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_flagValues(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_frequency(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_longName(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  // special consideration for tracers in table sheet Omon
  if( QA::tableSheet == "Oyr" && tableSheetBuf[0] == "Omon" )
  {
    // the name in Omon is taken from Oyr + 'at surface'
    std::string t(tEntry.attMap[n_long_name]);
    t += " at surface";
    if( t == vMD.attMap[n_long_name] )
      return;
    else
    {
        t = tEntry.attMap[n_long_name];
        t += " at Surface";
        if( t == vMD.attMap[n_long_name] )
          // this for the correct spelling
          return;
    }
  }

  std::string key("4_5");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string currTable(QA::tableSheet) ;

    std::string capt(QA_Exp::getCaptionIntroVar(currTable, vMD, n_long_name));
    capt += "expected" ;

    if( tEntry.attMap[n_long_name + "_orig"].size() )
      capt += hdhC::tf_val(tEntry.attMap[n_long_name + "_orig"]) ;
    else
      capt += hdhC::tf_val(tEntry.attMap[n_long_name]) ;

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return ;
}

bool
CMOR::checkMIPT_var_positive(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return false;
}

void
CMOR::checkMIPT_var_realm(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_stdName(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_type(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  // the standard table has type==real. Is it for
  // float only, or also for double? So, in case of real,
  // any non-int type is accepted
  bool isTblTypeReal =
      tEntry.attMap[n_type] == "real"
         || tEntry.attMap[n_type] == "float"
             || tEntry.attMap[n_type] == "double" ;
  bool isNcTypeReal =
      vMD.attMap[n_type] == "real"
         || vMD.attMap[n_type] == "float"
              || vMD.attMap[n_type] == "double" ;

  if( tEntry.attMap[n_type].size() == 0 && vMD.attMap[n_type].size() != 0 )
  {
    std::string key("4_11");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string currTable(QA::tableSheet) ;

      std::string capt(QA_Exp::getCaptionIntroVar(currTable, vMD, n_type));
      capt += "check discarded, not found in table " ;
      capt += hdhC::tf_assign(QA::tableSheet);

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }
  else if( (isTblTypeReal && ! isNcTypeReal)
            || ( ! isTblTypeReal && isNcTypeReal) )
  {
    std::string key("4_8");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string currTable(QA::tableSheet) ;

      std::string capt(QA_Exp::getCaptionIntroVar(currTable, vMD, n_type));
      capt += ", expected";

      if( tEntry.attMap[n_type].size() )
        capt += hdhC::tf_assign("type", tEntry.attMap[n_type]) ;
      else
        capt += " no type";

      capt += ", found" ;
      if( vMD.attMap[n_type].size() )
        capt += hdhC::tf_val(vMD.attMap[n_type]) ;
      else
        capt += "no type" ;


      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return ;
}

void
CMOR::checkMIPT_var_unformattedUnits(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkRequestedAttributes(void)
{
  // check requested attributes of specific variables
  for(size_t i=0 ; i < pQA->drs_cv_table.varName.size() ; ++i)
  {
    // global attributes
    if( pQA->drs_cv_table.varName[i] == pQA->s_global
            && pQA->pIn->variable[pQA->pIn->varSz].name == "NC_GLOBAL" )
      checkReqAtt_global();
    else
    {
      for( size_t j=0 ; j < pQA->pIn->varSz ; ++j )
      {
        // check for requested attributes of variables (whether coordinates var
        // or data var doesn't matter).
        if( pQA->drs_cv_table.varName[i] == pQA->pIn->variable[j].name )
        {
          checkReqAtt_variable(pQA->pIn->variable[j]);
          break;
        }
      }
    }
  }

  checkSource();

/*
  // check attribute _FillValue and missing_value, if available
  std::string fV("_FillValue");
  std::string mV("missing_value");

  std::vector<std::string> fVmv;
  fVmv.push_back(fV);
  fVmv.push_back(mV);

  for( size_t j=0 ; j < pQA->pIn->varSz ; ++j )
  {
    Variable& var = pQA->pIn->variable[j] ;

    for( size_t l=0 ; l < 2 ; ++l )
    {
      int k;
      if( (k=var.getAttIndex(fVmv[l])) > -1 )
      {
        std::vector<float> aV;
        pQA->pIn->nc.getAttValues(aV, fVmv[l], var.name ) ;
        float rV = 1.E20 ;

        if( aV.size() == 0 || aV[0] != rV )
        {
          std::string key("3_11");

          if( notes->inq( key, var.name) )
          {
            std::string capt(
              hdhC::tf_att(var.name, fVmv[l], var.attValue[j][0])) ;
            capt += "does not match requested value=1.E20";

            (void) notes->operate(capt) ;
            notes->setCheckStatus("CV",  pQA->n_fail );
          }
        }
      }
    }
  }

*/

  return ;
}

void
CMOR::checkReqAtt_global(void)
{
  // check requested attributes
  DRS_CV_Table& drs = pQA->drs_cv_table;

  Variable& glob = pQA->pIn->variable[pQA->pIn->varSz] ;

  size_t ix;
  for( ix=0 ; ix < drs.varName.size() ; ++ix )
    if( drs.varName[ix] == pQA->s_global )
      break;

  if( ix == drs.varName.size() )
    return;

  for( size_t k=0 ; k < drs.attName[ix].size(); ++k )
  {
    std::string& rqName  = drs.attName[ix][k] ;
    std::string& rqValue = drs.attValue[ix][k] ;

    // missing requested global attribute
    if( ! glob.isValidAtt(rqName) )
    {
       std::string key("2_3");

       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("requested ");
         capt += pQA->s_global;
         capt += hdhC::blank;
         capt += hdhC::tf_att(rqName);
         capt += "is missing" ;

         (void) notes->operate(capt) ;
         notes->setCheckStatus("CV",  pQA->n_fail );

         continue;
       }
    }

    // requested attribute expects a requested value
    if( rqValue.size() )  // a value is requested
    {
      std::string aV = glob.getAttValue(rqName) ;

      if( aV.size() == 0 )
      {
        std::string key("2_2");
        if( notes->inq( key, pQA->s_global) )
        {
           std::string capt(pQA->s_global);
           capt += hdhC::blank;
           capt += hdhC::tf_att(rqName, hdhC::no_blank);
           capt=" missing requested value=";
           capt += rqValue;

           if( rqName == n_forcing )
           {
             capt += ", expected at least";
             capt += hdhC::tf_assign(n_forcing, "N/A") ;
           }

           (void) notes->operate(capt) ;
           notes->setCheckStatus("CV",  pQA->n_fail );

           continue;
         }
       }
       else
       {
          // note that several alternatives may be acceptable
          Split x_rqValue(rqValue, "|");
          std::vector<std::string> vs_rqValue(x_rqValue.getItems());
          vs_rqValue = hdhC::unique(vs_rqValue, hdhC::blank);

          if( rqName == n_forcing )
            checkForcing(vs_rqValue, aV);
          else if( rqName == "initialization_method"
                      || rqName == "physics_version_method"
                         || rqName == "realization" )
            checkEnsembleMemItem(rqName, aV) ;

          else if( x_rqValue[0] == "YYYY-MM-DDTHH:MM:SSZ" )
          {
            // a valid date format may have T substituted by a blank
            // and Z omitted or the latter be lower case.
            bool is=false;
            if( aV.size() > 18 )
            {
              if( aV[4] == '-' && aV[7] == '-' )
              {
                  if( aV[10] == 'T' || aV[10] == ' ' )
                  {
                    if( aV[13] == ':' && aV[16] == ':' )
                    {
                      if( aV.size() == 20 )
                      {
                        if( !(aV[19] == 'Z' || aV[19] == 'z' ) )
                          is=true;
                      }

                      if( ! hdhC::isDigit( aV.substr(0, 4) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(5, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(8, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(11, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(14, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(17, 2) ) )
                        is=true ;
                    }
                    else
                      is=true;
                  }
                  else
                    is=true;
              }
              else
                is=true;
            }
            else
              is=true;

            if( is )
            {
              std::string key("2_4");
              if( notes->inq( key, pQA->s_global) )
              {
                std::string capt(pQA->s_global);
                capt += hdhC::blank;
                capt += hdhC::tf_att(rqName);
                capt += "does not comply with YYYY-MM-DDThh:mm:ssZ, found: " ;
                capt += aV ;

                (void) notes->operate(capt) ;
                notes->setCheckStatus("CV",  pQA->n_fail );
              }
            }
          }

          else if( !hdhC::isAmong(aV, vs_rqValue) )
          {
            // there is a value. is it the requested one?
            std::string key("2_8");
            if( notes->inq( key, pQA->s_global ) )
            {
              std::string capt(pQA->s_global);
              capt += hdhC::blank;
              capt += hdhC::tf_att(hdhC::empty, rqName, aV);
              capt += "does not match requested value";
              capt += hdhC::tf_val(rqValue) ;

              (void) notes->operate(capt) ;
              notes->setCheckStatus("CV",  pQA->n_fail );

              continue;
            }
          }
       }
    }
  } // end of for-loop

  return;
}

void
CMOR::checkReqAtt_variable(Variable &var)
{
  // vName := name of an existing variable (already tested)
  // ix    := index of InnFile.variable[ix]

/*
  DRS_CV_Table& drs = pQA->drs_cv_table;

  std::string &vName = var.name;

  // check required attributes
  size_t ix;
  for( ix=0 ; ix < drs.varName.size() ; ++ix )
    if( drs.varName[ix] == vName )
      break;

  // find required attributes missing in the file
  for( size_t k=0 ; k < drs.attName[ix].size(); ++k )
  {
    std::string& rqName  = drs.attName[ix][k] ;
    std::string& rqValue = drs.attValue[ix][k] ;

    // find corresponding att_name of given vName in the file
    int jx=var.getAttIndex(rqName);

    // special: values may be available for cloud amounts

    // missing required attribute
    if( jx == -1 && rqName != "values" )
    {
       std::string key("2_1");

       if( notes->inq( key, vName) )
       {
         std::string capt(hdhC::tf_att(vName,rqName));
         capt += "is missing";

         (void) notes->operate(capt) ;
         notes->setCheckStatus("CV",  pQA->n_fail );

         continue;
       }
    }

    std::string &aN = var.attName[jx] ;

    // required attribute expects a value (not for cloud amounts)
    if( rqValue.size() )  // a value is required
    {
      if( ! isCloudAmount && rqName == "values" )
         continue;

      std::string aV( var.attValue[jx][0] ) ;
      for( size_t i=1 ; i < var.attValue[jx].size() ; ++i )
      {
        aV += " " ;
        aV += var.attValue[jx][i] ;
      }

      if( aV.size() == 0 )
      {
        std::string key("2_2");
        if( notes->inq( key, vName) )
        {
           std::string capt(hdhC::tf_att(vName, aN, hdhC::colon));
           capt="missing required value=" ;
           capt += rqValue;

           (void) notes->operate(capt) ;
           notes->setCheckStatus("CV",  pQA->n_fail );

           continue;
         }
       }

       // there is a value. is it the required one?
       else if( aV != rqValue )
       {
         bool is=true;
         if( rqName == pQA->n_long_name)
         {
           // mismatch tolerated, because the table does not agree with CMIP5
           if( rqValue == "pressure" && aV == "pressure level" )
             is=false;
           if( rqValue == "pressure level" && aV == "pressure" )
             is=false;
         }
         else if( (rqName == pQA->n_positive) || (rqName == pQA->n_axis) )
         {
           // case insensitive
           if( rqValue == hdhC::Lower()(aV) )
             is=false;
         }

         std::string key("2_3");
         if( is &&  notes->inq( key, vName ) )
         {
           std::string capt(hdhC::tf_att(vName, aN, aV));
           capt += "does not match required value=" ;
           capt += rqValue;

           (void) notes->operate(capt) ;
           notes->setCheckStatus("CV",  pQA->n_fail );

           continue;
         }
       }
    }
  } // end of for-loop
*/

  return;
}

void
CMOR::checkSource(void)
{
  // global attribute 'source'
  Variable& glob = pQA->pIn->variable[pQA->pIn->varSz] ;

  std::string n_source("source");

  Split x_colon(glob.getAttValue(n_source),";");

  // note: existence is checked elsewhere
  if( x_colon.size() == 0 )
    return;

  // some words
  std::string n_model_id("model_id");

  std::vector<std::string> vs_dscr;
  vs_dscr.push_back("atmosphere:");
  vs_dscr.push_back("ocean:");
  vs_dscr.push_back("sea ice:");
  vs_dscr.push_back("land:");

  Split x_word(x_colon[0]);
  x_word.setProtector("()",true);

  Split x_brackets;
  x_brackets.setSeparator("()", true);

  Split x_brackets_items;

  for( size_t i=0; i < x_colon.size() ; ++i )
  {
    size_t j=0;
    x_word = x_colon[i] ;

    if( i==0 )
    {
      std::string model_id(glob.getAttValue(n_model_id)) ;

      if( x_word[j].size() == 0 )
      {
        std::string key("2_7a");
        if( notes->inq(key) )
        {
          std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
          capt += "The 1st item should be the model_id attribute";

          (void) notes->operate(capt) ;
          notes->setCheckStatus("CV", pQA->n_fail);
        }
      }

      else
      {
        if( model_id.size() != x_word[j].size() )
        {
          std::string key("2_7b");
          if( notes->inq(key) )
          {
            std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
            capt += "The model_id does not match, found";
            capt += hdhC::tf_val(x_word[j]) ;
            capt += ", expected";
            capt += hdhC::tf_val(model_id) ;

            (void) notes->operate(capt) ;
            notes->setCheckStatus("CV", pQA->n_fail);
          }

          ++j;
        }

        if( x_word[j].size() > 1 )
        {
          if( ! hdhC::isDigit(x_word[1]) )
          {
            std::string key("2_7c");
            if( notes->inq(key) )
            {
              std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
              capt += "The 2nd item should be a year in digits";

              (void) notes->operate(capt) ;
              notes->setCheckStatus("CV", pQA->n_fail);
            }
          }

          ++j;
        }
      }
    }

    // look for descriptors
    size_t dscr_ix;
    for( dscr_ix=0 ; dscr_ix < vs_dscr.size() ; ++dscr_ix )
      if( vs_dscr[dscr_ix] == x_word[j] )
        break;

    if( dscr_ix < vs_dscr.size() )
    {
      // model_name is required before ()-term
      if( x_word[j][0] == '(' )
      {
        std::string key("2_7d");
        if( notes->inq(key) )
        {
          std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
          capt += "The descriptor";
          capt += hdhC::tf_val(vs_dscr[dscr_ix]) ;
          capt += " should be followed by a model_name";

          (void) notes->operate(capt) ;
          notes->setCheckStatus("CV", pQA->n_fail);
        }
      }
      else
        ++j;
    }

    // the term in brackets
    bool foundBrackets=false;

    for( ; j < x_word.size() ; ++j )
    {
//      size_t last = x_word[j].size() - 1;

      x_brackets = x_colon[i];

//      if( x_word[j][0] == '(' && x_word[j][last] == ')' )
      if( x_brackets.size() > 1 )
      {
        foundBrackets=true;
//        x_brackets_items = x_word[j].substr(1,last-1);
        x_brackets_items = x_brackets[1];

        bool isA = (x_brackets_items.size() == 1 && dscr_ix != 2 )
                    ? true : false ;   // sea ice: or land:

        if( !isA )
          isA = x_brackets_items.size() == 2 ? false : true ;

        if(isA)
        {
          std::string key("2_7f");
          if( notes->inq(key) )
          {
            std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
            capt += "faulty term (<technical_name>, <resolution_and_levels>), found";
            capt += hdhC::tf_val(x_brackets_items.getStr()) ;

            (void) notes->operate(capt) ;
            notes->setCheckStatus("CV", pQA->n_fail);
          }

          break;
        }
      }
    }

    if( dscr_ix < vs_dscr.size() && !foundBrackets )
    {
      std::string key("2_7e");
      if( notes->inq(key) )
      {
        std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
        capt += "Missing bracketed term in ";
        capt += x_colon[i];

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV", pQA->n_fail);
      }
    }
  }

  return;
}

void
CMOR::checkStringValues( struct DimensionMetaData& f_DMD,
  struct DimensionMetaData& t_DMD,
  std::string& cName, size_t maxSz )
{
  Split x_tVal( t_DMD.attMap[cName]) ;

  std::vector<std::string> vs_values;

  pQA->pIn->nc.getData(vs_values, f_DMD.var->name);

  if( vs_values.size() != x_tVal.size() )
  {
    std::string key("4_4");

    if( cName == n_value )
      key += 'n';
    else if( cName == n_bounds_values )
      key += 'o';
    else if( cName == n_requested )
      key += 'p';
    else if( cName == n_bounds_requested )
      key += 'q';

    if( notes->inq( key, f_DMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
      capt += "mismatch of number of data values, found";
      capt += hdhC::tf_val(hdhC::double2String(vs_values.size()));
      capt += " items in the file and ";
      capt += hdhC::tf_assign(cName, hdhC::double2String(x_tVal.size()));
      capt += " in the table";

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  size_t i;
  for( i=0 ; i < vs_values.size() ; ++i )
    vs_values[i] = hdhC::stripSides(vs_values[i]) ;

  for( i=0 ; i < x_tVal.size() ; ++i )
    if( !hdhC::isAmong(x_tVal[i], vs_values) )
      break;

  if( i == x_tVal.size() )
    return;

  std::string key("4_4");

  if( cName == n_value )
    key += 'n';
  else if( cName == n_bounds_values )
    key += 'o';
  else if( cName == n_requested )
    key += 'p';
  else if( cName == n_bounds_requested )
    key += 'q';

  if( notes->inq( key, f_DMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
    capt += "mismatch of data values between file and table, expected also";
    capt += hdhC::tf_assign(cName, x_tVal[i]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkWithTolerance( struct DimensionMetaData& f_DMD,
  struct DimensionMetaData& t_DMD,
  std::string& cName, size_t maxSz )
{
  Split x_tVal( t_DMD.attMap[cName]) ;

  MtrxArr<double> ma;
  pQA->pIn->nc.getData(ma, f_DMD.var->name);

  if( ma.size() != x_tVal.size() )
  {
    std::string key("4_4");

    if( cName == n_value )
      key += 'n';
    else if( cName == n_bounds_values )
      key += 'o';
    else if( cName == n_requested )
      key += 'p';
    else if( cName == n_bounds_requested )
      key += 'q';

    if( notes->inq( key, f_DMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
      capt += "mismatch of number of data values, found";
      capt += hdhC::tf_val(hdhC::double2String(ma.size()));
      capt += " items in the file and ";
      capt += hdhC::tf_assign(cName, hdhC::double2String(x_tVal.size()));
      capt += " in the table";

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  double tolVal=0;
  if( t_DMD.attMap[n_tol_on_requests].size());
    tolVal = hdhC::string2Double(t_DMD.attMap[n_tol_on_requests]);

  if( !maxSz )
    maxSz = x_tVal.size();

  std::vector<double> tolMin;
  std::vector<double> tolMax;

  for( size_t i=0 ; i < maxSz ; ++i )
  {
    tolMin.push_back(ma[i] - tolVal);
    tolMax.push_back(ma[i] + tolVal);
  }

  // convert table values to double, which is always possible for CMIP5
  size_t i;
  for( i=0 ; i < maxSz ; ++i )
  {
    double tVal = hdhC::string2Double(x_tVal[i]) ;
    if( !( (tolMin[i] <= tVal) && (tVal <= tolMax[i]) ) )
      break;
  }

  if( i == maxSz )
    return;

  std::string key("4_4");

  if( cName == n_value )
    key += 'n';
  else if( cName == n_bounds_values )
    key += 'o';
  else if( cName == n_requested )
    key += 'p';
  else if( cName == n_bounds_requested )
    key += 'q';

  if( notes->inq( key, f_DMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
    capt += "Mismatch of data values between file and table, found";
    capt += hdhC::tf_val(hdhC::double2String(ma[i]));
    capt += ", expected ";
    capt += hdhC::tf_assign(cName, x_tVal[i]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

bool
CMOR::findVarReqTableEntry(ReadLine& ifs, std::string& str0,
   VariableMetaData& vMD,
   std::map<std::string, size_t>& col, VariableMetaData& tEntry)
{
   // return true: entry is not the one we look for.
   Split x_line;
   x_line.setSeparator(',');
   x_line.enableEmptyItems();

   size_t colIx=col[CMOR::n_CMOR_variable_name];

   while( ! ifs.getLine(str0) )
   {
     // no entry
     if( str0.substr(0,10) == "CMOR Table" )
       return false;

     // Get the string for the MIP-sub-table corresponding to
     // time table entries read by parseTimeTable().
     if( str0.substr(0,13) == "In CMOR Table" )
     {
       getVarReqTableSubSheetName(str0, vMD) ;
       continue;
     }

     x_line=str0;

     if( tableSheetBufIx == 1 && ! tEntry.attMap.empty() )
     {
       if( tableSheetBuf[tableSheetBufIx] == "Omon" )
       {
          if( x_line[1].find("1st table in Oyr") < std::string::npos )
            return true; // found an entry and the shape matches.
       }
/*
       else if( tableSheetBuf[0] == "cf3hr" )
         isFindSubSheet=false;
       else if( tableSheetBuf[0] == "cfSites" )
         isFindSubSheet=false;
*/

     }

     if( x_line.size() > colIx && x_line[colIx] == vMD.var->name )
       return true; // found an entry and the shape matches.
   }

   return false;
}

bool
CMOR::findVarReqTableSheet(ReadLine& ifs, std::string& str0,
  VariableMetaData& vMD, VariableMetaData& tEntry)
{
  // return true, if no entry was found
  size_t pos;
  Split x_col;

  // very special: a tracer variable was not found
  // in table sheet Omon, because it is defined in Oyr, but
  // some properties from the Omon
  if( !tableSheetBuf.size() )
    bufTableSheets(vMD);

  do
  {
    if( str0.substr(0,10) == "CMOR Table" )
    {
      if( (pos=str0.find(',')) < std::string::npos )
      {
        x_col = str0.substr(0,pos) ;

        // The name of the table is the 3rd blank-separated item;
        // it may have an appended ':'
        if( x_col.size() > 2 )
          if( (pos=x_col[2].rfind(hdhC::colon)) < std::string::npos )
            str0=x_col[2].substr(0, pos);

        if( str0 == tableSheetBuf[tableSheetBufIx] )
          return true;  // sub table found
      }
    }
  }
  while( ! ifs.getLine(str0) );

  return false;  // not found; try next
}

void
CMOR::getVarReqTableSubSheetName(std::string& str0, VariableMetaData& vMD)
{
   // get the string for the MIP-sub-table, i.e. table sub-sheet name.
   // This is used in conjunction to the time table (parseTimeTable()).
   size_t p;
   if( (p = str0.find(hdhC::colon)) < std::string::npos )
     // skip also a leading ' ' after the colon
     QA::tableSheetSub = str0.substr(p+2) ;

   return;
}

bool
CMOR::getDimMetaData(InFile& in,
      VariableMetaData& vMD,
      struct DimensionMetaData& f_DMD,
      std::string dName, size_t ix)
{
  // return 0:
  // collect dimensional meta-data into a struct.

  // the corresponding variable
  f_DMD.var = &pQA->pIn->variable[ix] ;

  // pre-set
  f_DMD.attMap[n_output_dim_name]=dName;
  f_DMD.attMap[n_coordinates]=dName;

  // get the var type
  f_DMD.attMap[n_type] = in.nc.getVarTypeStr(dName);

  // get the attributes

  for( size_t j=0 ; j < f_DMD.var->attName.size() ; ++j)
  {
    std::string& aName  = f_DMD.var->attName[j];
    std::string& aValue = f_DMD.var->attValue[j][0];

    if( aName == n_axis )
      f_DMD.attMap[n_axis] = aValue ;
    else if( aName == n_long_name )
    {
      f_DMD.attMap[n_long_name] = hdhC::unique(aValue, ' ') ;
      if( notes->inq( "7_14", vMD.var->name, "INQ_ONLY") )
        f_DMD.attMap[n_long_name] = hdhC::Lower()(f_DMD.attMap[n_long_name]) ;
    }
    else if( aName == n_standard_name )
      f_DMD.attMap[n_standard_name] = aValue ;
    else if( aName == n_units )
      f_DMD.attMap[n_units] = aValue ;
    else if( aName == n_positive )
      f_DMD.attMap[n_positive] = aValue ;
  }

  // determine the checksum of limited var-presentations of dim
  if( ! f_DMD.var->checksum && ! f_DMD.var->isUnlimited() )
  {
    if( f_DMD.var->type == NC_CHAR )
    {
      std::vector<std::string> vs;
      in.nc.getData(vs, dName);

      bool reset=true;  // is set to false during the first call
      for(size_t i=0 ; i < vs.size() ; ++i)
      {
        vs[i] = hdhC::stripSides(vs[i]);
        f_DMD.var->checksum = hdhC::fletcher32_cmip5(vs[i], &reset) ;
      }
    }
    else
    {
      MtrxArr<double> mv;
      in.nc.getData(mv, dName);

      bool reset=true;
      for( size_t i=0 ; i < mv.size() ; ++i )
        f_DMD.var->checksum = hdhC::fletcher32_cmip5(mv[i], &reset) ;
    }
  }

  return dName.size() ? true : false;
}

void
CMOR::getDimSheetEntry(std::string& CMOR_dim,
   std::vector<std::string>& vs_dimSheet,
   std::map<std::string, size_t>& col,
   DimensionMetaData& t_DMD, std::string column)
{
   Split x_line;
   x_line.setSeparator(',');
   x_line.enableEmptyItems();

   // look for a matching entry in the dimension sheet
   for( size_t i=0 ; i < vs_dimSheet.size() ; ++i )
   {
     x_line=vs_dimSheet[i];

     // is it in the table?
     if( x_line[col[n_CMOR_dimension]] == CMOR_dim )
     {
       if( column.size() )
       {
          t_DMD.attMap[column]=x_line[col[column]];
          return ;
       }

       t_DMD.attMap[n_CMOR_dimension]=x_line[col[n_CMOR_dimension]];

       // is there a mapping to a name in column 'coords_attr'?
       if( x_line[col[n_coordinates]].size() )
         t_DMD.attMap[n_output_dim_name]  = x_line[col[n_coordinates]];
       else
         t_DMD.attMap[n_output_dim_name]  = x_line[col[n_output_dim_name]];

       t_DMD.attMap[n_standard_name] =x_line[col[n_standard_name]];

       t_DMD.attMap[n_long_name]
          = hdhC::unique(x_line[col[n_long_name]], ' ');
       if( notes->inq( "7_14", "", "INQ_ONLY") )
         t_DMD.attMap[n_long_name] = hdhC::Lower()(t_DMD.attMap[n_long_name]) ;

       t_DMD.attMap[n_axis]             = x_line[col[n_axis]];
       t_DMD.attMap[n_units]            = hdhC::unique(x_line[col[n_units]], ' ');
       t_DMD.attMap[n_index_axis]       = x_line[col[n_index_axis]];
       t_DMD.attMap[n_coordinates]      = x_line[col[n_coordinates]];
       t_DMD.attMap[n_bounds_quest]     = x_line[col[n_bounds_quest]];
       t_DMD.attMap[n_valid_min]        = x_line[col[n_valid_min]];
       t_DMD.attMap[n_valid_max]        = x_line[col[n_valid_max]];
       t_DMD.attMap[n_type]             = x_line[col[n_type]];
       t_DMD.attMap[n_positive]         = x_line[col[n_positive]];
       t_DMD.attMap[n_value]            = x_line[col[n_value]];
       t_DMD.attMap[n_bounds_values]    = x_line[col[n_bounds_values]];
       t_DMD.attMap[n_requested]        = x_line[col[n_requested]];
       t_DMD.attMap[n_bounds_requested] = x_line[col[n_bounds_requested]];

       t_DMD.attMap[n_tol_on_requests]  = hdhC::empty;
       std::string& str0 = x_line[col[n_tol_on_requests]] ;
       if( str0.substr(0, n_tol_on_requests.size()) == n_tol_on_requests )
          t_DMD.attMap[n_tol_on_requests]=x_line[col[n_tol_on_requests]];
     }
   }

  return;
}

void
CMOR::getMIPT_var(std::string& entry,
    VariableMetaData& tEntry,
    std::map<std::string, size_t>& v_col)
{
  Split x_line;
  x_line.setSeparator(',');
  x_line.enableEmptyItems();
  x_line = entry;

  // This was tested in findVarReqTableSheet()
  tEntry.var->name = x_line[v_col[n_CMOR_variable_name]];
  tEntry.attMap[n_output_var_name] = tEntry.var->name;

//     tEntry.attMap[n_priority] = x_line[v_col[n_priority]];
  tEntry.attMap[n_long_name]
    = hdhC::unique( x_line[v_col[n_long_name]], ' ' );
  if( notes->inq( "7_14", "", "INQ_ONLY") )
    tEntry.attMap[n_long_name]
      = hdhC::Lower()(tEntry.attMap[n_long_name]) ;

  tEntry.attMap[n_standard_name] = x_line[v_col[n_standard_name]];
  tEntry.attMap[n_units]
    = hdhC::unique( x_line[v_col[n_unformatted_units]], ' ' );
  if( tEntry.var->units.size() )
    tEntry.var->isUnitsDefined=true;
  else
    tEntry.var->isUnitsDefined=false;

  tEntry.attMap[n_cell_methods]   = x_line[v_col[n_cell_methods]];
  tEntry.attMap[n_positive]       = x_line[v_col[n_positive]];
  tEntry.attMap[n_type]           = x_line[v_col[n_type]];
  tEntry.attMap[n_CMOR_dimension] = x_line[v_col[n_CMOR_dimension]];

// no way to check CMOR variable name

  tEntry.attMap[n_realm]          = x_line[v_col[n_realm]];
  tEntry.attMap[n_frequency]      = x_line[v_col[n_frequency]];
  tEntry.attMap[n_cell_measures]  = x_line[v_col[n_cell_measures]];
  tEntry.attMap[n_flag_meanings]  = x_line[v_col[n_flag_meanings]];
  tEntry.attMap[n_flag_values]    = x_line[v_col[n_flag_values]];

  return ;
}

void
CMOR::initDefaults(void)
{
  tableSheetBufIx=0;

  return;
}

bool
CMOR::readHeadline(ReadLine& ifs,
   VariableMetaData& vMD, std::string& fx_header,
   std::vector<std::string>& vs_dimSheet,
   std::map<std::string, size_t>& v_col,
   std::map<std::string, size_t>& d_col)
{
   // find the captions for table sheets dims and for the variables

   std::string str0;

   Split x_line;
   x_line.setSeparator(',');
   x_line.enableEmptyItems();

   // This should give the heading of the table sheet for the dimensions.
   // The table sheet is identified by the first column.
   while( !ifs.getLine(str0) )
   {
     if( str0.substr(0,13) == "CMOR table(s)" )
       break ;
   }

   x_line=str0;

   // identify columns of the Taylor table; look for the indexes
   for( size_t d=0 ; d < x_line.size() ; ++d)
   {
      if( d == 0 )
        d_col[n_CMOR_tables] = d;
      else if( x_line[d] == "CMOR dimension" )
        d_col[n_CMOR_dimension] = d;
      else if( x_line[d] == "output dimension name" )
        d_col[n_output_dim_name] = d;
      else if( x_line[d] == "standard name" )
        d_col[n_standard_name] = d;
      else if( x_line[d] == "long name" )
        d_col[n_long_name] = d;
      else if( x_line[d] == n_axis )
        d_col[n_axis] = d;
      else if( x_line[d] == "index axis?" )
        d_col[n_index_axis] = d;
      else if( x_line[d] == n_units )
        d_col[n_units] = d;
      else if( x_line[d] == "coords_attrib" )
        d_col[n_coordinates] = d;
      else if( x_line[d] == "bounds?" )
        d_col[n_bounds_quest] = d;
      else if( x_line[d] == n_valid_min )
        d_col[n_valid_min] = d;
      else if( x_line[d] == n_valid_max )
        d_col[n_valid_max] = d;
      else if( x_line[d] == n_type )
        d_col[n_type] = d;
      else if( x_line[d] == n_positive )
        d_col[n_positive] = d;
      else if( x_line[d] == n_value )
        d_col[n_value] = d;
      else if( x_line[d] == n_requested )
        d_col[n_requested] = d;
      else if( hdhC::clearSpaces(x_line[d]) == n_bounds_values )
        d_col[n_bounds_values] = d;
      else if( hdhC::clearSpaces(x_line[d]) == n_bounds_requested )
        d_col[n_bounds_requested] = d;
   }

   // find the capt for variables
   while( ! ifs.getLine(str0) )
   {
     vs_dimSheet.push_back( str0 );

     if( str0.substr(0,10) == "CMOR Table" )
       fx_header = str0;

     if( str0.substr(0,8) == n_priority )
       break;
   } ;

   x_line=str0;

   // now, the variable's columns
   for( size_t v=0 ; v < x_line.size() ; ++v)
   {
     if( x_line[v] == "CMOR variable name" )
       v_col[n_CMOR_variable_name] = v;
     if( x_line[v] == "output variable name" )
       v_col[n_output_var_name] = v;
     else if( x_line[v] == n_priority )
       v_col[n_priority] = v;
     else if( x_line[v] == "standard name" )
       v_col[n_standard_name] = v;
     else if( x_line[v] == "unconfirmed or proposed standard name" )
       v_col["proposed " + n_standard_name] = v;
     else if( x_line[v] == "long name" )
       v_col[n_long_name] = v;
     else if( x_line[v] == "unformatted units" )
       v_col[n_unformatted_units] = v;
     else if( x_line[v] == n_cell_methods )
       v_col[n_cell_methods] = v;
     else if( x_line[v] == n_cell_measures )
       v_col[n_cell_measures] = v;
     else if( x_line[v] == n_type )
       v_col[n_type] = v;
     else if( x_line[v] == n_positive )
       v_col[n_type] = v;
     else if( x_line[v] == "CMOR dimensions" )
       v_col[n_CMOR_dimension] = v;
     else if( x_line[v] == "valid min" )
       v_col[n_valid_min] = v;
      else if( x_line[v] == "valid max" )
       v_col[n_valid_max] = v;
   }

   return true;
}

void
CMOR::run(InFile& in, VariableMetaData& vMD)
{
  // Matching the ncfile inherent meta-data against pre-defined
  // CMOR tables.

  // in fact only global attributes
  checkRequestedAttributes();

  // Meta data of variables from file or table are stored in struct varMeDa.
  // Similar for dimMeDa for the associated dimensions.

  std::vector<struct DimensionMetaData> vs_f_DMD_entries;

  // Scan through the standard output table, respectivels variable requests.
  if( QA::tableSheet.size() && ! vMD.var->isExcluded )
  {
    checkMIP_table(in, vMD, vs_f_DMD_entries) ;

/*
    if( checkMIP_table(in, vMD, vs_f_DMD_entries) )
    {
      // very special: a tracer variable was not found
      // in table sheet Omon, because it is defined in Oyr, but
      // some properties from the Omon
      if( QA::tableSheet == "Omon" )
      {
        tableSheetBuf = "Omon" ;
        QA::tableSheet = "Oyr" ;

        is = checkMIP_table(in, vMD, vs_f_DMD_entries) ;

        QA::tableSheet = "Omon" ;
        QA::tableSheetSub =  "Marine Bioge" ;
        tableSheetBuf = "Oyr" ;
      }
      else if( QA::tableSheet == "cf3hr" )
      {
        tableSheetBuf = "cf3hr" ;
        QA::tableSheet = "Amon" ;
        std::string saveCellMethods(vMD.attMap[n_cell_methods]);
        vMD.attMap[n_cell_methods]="time: point";

        is = checkMIP_table(in, vMD, vs_f_DMD_entries) ;

        // switch back to the original table required for the project table entry
        QA::tableSheet = "cf3hr" ;
        QA::tableSheetSub.clear() ;
        tableSheetBuf = "Amon" ;

        if( ! is )
          vMD.attMap[n_cell_methods]=saveCellMethods;
      }
      else if( QA::tableSheet == "cfSites" )
      {
        tableSheetBuf = "cfSites" ;
        QA::tableSheet = "Amon" ;
        std::string saveCellMethods(vMD.attMap[n_cell_methods]);
        vMD.attMap[n_cell_methods]="time: point";

        is = checkMIP_table(in, vMD, vs_f_DMD_entries) ;

        // switch back to the original table required for the project table entry
        QA::tableSheet = "cfSites" ;
        QA::tableSheetSub =  "CFMIP 3-ho" ;
        tableSheetBuf = "Amon" ;

        if( ! is )
          vMD.attMap[n_cell_methods]=saveCellMethods;
      }
    }
*/
  }

  return;
}

void
CMOR::bufTableSheets(VariableMetaData& vMD)
{
/*
  if( tableSheetBuf.size() )
  {
    if( QA::tableSheet == "Omon" )
      QA::tableSheetSub = "Marine Bioge" ;
    else if( QA::tableSheet == "cf3hr" )
    {
      tableSheetSub.clear() ;

      if( tableSheetBuf.size() == 2 )
        vMD.attMap[n_cell_methods]=tableSheetBuf[1];
    }
    else if( QA::tableSheet == "cfSites" )
    {
      QA::tableSheetSub = "CFMIP 3-ho" ;

      if( tableSheetBuf.size() == 2 )
        vMD.attMap[n_cell_methods]=tableSheetBuf[1];
    }
  }

  else
  {
*/

  if( QA::tableSheet == "Omon" )
  {
    tableSheetBuf.push_back("Oyr") ;
    tableSheetBuf.push_back(QA::tableSheet) ;
  }
  else if( QA::tableSheet == "cf3hr" )
  {
    tableSheetBuf.push_back(QA::tableSheet) ;
    QA::tableSheet = "Amon" ;
    tableSheetBuf.push_back(vMD.attMap[n_cell_methods]);
    vMD.attMap[n_cell_methods]="time: point";
  }
  else if( QA::tableSheet == "cfSites" )
  {
    tableSheetBuf.push_back(QA::tableSheet) ;
    QA::tableSheet = "Amon" ;
    tableSheetBuf.push_back(vMD.attMap[n_cell_methods]);
    vMD.attMap[n_cell_methods]="time: point";
  }
  else
    tableSheetBuf.push_back(QA::tableSheet) ;

//  }

  return;
}

// class with project specific purpose
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

     if( split[0] == "eA" || split[0] == "excludedAttribute"
         || split[0] == "excluded_attribute" )
     {
       Split cvs(split[1],",");
       for( size_t i=0 ; i < cvs.size() ; ++i )
         excludedAttribute.push_back(cvs[i]);
       continue;
     }

     if( split[0] == "tVR"
          || split[0] == "table_variable_requirements" )
     {
       if( split.size() == 2 )
          varReqTable.setFile(split[1]);
       continue;
     }

     if( split[0] == "uS" || split[0] == "useStrict"
            || split[0] == "use_strict" )
     {
          isUseStrict=true ;
          pQA->setCheckMode("meta");
     }
     continue;
   }

   // apply a general path which could have also been provided by setTablePath()
   if( varReqTable.path.size() == 0 )
      varReqTable.setPath(pQA->tablePath);

   return;
}

void
QA_Exp::checkDataVarNum(void)
{
  // each file must contain only a single output field from a single simulation
  if( pQA->pIn->dataVarIndex.size() == 1 )
    return;

  if( pQA->pIn->dataVarIndex.size() == 0 )
  {
    if( fVarname.size() )
    {
      // unfortunately, no data-variable was found previously
      for( size_t i=0 ; i < pQA->pIn->varSz ; ++i )
      {
        if( pQA->pIn->variable[i].name == fVarname )
        {
          // correction
          pQA->pIn->dataVarIndex.push_back(i);
          pQA->pIn->variable[i].isDATA=true;
          return;
        }
      }
    }

    std::string key("9_1b");
    if( notes->inq( key) )
    {
      std::string capt("no data variable present") ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  std::string key("9_1a");
  if( notes->inq( key) )
  {
    std::string capt("multiple variables, found: ") ;

    for( size_t i=0 ; i < pQA->pIn->dataVarIndex.size() ; ++i)
    {
      if(i)
        capt += ", ";

      capt += pQA->pIn->variable[pQA->pIn->dataVarIndex[i]].name;
    }

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
QA_Exp::checkMetaData(InFile& in)
{
  // is it NetCDF-3 classic?
  checkNetCDF(in);

  // check basic properties between this file and
  // requests in the table. When a MIP-subTable is empty, use the one
  // from the previous instance.
  CMOR cmor(pQA);

  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    if( varMeDa[i].var->isDATA )
    {
      cmor.run(in, varMeDa[i] );
    }
  }

  // inquire passing the meta-data check
  int ev;
  if( (ev = notes->getExitValue()) > 1 )
    pQA->setExit( ev );

  return;
}

void
QA_Exp::checkNetCDF(InFile& in)
{
  // NC_FORMAT_CLASSIC (1)
  // NC_FORMAT_64BIT   (2)
  // NC_FORMAT_NETCDF4 (3)
  // NC_FORMAT_NETCDF4_CLASSIC  (4)

  int fm = in.nc.inqNetcdfFormat();

  if( fm == 1 || fm == 4 )
    return;

  std::string s("");

  if( fm == 2 )
  {
    s = "3, 64BIT formatted";
  }
  else if( fm == 3 )
  {
    s = "4, ";

    if( in.nc.inqDeflate() )
      s += "deflated (compressed)";
  }

  if( s.size() )
  {
    std::string key("12");
    if( notes->inq( key, pQA->fileStr ) )
    {
      std::string capt("format does not conform to netCDF classic, found") ;
      capt += hdhC::tf_val(s);

      (void) notes->operate( capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
    }
  }

  return;
}

void
QA_Exp::createVarMetaData(void)
{
  // create instances of VariableMetaData. These have been identified
  // previously at the opening of the nc-file and marked as
  // Variable::VariableMeta(Base)::isDATA == true. The index
  // of identified targets is stored in the InFile::dataVarIndex vector.

  //there should be only present a single data variable
  checkDataVarNum();

  for( size_t i=0 ; i< pQA->pIn->dataVarIndex.size() ; ++i )
  {
    Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[i]];

    pushBackVarMeDa( &var );  //push next instance
  }

  if( pQA->pIn->dataVarIndex.size() > 1 )
  {
     std::string key("9_1");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("multiple data variables are present, found ");

       for( size_t j=0 ; j < pQA->pIn->dataVarIndex.size() ; ++j )
       {
         Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[j]];
         if(j)
           capt += ", ";

         capt += var.name;
       }

       (void) notes->operate(capt) ;
       notes->setCheckStatus("CV",  pQA->n_fail );
     }
  }

  return;
}

std::string
QA_Exp::getCaptionIntroDim(
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD, std::string att)
{
  std::string& t_DMD_outname = t_DMD.attMap[CMOR::n_output_dim_name] ;

  std::string intro("table=");
  intro += QA::tableSheet + ", var(file)=";
  intro += f_DMD.var->name + ", CMOR-dim=";
  intro += t_DMD.attMap[CMOR::n_CMOR_dimension] ;

  if( t_DMD_outname == "basin" )
    intro += ", region";
  if( t_DMD_outname == "line" )
    intro += ", passage";
  if( t_DMD_outname == CMOR::n_type )
    intro += ", type_description";

  if( att.size() )
    intro += ", " + att ;

  intro += ": ";
  return intro;
}

std::string
QA_Exp::getCaptionIntroVar( std::string table,
    struct VariableMetaData& f_VMD, std::string att)
{
  std::string intro("table=");
  intro += table + ", var=";
  intro += f_VMD.var->name ;

  if( att.size() )
    intro += ", " + att ;

  intro += ": ";
  return intro;
}

std::string
QA_Exp::getFrequency(void)
{
  if( frequency.size() )
    return frequency;  // already known

  // get frequency from attribute (it is required)
  frequency = pQA->pIn->nc.getAttString("frequency") ;

  if( frequency.size() )
    return frequency;  // no frequency provided

  // not found, but error issue is handled elsewhere

  // try the filename
  std::string f( pQA->pIn->file.basename );

  Split splt(f, "_");

  // the second term denotes the mip table for CMIP5
  std::string mip_f = splt[1];

  // now, try also global att 'table_id'
  Split mip_a( pQA->pIn->nc.getAttString(CMOR::n_table_id) ) ;

  if( mip_a.size() > 1 )
  {
    if( mip_a[1] != mip_f && mip_a[1].size() )
      mip_f = mip_a[1]; // annotation issue?

    // convert mip table --> frequency
    if( mip_f.substr(mip_f.size()-2) == "yr" )
      frequency = "yr" ;
    else if( mip_f.substr(mip_f.size()-3) == "mon" )
      frequency = "mon" ;
    else if( mip_f == "day" )
      frequency = "day" ;
    else if( mip_f.substr(mip_f.size()-3) == "Day" )
      frequency = "day" ;
    else if( mip_f.substr(0,3) == "6hr" )
      frequency = "6hr" ;
    else if( mip_f.substr(0,3) == "3hr" )
      frequency = "3hr" ;
    else if( mip_f.substr(mip_f.size()-3) == "3hr" )
      frequency = "3hr" ;
  }

  return frequency ;
}

std::string
QA_Exp::getMIP_tableName(std::string tName)
{
  if( QA::tableSheet.size() )
    return QA::tableSheet;

  // the counter-parts in the attributes
  std::string tableID;
  if( tName.size() )
    tableID = tName ;
  else
    tableID = pQA->pIn->nc.getAttString(CMOR::n_table_id) ;

  if( tableID.size() == 0 )
    return tableID;

  Split x_tableID(tableID);

  // The table sheet name from the global attributes.
  // Find valid names, even with deviations
  size_t x_tSz = x_tableID.size() ;

  if( x_tSz > 1 )
  {
    if(  x_tableID[0].substr(1,4) == "able"
        || x_tableID[0].substr(1,4) == "ABLE" )
      QA::tableSheet = x_tableID[1] ;
  }
  else if( x_tableID.size() )
    QA::tableSheet = x_tableID[0] ;

  // check the format of the total line
  bool is=true;
  for( size_t i=0 ; i < MIP_tableNames.size() ; ++i )
  {
    if( MIP_tableNames[i] == QA::tableSheet )
    {
      is=false ;
      break;
    }
  }

  if( is )
  {
    std::string key("7_7a");

    if( notes->inq( key, CMOR::n_global) )
    {
      std::string capt("invalid MIP table name in global ") ;
      capt += hdhC::tf_att(hdhC::empty, CMOR::n_table_id, x_tableID.getStr()) ;

      (void) notes->operate(capt) ;
      pQA->setExit( notes->getExitValue() ) ;
    }

    QA::tableSheet.clear();
    return QA::tableSheet;
  }

  // Is the date of the table given; within ()?
  size_t p0,p1;
  std::string dateItem;
  if( (p0=tableID.find("(")) < std::string::npos )
    if( (p1=tableID.find(")", p0)) < std::string::npos )
      dateItem = tableID.substr(p0+1, p1-p0-1);

  is=true;
  if( dateItem.size() )
    if( Date::isValidDate(dateItem) )
      is=false;

  if( is )
  {
    std::string key("7_7b");

    if( notes->inq( key, CMOR::n_global) )
    {
      std::string capt(CMOR::n_global + hdhC::blank) ;
      capt += hdhC::tf_att(hdhC::empty, CMOR::n_table_id, x_tableID.getStr()) ;
      capt += ": missing date of the MIP table, expected e.g. (5 Jan 2016)";

      (void) notes->operate(capt) ;
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return QA::tableSheet;
}

std::string
QA_Exp::getTableEntryID(std::string vName)
{
  vName += "," ;
  vName += getFrequency() ;

  return vName + ",";
}

std::string
QA_Exp::getVarnameFromFilename(void)
{
  if( fVarname.size() )
    return fVarname;

  return getVarnameFromFilename(pQA->pIn->file.getFilename()) ;
}

std::string
QA_Exp::getVarnameFromFilename(std::string fName)
{
  size_t pos;
  if( (pos = fName.find("_")) < std::string::npos )
    fName = fName.substr(0,pos) ;

  return fName;
}

void
QA_Exp::init(std::vector<std::string>& optStr)
{
   // apply parsed command-line args
   applyOptions(optStr);

   if( pQA->isCheckDRS || pQA->isCheckCV || pQA->isCheckTime )
   {
     fVarname = getVarnameFromFilename(pQA->pIn->file.filename);
     getFrequency();
   }

   if( pQA->isCheckCV || pQA->isCheckData )
   {
     // Create and set VarMetaData objects.
     createVarMetaData() ;
   }

   // Check varname from filename with those in the file.
   // Is the shortname in the filename also defined in the nc-header?
   if( pQA->isCheckCV)
   {
     std::vector<std::string> vNames( pQA->pIn->nc.getVarName() ) ;
     size_t i;
     for( i=0 ; i < vNames.size() ; ++i )
       if( fVarname == vNames[i] )
         break;

     if( i == vNames.size() )
     {
       std::string key("1_3");
       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("variable ");
         capt += hdhC::tf_assign("acronym", fVarname);
         capt += " in the filename does not match any variable in the file" ;

         (void) notes->operate(capt) ;
         notes->setCheckStatus("CV",  pQA->n_fail );
       }
     }
   }

   return;
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
      if( prevTargets[i] == varMeDa[j].var->name )
        break;

    if( j == varMeDa.size() )
    {
       std::string key("3_14");
       if( notes->inq( key, prevTargets[i]) )
       {
         std::string capt("variable");
         capt += hdhC::tf_val(prevTargets[i]) ;
         capt += " is missing in sub-temporal file" ;

         if( notes->operate(capt) )
         notes->setCheckStatus("Consistency", pQA->n_fail );
         pQA->setExit( notes->getExitValue() ) ;
       }
    }
  }

  // a new variable?
  for( size_t j=0 ; j < varMeDa.size() ; ++j)
  {
    size_t i;
    for( i=0 ; i < prevTargets.size() ; ++i)
      if( prevTargets[i] == varMeDa[j].var->name )
        break;

    if( i == prevTargets.size() )
    {
       std::string key("3_15");
       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("variable");
         capt += hdhC::tf_val(varMeDa[j].var->name);
         capt += " is new in sub-temporal file" ;

         if( notes->operate(capt) )
         notes->setCheckStatus("Consistency", pQA->n_fail );
         pQA->setExit( notes->getExitValue() ) ;
       }
    }
  }

  return;
}

bool
QA_Exp::inqTables(void)
{
  if( ! varReqTable.isExisting(varReqTable.path) )
  {
     std::string key("7_12");

     if( notes->inq( key, pQA->fileStr) )
     {
        std::string capt("no path to a table, tried ") ;
        capt += varReqTable.path;

        (void) notes->operate(capt) ;
        notes->setCheckStatus("QA_path", pQA->n_fail);
        pQA->setExit( notes->getExitValue() ) ;
     }

     return false;
  }

  return true;
}

/*
bool
QA_Exp::locate( GeoData<float> *gd, double *alat, double *alon, const char* crit )
{
  std::string str(crit);

// This is a TODO; but not needed for QA
  MtrxArr<float> &va=gd->getCellValue();
  MtrxArr<double> &wa=gd->getCellWeight();

  size_t i=0;
  double val;  // store extrem value
  int ei=-1;   // store index of extreme

  // find first valid value for initialisation
  for( ; i < va.size() ; ++i)
  {
      if( wa[i] == 0. )
        continue;

      val= va[i];
      ei=i;
      break;
  }

  // no value found
  if( ei < 0 )
  {
    *alat = 999.;
    *alon = 999.;
    return true;
  }

  if( str == "min" )
  {
    for( ; i < va.size() ; ++i)
    {
        if( wa[i] == 0. )
          continue;

        if( va[i] < val )
        {
          val= va[i];
          ei=i;
        }
    }
  }
  else if( str == "max" )
  {
    for( ; i < va.size() ; ++i)
    {
        if( wa[i] == 0. )
          continue;

        if( va[i] > val )
        {
          val= va[i];
          ei=i;
        }
    }
  }

  *alat = gd->getCellLatitude(static_cast<size_t>(ei) );
  *alon = gd->getCellLongitude(static_cast<size_t>(ei) );

  return false;
}
*/

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

      // some more properties
      vMD.var->std_name = vMD.var->getAttValue(CMOR::n_standard_name);

      std::string s( vMD.var->getAttValue(CMOR::n_long_name));
      vMD.attMap[CMOR::n_long_name] = hdhC::unique(s, ' ');
      if( notes->inq( "7_14", "", "INQ_ONLY") )
         vMD.attMap[CMOR::n_long_name]
           = hdhC::Lower()(vMD.attMap[CMOR::n_long_name]) ;

      vMD.attMap[CMOR::n_cell_methods] = vMD.var->getAttValue(CMOR::n_cell_methods);
      vMD.attMap[CMOR::n_cell_measures] = vMD.var->getAttValue(CMOR::n_cell_measures);
      vMD.attMap[CMOR::n_type] = pQA->pIn->nc.getVarTypeStr(var->name);
   }

   return;
}

void
QA_Exp::run(void)
{
  if( inqTables() )
  {
    QA::tableSheet = getMIP_tableName() ;

    if( pQA->drs_cv_table.table_DRS_CV.is )
    {
      if(pQA->isCheckDRS)
      {
        DRS_CV drsFN(pQA);
        drsFN.run();
      }
    }

    if(pQA->isCheckCV)
    {
      // get meta data from file and compare with tables
      checkMetaData(*(pQA->pIn));
    }
  }

  return ;
}

void
QA_Exp::setParent(QA* p)
{
   pQA = p;
   notes = p->notes;
   return;
}

VariableMetaData::VariableMetaData(QA *p, Variable *v)
{
   pQA = p;

   if( v )
   {
     var = v;
     isNewVar=false;
   }
   else
   {
     var = new Variable ;
     isNewVar=true;
   }
}

VariableMetaData::~VariableMetaData()
{
  if( isNewVar )
    delete var;

  dataOutputBuffer.clear();
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
          notes->setCheckStatus("CV",  pQA->n_fail );
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
          notes->setCheckStatus("CV",  pQA->n_fail );
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
