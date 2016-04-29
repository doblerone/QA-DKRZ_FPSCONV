#include "base.h"

Base::Base()
{
  initDefaults();
}

Base::Base(const Base &b)
{
  initDefaults();
  copy(b);
}

Base&
Base::operator=( const Base &b)
{
  if( this != &b )
    copy(b);

  return *this;
}

void
Base::applyOptions()
{
   // options only applicable to Base properties

   Split split;
   split.setSeparator("=");

   // now we apply to the varNames found
   for( size_t j=0 ; j < optStr.size() ; ++j)
   {
     split = optStr[j];
     if( split.size() == 2 )
     {
       if( isVarname( split[0] ) )
       {
         if( getObjName() == "IN" )
           continue;
         else
         {
           Split list(split[1], ",");
           for( size_t i=0 ; i < list.size() ; i++)
             varName.push_back( list[i] );
         }
       }

       if( split[0][0] == 'f' )
       {
         setFilename( split[1] );

         continue;
       }

       if( split[0] == "filling_value" ||
              split[0] == "fillingValue" || split[0] == "fV"
                  || split[0] == "_FillValue" )
       {
         isMissValSet=true;
         Split spltFV(split[1],",");
         for( size_t i=0 ; i < spltFV.size() ; ++i)
           fillingValue.push_back( spltFV.toDouble(i) ) ;
         continue;
       }

     }

     // skip ident number
     if( split.isNumber(0) )
        continue;

     // these are applied to each GD obj
//     for( size_t i=0 ; i < pGM.size() ; ++i)
/*
     {
       if( split[0] == "back_rotation"
            || split[0] == "backRotation")
       {
         isBackRotation=true;
         pGM[i]->setBackRotation();
         continue;
       }
*/

/*
       if( split[0] == "gmt_multi_segment"
            || split[0] == "gmtMultiSegment" )
       {
         pGM[i]->setOutputGMT_MultiSegment();
         continue;
       }

       if( split[0] == "oF" )
       {
         pGM[i]->setPrintFilename(split[1]);
         continue;
       }

       if( split[0] == "print_cells" || split[0] == "printCells" )
       {
         pGM[i]->setPrintWithGeoCoord();
         continue;
       }

       if( split[0].substr(0,3) == "reg"
            || split[0].substr(0,3) == "Reg" )
       {
         regionFile = split[1];
         continue;
       }
*/
//     }
   }

   return;
}

void
Base::clearVariable( Variable &v )
{
  v.name.clear();
  v.id=-1;
  v.pGM=0;
  v.pDS=0;
  v.pSrcBase=0;
  v.pIn=0;
  v.pNc=0;
  return;
}

void
Base::copy(const Base &b)
{
  objName = b.objName;
  thisID=b.thisID;
  file=b.file;

  srcStr.clear();
  for( size_t i=0 ; i < b.srcStr.size() ; ++i)
    srcStr.push_back(b.srcStr[i]);

  for( size_t i=0 ; i<b.optStr.size() ; ++i )
    optStr.push_back( b.optStr[i] );

  //see rules for variable and srcName in attach()
/*
  for( size_t i=0 ; i<b.variable.size() ; ++i )
  {
    variable.push_back( *new Variable );
    variable[i].name= b.variable[i].name;
  }
*/
  for( size_t i=0 ; i<b.obsolete_srcVarName.size() ; ++i )
    obsolete_srcVarName.push_back( obsolete_srcVarName[i]);

  isBackRotation=b.isBackRotation;
  isMissValSet=b.isMissValSet;
  explicitFillingValue=b.explicitFillingValue;

  for( size_t i=0 ; i<b.regioStr.size() ; ++i )
    regioStr.push_back(b.regioStr[i]);

  // index mapped to varName for the next vectors
  for( size_t i=0 ; i<b.fillingValue.size() ; ++i )
    fillingValue.push_back( b.fillingValue[i]);

  // used to point to those source-gM(s) that will be used
  for( size_t i=0 ; i<b.pSrcGD.size() ; ++i )
    pSrcGD.push_back( b.pSrcGD[i] );

  notes=b.notes;
  cF=b.cF;
  pIn=b.pIn;
  fDI=b.fDI;
  pOper=b.pOper;
  pOut=b.pOut;
  qA=b.qA;
  tC=b.tC;

  // point to the source base
  pSrcBase=b.pSrcBase;
  pNc=b.pNc;  // only set in InFile objects

  return;
}

bool
Base::exceptionHandling(std::string key,
  std::string capt, std::string text,
  std::string vName )
{
   if( notes )
   {

//     bool sendEmail=false;
//     std::string table="*";
//     std::string task="L1";
//     notes->push_front(key , vName, sendEmail, table, "L1", "");

     if( ! notes->inq(key, vName ) )
       return false;  // if discarded

     notes->setCheckStatus("Base", "FAIL");
     return notes->operate(capt, text);
   }

   return false ;
}

void
Base::exceptionError(std::string str)
{
  // Occurrence of an error usually stops the run at once.
  // But, the calling program unit is due to exit.
  static bool doInit=true;
  if( doInit )
  {
    xcptn.strError = getObjName() ;
    xcptn.strError += "_error" ;

     // base-name if available, i.e. after initialisation of the InFile obj
    if( file.is )
    {
      xcptn.strError += "_";
      xcptn.strError += file.basename ;
    }
    xcptn.strError += ".txt";

    doInit = false;
  }

  // open file for writing
  if( ! xcptn.ofsError )
    xcptn.ofsError = new std::ofstream(xcptn.strError.c_str());

  *xcptn.ofsError << str << std::endl;

  return ;
}

void
Base::exceptionWarning(std::string str)
{
  // a warning does not stop the run

  static bool doInit=true;
  if( doInit )
  {
    // This happens only once. All error and warning
    // calls on the global scale and in any class-related
    // objects refer to this.

    xcptn.strWarning = getObjName() ;
    xcptn.strWarning += "_warning" ;

    if( file.is )
    {
      xcptn.strWarning += "_";
      xcptn.strWarning += file.basename ;
    }
    xcptn.strWarning += ".txt";

    doInit = false;
  }

  // open file for writing
  if( ! xcptn.ofsWarning )
    xcptn.ofsWarning = new std::ofstream(xcptn.strWarning.c_str());

  if( xcptn.ofsWarning->is_open() )
  {
    // write message
    xcptn.ofsWarning->write(str.c_str(), str.size()+1);
    *xcptn.ofsWarning << std::endl;
  }

  return ;
}

void
Base::finally(int errCode, std::string note)
{
  // print a message to std::cout. This will be further handled by
  // the calling entity.
  if( note.size() )
    std::cout << note << ";" << std::endl;

  // distinguish from a sytem crash (segmentation error)
  std::cout << "NormalExecution;" << std::endl;

  // in case that just a message is issued, but the program
  // is going to continue.
  if( errCode < 0)
    return;

  if( errCode > 0)
    exit(errCode);

  return;
}

std::vector<std::string>
Base::getVarname( std::string &s)
{
  // There might be a varname specification or not.
  // no varname found: return true

  std::vector<std::string> names;
  std::string list;

  std::vector<std::string> tokens;
  tokens.push_back( ":v=" ) ;
  tokens.push_back( ":varname=" ) ;
  tokens.push_back( ":vname=" ) ;
  tokens.push_back( ":variable" ) ;

  bool isEmpty=true;
  size_t pos0=0;
  for( size_t i=0 ; i < tokens.size() ; ++i )
  {
    if( (pos0 = s.find( tokens[i] ) ) < std::string::npos )
      // e.g.: s: "...:vname=...:..." and tokens: ":vname=..."
      isEmpty=false;
    else if( s.substr(0,tokens[i].size()-1) == tokens[i].substr(1) )
      // e.g.: s: "vname=..." and tokens: "vname=..."
      isEmpty=false;
  }

  if( isEmpty )
    return names;

  // the assignment sign
  size_t pos1=s.find("=",pos0);
  if( pos1 == std::string::npos )
  {
     std::ostringstream ostr(std::ios::app);
     ostr << "Base::getVarname(): invalid assignment";

     exceptionError( ostr.str() );
     std::string note("E8: no rules for finding a variable name.");
     finally(8, note);
  }
  else
    ++pos1 ;

  // the end of the assignment
  size_t pos2=s.find(":",pos1);
  if( pos2 == std::string::npos )
     // the end of the string
     pos2=s.size();

  list=s.substr(pos1, pos2-pos1);

  // expand the list
  Split spl(list, ",");
  for( size_t i=0 ; i < spl.size() ; ++i)
      names.push_back( spl[i] ) ;

  return names;
}

std::vector<std::string>
Base::getVarname( std::string &s, std::vector<std::string> &alias)
{
  // There might be a varname specification or not.
  // no varname found: return true
  // The reference 'alias' is special for operations,
  // e.g. v=x=var1,y=var2,... . If given, then var1 is the
  // variable returned via the vector and x,y,... are members
  // stored in alias.

  std::vector<std::string> list;
  std::vector<std::string> names;

  list = getVarname( s ) ;

  // expand the list
  for( size_t i=0 ; i < list.size() ; ++i)
  {
    Split spl(list[i], "=");

    if( spl.size() == 2 )
    {
      alias.push_back( spl[0] ) ;
      names.push_back( spl[1] ) ;
    }
    else if( spl.size() == 1 )
    {
      alias.push_back( "" ) ;
      names.push_back( spl[0] ) ;
    }
    else
    {
      std::ostringstream ostr(std::ios::app);
      ostr << "Base::getVarname(): invalid assignment\n";
      ostr << " of variable names." ;

      exceptionError( ostr.str() );
      std::string note("E9: invalid assignment of variable names.");
      finally(9, note);
    }
  }

  return names;
}

void
Base::help(void)
{
  std::cerr << "Description of options:\n" ;
  std::cerr << "Options for class Base applying to derived classes:\n" ;
  std::cerr << "CellStatistics, InFile, Oper, and OutFile.\n" ;
  std::cerr << "Option strings for these classes may enclose\n" ;
  std::cerr << "other option strings of class designators\n";
  std::cerr << "QA, TC, and FD, which may be embedded with\n" ;
  std::cerr << "e.g. QA@ where @ is any character not occurring\n";
  std::cerr << "in the enclosing string (the last may be omitted,\n" ;
  std::cerr << "but, may be repeated in another embedded string).\n\n";
  std::cerr << "Options: The []-part is optional, patterns like\n" ;
  std::cerr << " '..a_b..' may also be written as '..aB..','\n" ;
  std::cerr << " alternatives are indicated below by '|'\n" ;
  std::cerr << "   back_rotation\n" ;
  std::cerr << "   f[ilename]=string\n" ;
  std::cerr << "   gmt_multi_segment\n" ;
  std::cerr << "   filling_value=figure | mV=figure\n" ;
  std::cerr << "   oF=output-filename-in-gM\n" ;
  std::cerr << "   print_cells\n" ;
  std::cerr << std::endl;
  return;
}

void
Base::initDefaults(void)
{
  // pre-settings
  isAllocate=false;
  isArithmeticMean=false ;
  isBackRotation=false;
  isMissValSet=false;

  xcptn.ofsError=0;
  xcptn.ofsWarning=0;

  return;
}

bool
Base::isVarname( std::string &s)
{
  // There might be a varname specification or not.
  // no varname found: return true, if a varname
  // is specified

  if( s.find("v") < std::string::npos )
    return true ;
  if( s.find("varname") < std::string::npos )
    return true;

  if( s.find("vname") < std::string::npos )
    return true ;
  if( s.find("variable") < std::string::npos )
    return true;

  return false ;
}

/*
void
Base::setVarProps(void)
{
  // Establish relation between the gM_Unlimited variable name(s)
  // of the current object to those of the source(s). Settings
  // for an operation are indicated by operndStr.size() > 0.

  // An operation will create new or modify data.
  // No operation is effective for OutFile, where data remains untouched
  // but renaming of variables and/or selection of multiple variables
  // is feasible.

  if( operandStr.size() == 0 )
    setVarPropsNoOperation();
  else
    setVarPropsForOperation();

  // apply srcVariables to set variables of *this
  for( size_t i=0 ; i < varName.size() ; ++i )
  {
      if( i == variable.size() )
      {
        // new obj from the source
        variable.push_back( *new Variable) ;
        variable.back().pGM      = srcVariable[i].pGM ;
        variable.back().pDS      = srcVariable[i].pDS ;
      }
      else // connect the current new pGM into the stream of sources
      {
        variable.back().pGM      = 0 ;
        variable.back().pDS      = 0 ;
      }

      variable.back().name     = varName[i] ;
      variable.back().id       = srcVariable[i].id ;
      variable.back().pSrcBase = srcVariable[i].pSrcBase ;
      variable.back().pIn      = srcVariable[i].pIn ;
      variable.back().pNc      = srcVariable[i].pNc ;
  }

  return;
}
*/
