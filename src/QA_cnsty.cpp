#include "qa_cnsty.h"

Consistency::Consistency(QA *p0, InFile *p1,
    std::vector<std::string> &opt, std::string tPath )
{
   pQA = p0;
   pIn = p1;
   checkEnabled=true;

   applyOptions(opt, tPath);

   excludedAttributes.push_back("comment");
   excludedAttributes.push_back("history");
   excludedAttributes.push_back("associated_files");

   status=false;
}

void
Consistency::applyOptions(std::vector<std::string> &optStr,
                          std::string& tablePath)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "dCC"
          || split[0] == "disableConsistencyCheck"
             || split[0] == "disable_consistency_check")
     {
       checkEnabled=false;
       return;
     }

     if( split[0] == "tP"
          || split[0] == "tablePath" || split[0] == "table_path")
     {
       if( split.size() == 2 )
          tablePath=split[1];

       continue;
     }

     if( split[0] == "tPr"
          || split[0] == "tableProject" )  // dummy
     {
       if( split.size() == 2 )
          consistencyTableFile.setFile(split[1]);
     }

   }

   if( consistencyTableFile.path.size() == 0 )
      consistencyTableFile.setPath(tablePath);

   return;
}

bool
Consistency::check(void)
{
  for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i )
  {
     Variable& var = pIn->variable[pIn->dataVarIndex[i]];

     std::string entryID( pQA->qaExp.getTableEntryID(var.name) );

     if( check(var, entryID) )
     {
       write(var, entryID);
       status=true;
     }
  }

  return status;
}

bool
Consistency::check(Variable &dataVar, std::string entryID)
{
  // return value is true, when there is not a project table, yet.

  // Search the project table for an entry
  // matching the varname and frequency (account also rotated).

  // Open project table. Mode:  read
  std::string str0(consistencyTableFile.getFile());

  std::ifstream ifs(str0.c_str(), std::ios::in);
  if( !ifs.is_open() )  // file does not exist
     return true;  // causes writing of a new entry

  size_t sz_PV = entryID.size();

  std::string t_md;
  bool notFound=true;

  while( getline(ifs, str0) )
  {
    if( str0.substr(0,sz_PV) != entryID )
      continue;

    // found a valid entry
    notFound=false;
    t_md = hdhC::stripSides(str0) ;

    // read aux-lines
    while( getline(ifs, str0) )
    {
      str0 = hdhC::stripSides(str0) ;
      if( str0.substr(0,4) != "aux=" )
        goto BREAK ;  // found the end of the entry

      t_md += '\n';
      t_md += str0 ;
    }
  }

  if(notFound)
    return true;  // entry not found

BREAK:

  // close the project table
  ifs.close();

  // get meta data info from the file
  std::string f_md;
  getMetaData(dataVar, entryID, f_md);

  // Comparison of meta-data from the project table and the file, respectively.
  // Note that the direct comparison fails because of different spaces.

  // Deviating distribution of auxiliaries and attributes is tolerated.
  // Use index-vector for book-keeping.
  // Meaning: x=split, t=file, t=table, a=attribute, eq=(att-name,att-value)

  Split splt_xt(t_md, '\n');
  Split splt_xf(f_md, '\n');
  size_t xt_sz=splt_xt.size();
  size_t xf_sz=splt_xf.size();

  std::vector<std::string> xf;
  for( size_t i=0 ; i < xf_sz ; ++i )
    xf.push_back( hdhC::stripSides(splt_xf[i]) );

  std::vector<std::string> xt;
  for( size_t i=0 ; i < xt_sz ; ++i )
    xt.push_back( hdhC::stripSides(splt_xt[i]) );

  // simple test for identity
  if( xt_sz == xf_sz )
  {
    bool is=true;
    for( size_t i=0 ; i < xt_sz ; ++i )
    {
       if( xt[i] != xf[i] )
       {
         is=false ;
         break;
       }
    }

    if( is )
       return false;
  }

  // store indexes
  std::vector<int> xf_ix;
  for( size_t i=0 ; i < xf_sz ; ++i )
    xf_ix.push_back( i );

  std::vector<int> xt_ix;
  for( size_t i=0 ; i < xt_sz ; ++i )
    xt_ix.push_back( i );

  // At first, test whether total auxiliary entries are identical.
  for( size_t i=0 ; i < xt_sz ; ++i )
  {
    for( size_t j=0 ; j < xf_sz ; ++j )
    {
       // only test strings for indexes still to be checked
       if( xt_ix[i] > -1 && xf_ix[i] > -1 && xf[j] == xt[i] )
       {
         xt_ix[i] = -1;
         xf_ix[j] = -1;
         break;
       }
    }
  }

  Split xf_a;  // meta-data from file
  xf_a.setSeparator(',');
  Split xt_a;  // from the table
  xt_a.setSeparator(',');

  Split xf_eq;
  xf_eq.setSeparator('=');
  Split xt_eq;
  xt_eq.setSeparator('=');

  bool isMissAux;
  for( size_t ixt=0 ; ixt < xt_ix.size() ; ++ixt )
  {
    int jt=xt_ix[ixt];

    if( jt < 0 )
      continue;  // passed already a check

    isMissAux=true;

    //split at comma
    xt_a = xt[jt] ;

    for( size_t ixf=0 ; ixf < xf_ix.size() ; ++ixf )
    {
      int jf=xf_ix[ixf];

      if( jf < 0 )
        continue;  // passed already a check

      //split at comma
      xf_a = xf[jf] ;

      // compare the names of the auxiliaries
      if( xf_a[0] != xt_a[0] )
        continue;

      isMissAux=false;

      // scan the attributes
      for( size_t ita=1 ; ita < xt_a.size() ; ++ita )
      {
        // split at '='
        xt_eq = xt_a[ita] ;

        bool isAttMissing=true;

        for( size_t ifa=1 ; ifa < xf_a.size() ; ++ifa )
        {
          // split at '='
          xf_eq = xf_a[ifa] ;

          if( xt_eq[0] == xf_eq[0] ) // found identical attribute names
          {
            isAttMissing=false;
            if( xt_eq[1] == xf_eq[1] )  // found identical values
              break;  // try the next attribute

            // this is a very special one for time: one with separator T and/or Z
            // the other one without
            std::string auxName;
            if( xt_a[0].substr(0,4) == "aux=" )
              auxName = xt_a[0].substr(4) ;
            else
              auxName = xt_a[0] ;

            if( auxName == "time" )
            {
               Split x_tt(xt_eq[1]," TZ",true) ;
               Split x_ff(xf_eq[1]," TZ",true) ;

               if( x_tt.size() == x_ff.size() )
               {
                 bool is=true;
                 for(size_t c=0 ; c < x_tt.size()  ; ++c )
                   if( x_tt[c] != x_ff[c] )
                     is=false;

                 if(is)
                   break;
               }
            }

            // different values --> annotation
            status=true;

            std::string key("8_8");
            if( notes->inq( key, dataVar.name ) )
            {
              std::string capt;
              if( xt_a[0].substr(0,4) == "aux=" )
              {
                capt = "auxiliary ";
                capt += hdhC::tf_var(auxName, hdhC::colon) ;
              }
              else
                capt += hdhC::tf_var(xt_a[0], hdhC::colon) ;

              if( xt_eq[0] == "values" )
                capt += "data has changed";
              else
              {
                capt += xt_eq[0] ;
                capt += " has changed from";
                capt += hdhC::tf_val(xt_eq[1]) ;
                capt += " to";
                capt += hdhC::tf_val(xf_eq[1]) ;
              }
              capt += " across experiment or sub-temporal files";

              (void) notes->operate(capt) ;
              notes->setCheckStatus("Consistency","FAIL" );
            }

            break;  // try next
          }
        }

        if( isAttMissing )
        {
          status=true;

          std::string key("8_7");
          if( notes->inq( key, dataVar.name ) )
          {
            std::string capt;
            if( xt_a[0].substr(0,4) == "aux=" )
            {
              capt = "auxiliary ";
              capt += hdhC::tf_var(xt_a[0].substr(4), hdhC::colon) ;
            }
            else
              capt += hdhC::tf_var(xt_a[0], hdhC::colon) ;

            if( xt_eq[0] == "values" )
              capt += "no data";
            else
            {
              capt += hdhC::tf_att(xt_eq[0]);
              capt += "is missing " ;
            }

            if( pQA->currQARec )
              capt += "across sub-temporal files";
            else
              capt += "across experiments";

            (void) notes->operate(capt) ;
            notes->setCheckStatus("Consistency","FAIL" );
          }
        }
      }
    }

    if( isMissAux )
    {
      status=true;

      std::string key("8_4");
      if( notes->inq( key, dataVar.name ) )
      {
        std::string capt;
        if( xt_a[0].substr(0,4) == "aux=" )
        {
          capt = "auxiliary ";
          capt += hdhC::tf_var(xt_a[0].substr(4), hdhC::colon) ;
        }
        else
          capt += hdhC::tf_var(xt_a[0], hdhC::colon) ;

        capt += "missing across experiments or sub-temporal files";

        (void) notes->operate(capt) ;
        notes->setCheckStatus("Consistency","FAIL" );
      }
    }
  }


  // test for missing attributes in the table (reversed for-loops)
  bool isAddAux=true;
  for( size_t ixf=0 ; ixf < xf_ix.size() ; ++ixf )
  {
    int jf=xf_ix[ixf];

    if( jf < 0 )
      continue;  // passed already a check

    isAddAux=true;

    //split at comma
    xf_a = xf[jf] ;

    for( size_t ixt=0 ; ixt < xt_ix.size() ; ++ixt )
    {
      int jt=xt_ix[ixt];

      if( jt < 0 )
        continue;  // passed already a check

      //split at comma
      xt_a = xt[jt] ;

      // compare the names of the auxiliaries
      if( xf_a[0] != xt_a[0] )
        continue;

      // found identical auxiliaries
      isAddAux=false;

      // scan the attributes
      for( size_t ifa=1 ; ifa < xf_a.size() ; ++ifa )
      {
        // split at '='
        xf_eq = xf_a[ifa] ;
        bool isAddAtt=true;

        for( size_t ita=1 ; ita < xt_a.size() ; ++ita )
        {
          // split at '='
          xt_eq = xt_a[ita] ;

          if( xt_eq[0] == xf_eq[0] ) // found identical attribute names
          {
            isAddAtt=false;
            break;
          }
        }

        if( isAddAtt )
        {
          status=true;

          // additional attribute of auxiliary in the file
          std::string key("8_6");
          if( notes->inq( key, dataVar.name ) )
          {
            std::string capt;
            if( xt_a[0].substr(0,4) == "aux=" )
            {
              capt = "auxiliary ";
              capt += hdhC::tf_var(xf_a[0].substr(4), hdhC::colon) ;
            }
            else
              capt += hdhC::tf_var(xf_a[0], hdhC::colon) ;

            if( xt_eq[0] == "values" )
            {
              capt += "additional data across experiments or sub-temporal files";
            }
            else
            {
              capt += hdhC::tf_att(xf_eq[0]);
              capt += "is new across experiments or sub-temporal files";
            }

            (void) notes->operate(capt) ;
            notes->setCheckStatus("Consistency","FAIL" );
          }
        }
      }
    }

    if( isAddAux )
    {
      status=true;

      std::string key("8_5");
      if( notes->inq( key, dataVar.name) )
      {
        std::string capt("additional auxiliary ");
        if( xf_a[0].size() > 4 )
          capt += hdhC::tf_var(xf_a[0].substr(4)) ;
        else
          capt += hdhC::tf_var(xf_a[0]) ;
        capt += "across experiments or sub-temporal files";

        (void) notes->operate(capt) ;
        notes->setCheckStatus("Consistency","FAIL" );
      }
    }
  }

  return false;
}

void
Consistency::getAtts(Variable &var, std::string &s)
{
   size_t sz = var.attName.size();

   for(size_t j=0 ; j < sz ; ++j )
   {
     std::string &aN = var.attName[j] ;

     bool isCont=false;
     for( size_t x=0 ; x < excludedAttributes.size() ; ++x )
     {
       if( aN == excludedAttributes[x] )
       {
         isCont=true;
         break;
       }
     }

     if( isCont )
       continue;

     s += ',';
     s += var.attName[j] ;
     s += '=';

     if( var.attValue[j].size() )
     {
        // convert comma --> space
       std::string &t = var.attValue[j][0] ;
       size_t pos;
       while( (pos=t.find(',')) < std::string::npos )
          t[pos]=' ';

       s += t ;
     }
   }

   // get checksum of values
   if( ! ( var.isUnlimited() || var.isDATA) )
     getValues(var, s);

   return;
}

void
Consistency::getMetaData(Variable &dataVar,
    std::string& entryID, std::string &md)
{
  // Put all meta-data to string.

  // beginning of extraction of meta data
  md = "\n" + entryID;

  // dimensions
  md += "dims=";
  md += dataVar.getDimNameStr(false, ' ');

  // get attributes of the data variable
  getAtts(dataVar, md);
  getVarType(dataVar, md);

  // get meta-data of auxiliaries
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].isDATA )
       continue;

    md += "\n  aux=" ;
    md += pIn->variable[i].name;

    getAtts(pIn->variable[i], md);
    getVarType(pIn->variable[i], md);
  }

  return;
}

void
Consistency::getValues(Variable &var, std::string &s)
{
   // Get the checksum of the limited variables.
   // The purpose of checksums of auxiliary variables is to
   // ensure consistency between follow-up experiments.
   // Note: targets have NC_DOUBLE and all other have NC_FLOAT

   uint32_t ck=0;
   std::string str;
   std::vector<std::string> vs;

   if( var.isUnlimited() )
     return;

   // get data and determine the checksum
   if( ! var.checksum )
   {
      if( var.type == NC_CHAR )
      {
        pIn->nc.getData(vs, var.name);
        bool reset=true;  // is set to false during the first call
        for(size_t l=0 ; l < vs.size() ; ++l)
        {
          vs[l] = hdhC::stripSides(vs[l]);
          ck = hdhC::fletcher32_cmip5(vs[l], &reset) ;
        }
      }
      else
      {
          MtrxArr<double> mv;
          pIn->nc.getData(mv, var.name );

          if( mv.size() > 0 )
            ck = hdhC::fletcher32_cmip5(mv.arr, mv.size()) ;
      }
   }

   s += ",values=";
   s += hdhC::double2String(ck, "p=|adj,float") ;

   return;
}

void
Consistency::getVarType(Variable &var, std::string &s)
{
   // Get the checksum of all non-unlimited variables
   // The purpose of checksums of auxiliary variables is to
   // ensure consistency between follow-up experiments.
   // Note: targets have NC_DOUBLE and all other have NC_FLOAT

   // get type of the variable
   std::string type(pIn->nc.getVarTypeStr( var.name ) ) ;
   s += ",type=";
   s += type;

   return;
}

bool
Consistency::lockFile(std::string &fName )
{
  //test for a lock
  std::string lockFile(fName);
  lockFile += ".lock" ;

  std::string test("/bin/bash -c \'test -e ") ;
  test += lockFile ;
  test += '\'' ;

  std::string lock("/bin/bash -c \'touch ");
  lock += lockFile ;
  lock += "  &> /dev/null\'" ;

  // see 'man system' for the return value, here we expect 0,
  // if file exists.

  size_t count=0;

  // THIS WILL lock until the lock-file is removed
  while ( ! system( test.c_str() ) )
  {
    sleep( 1 ) ;  // in seconds
    ++count;

    if( count == 1800 )  // 1/2 hr
    {
      std::string key("8_3");
      if( notes->inq( key, "PT") )
      {
         std::string capt("consistency-check table is locked for 1/2 hour") ;

         (void) notes->operate(capt) ;
         pQA->setExitState( notes->getExitState() ) ;
      }
    }
  }

  if( system( lock.c_str() ) )
  {
     std::string key("8_2");
     if( notes->inq( key, "PT") )
     {
        std::string capt("could not lock the consistency-check table") ;

        if( notes->operate(capt) )
          pQA->setExitState( notes->getExitState() ) ;
     }

     return true;
  }

  return false;
}

void
Consistency::setExcludedAttributes(std::vector<std::string> &v)
{
   for( size_t i=0 ; i < v.size() ; ++i )
     excludedAttributes.push_back(v[i]) ;
   return;
}

bool
Consistency::unlockFile(std::string &fName )
{
  //test for a lock
  std::string lockFile(fName);
  lockFile += ".lock" ;

  std::string unlock("/bin/bash -c \'rm -f ");
  unlock += lockFile ;
  unlock += '\'' ;

  // see 'man system' for the return value, here we expect 0,
  // if file exists.

  size_t retries=5;

  for( size_t r=0 ; r < retries ; ++r )
  {
    if( ! system( unlock.c_str() ) )
      return false;
    sleep(1);
  }

  return true;
}

void
Consistency::write(Variable &dataVar, std::string& entryID)
{
  // store meta data info
  std::string md;

  getMetaData(dataVar, entryID, md);

  std::string pFile =  consistencyTableFile.getFile();

  lockFile(pFile);

  // open the file for appending.
  std::fstream oifs;

  oifs.open(pFile.c_str(),
          std::ios::in | std::ios::out );

  if( !oifs) // does not exist
    oifs.open(pFile.c_str(),
          std::ios::out | std::ios::trunc | std::ios::in);

  if (! oifs.is_open() )
  {
    std::string key("8_1");
    if( notes->inq( key, "PT") )
    {
      std::string capt("could not create a consistency-check table") ;

      if( notes->operate(capt) )
      {
        notes->setCheckStatus("QA_PT_table", pQA->n_fail );
        pQA->setExitState( notes->getExitState() ) ;
      }
    }
  }
  else
  {
    // append
    oifs.seekp(0, std::ios::end);

    oifs << md << std::flush;
    oifs.close();
  }

  unlockFile(pFile);

  return;
}
