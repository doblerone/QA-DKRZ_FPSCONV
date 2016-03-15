void
DRS_CV_Table::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "tCV"
           || split[0] == "tableControlledVocabulary"
                || split[0] == "table_controlled_vocabulary" )
     {
       if( split.size() == 2 )
          table_DRS_CV.setFile(split[1]) ;

       continue;
     }

     if( split[0] == "tP"
          || split[0] == "tablePath" || split[0] == "table_path")
     {
       if( split.size() == 2 )
          tablePath=split[1];

       continue;
     }

   }

   if( table_DRS_CV.path.size() == 0 )
      table_DRS_CV.setPath(tablePath);

   return;
}

void
DRS_CV_Table::read(void)
{
  hdhC::FileSplit& fs = table_DRS_CV;

  ReadLine ifs(fs.getFile());

  if( ! ifs.isOpen() )
  {
     std::string key("7_1");
     if( pQA->notes->inq( key) )
     {
        std::string capt("no path to a table, tried " + fs.getFile()) ;

        if( notes->operate(capt) )
        {
          pQA->notes->setCheckMetaStr( pQA->fail );
          pQA->setExit( notes->getExitValue() ) ;
        }
     }

     return;
  }

  std::string s0;
  std::vector<std::string> vs;

  // parse table; trailing ':' indicates variable or 'global'
  ifs.skipWhiteLines();
  ifs.skipComment();
  ifs.skipCharacter("<>");
  ifs.clearSurroundingSpaces();

  Split x_item;
  x_item.setSeparator("=");
  x_item.setStripSides(" ");

  bool isDS=false;
  bool isFE=false;

  while( ! ifs.getLine(s0) )
  {
    if( s0.size() == 0 )
      continue;

    if( s0 == "BEGIN: DRS" )
    {
      while( ! ifs.getLine(s0) )
      {
        size_t pos;
        if( s0 == "END: DRS" )
        {
          isDS=false;
          isFE=false;
          break;
        }
        else if( (pos=s0.find(':')) < std::string::npos )
        {
          if( s0.substr(pos-3, 4) == "-DS:" )
          {
            isDS=true;
            isFE=false;
            pathEncodingName.push_back(s0.substr(0, pos-3));
            pathEncoding.push_back(
              hdhC::stripSides( s0.substr(++pos)) );
          }
          else if( s0.substr(pos-3, 4) == "-FE:" )
          {
            isDS=false;
            isFE=true;
            fileEncodingName.push_back(s0.substr(0, pos-3));
            fileEncoding.push_back(
              hdhC::stripSides( s0.substr(++pos)) );
          }
        }
        else if( s0[0] == '!' )
        {
          if(isDS)
            pathEncoding.back() += hdhC::stripSides( s0.substr(1)) ;
          else if(isFE)
            fileEncoding.back() += hdhC::stripSides( s0.substr(1)) ;
        }
        else
        {
          x_item = s0;

          if( x_item.size() > 1 )
          {
            cvMap[x_item[0]] = x_item[1];
            if( x_item.size() > 2 )
            {
              std::string s(x_item[0]+"_constr");
              cvMap[s] = x_item[2];
            }
          }
          else if( x_item.size() )
            cvMap[x_item[0]] = "*";
        }
      }

      continue;
    }

    if( s0.substr(0,11) == "BEGIN_LINE:" )
    {
      section.push_back( hdhC::stripSides( s0.substr(11) ) ) ;
      line.push_back(std::vector<std::string>());

      while( ! ifs.getLine(s0) )
      {
        if( s0.substr(0,9) == "END_LINE:" )
          break;
        else if( s0[0] == '!' )
          line.back().back() += s0.substr(1) ;
        else
          line.back().push_back(s0);
      }

      continue;
    }

    // variable section
    size_t last = s0.size() - 1;
    if( s0[last] == ':' )
    {
      // new variable
      // special ':' is equivalent to 'global:'
      if( s0[0] == ':' )
        s0 = "global:";

      varName.push_back(s0.substr(0, last));
      attName.push_back( std::vector<std::string>() );
      attValue.push_back( std::vector<std::string>() );
      continue;
    }

    // attributes
    if( attName.size() == 0 )
    {
      std::string key("7_2");
      if( notes->inq( key) )
      {
        std::string capt(
           "Syntax fault in the DRS_CV table: orphaned attribute, found ");
        capt += s0 ;

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
      }

      break;
    }

    x_item = s0;
    size_t sz = x_item.size() ;

    if( sz )
    {
      attName.back().push_back(x_item[0]);

      if( sz > 1 )
        attValue.back().push_back(x_item[1]);
      else
        attValue.back().push_back(hdhC::empty);
    }
  }

  ifs.close();

  return;
}

void
DRS_CV_Table::setParent(QA* p)
{
   pQA = p;
   notes = p->notes;
   return;
}

