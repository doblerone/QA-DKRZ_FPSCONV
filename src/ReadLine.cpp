#include "readline.h"

ReadLine::ReadLine()
   : isExternalStream(false)
{
  FStream = new std::ifstream ;

  init();
}

ReadLine::ReadLine( std::string file, bool testExistence )
   : isExternalStream(false)
{
  init();

  if( setFilename(file, testExistence) )
  {
    FStream = new std::ifstream ;
    FStream->open( inFile.c_str());
    feedStream();
  }
}

ReadLine::ReadLine( std::ifstream &is )
   : isExternalStream(true)
{
  init();
  FStream = &is ;
}

ReadLine::~ReadLine()
{
  disconnect_cin();  // save, even if not existing

  if( stream )
    delete stream;

  if( ! isExternalStream )
  {
    close() ; // closes the ifstream obj, if existing
    if( FStream->is_open() )
      delete FStream ;
  }

  return;
}

void
ReadLine::init()
{
  rangeFirst = 0. ;
  rangeLast = MAXDOUBLE ;
  rangeCol = 0  ;

  isClearSurroundingSpaces = false;
  isEof=false;
  isPutBackLine=false;
  isSkipComment = false;
  isSkipCharacter=false;
  isSkipWhiteLine = false;
  isRange = false ;
  isReadFloat=false;
  is_fopen=false;

  lineContinuation='\\';
  commentChar='#';

  stream=0;
}

void
ReadLine::close()
{
  if( FStream->is_open() )
  {
    FStream->close();
    is_fopen=false;
  }

  return;
}

void
ReadLine::connect_cin(void)
{
  save_sbuf_cin = std::cin.rdbuf();
//  std::streamsize sz=save_sbuf_cin->in_avail();
//  int n=save_sbuf_cin->sbumpc();

  stream = new std::istream( save_sbuf_cin);
  return ;
}

void
ReadLine::disconnect_cin(void)
{
  if( stream )
  {
    std::cin.rdbuf(save_sbuf_cin);
    stream->rdbuf(save_sbuf_stream);
    delete stream;
    stream=0;
  }

  return ;
}

bool
ReadLine::eof()
{
  // only if no line is currently put back is active
  if( ! isPutBackLine )
    isEof = stream->eof() ;

  return isEof ;
}

void
ReadLine::feedStream()
{
  if( FStream->is_open() )
  {
    save_sbuf_fin = FStream->rdbuf();

    stream = new std::istream( 0 );
    save_sbuf_stream = stream->rdbuf();
    stream->rdbuf( save_sbuf_fin );

    is_fopen=true;
  }

  return;
}

bool
ReadLine::findLine( std::string &rline, std::string &str, int opos )
{
  bool is;

  size_t i=0;
  if( opos > -1 )
    i = static_cast<size_t>( opos );

  size_t sz=str.size();

  while( ! (is=readLine()) )
  {
     if( opos < 0 )
     {
       if( line.find(str) < std::string::npos )
       {
          rline=line;
          return is;
       }
     }
     else
     {
       if( line.substr(i, sz) == str )
       {
          rline=line;
          return is;
       }
     }
  }

  return is;
}

std::vector<std::string>
ReadLine::getItems( size_t beg, size_t end )
{
   if( end == 0 )
     end = split.size();

   std::vector<std::string> tmp ;
   for( size_t i=beg ; i < end ; ++i )
     tmp.push_back( split[i]) ;

   return tmp ;
}

bool
ReadLine::getLine( std::string &str0)
{
  bool bret;
  if( (bret=readLine()) )
    return bret;

  if( isSkipWhiteLine )
    if( hdhC::isWhiteString( line ) )
      return getLine(str0) ; // safe for empty str0

  str0=line;

  if( isClearSurroundingSpaces )
     str0 = hdhC::stripSides( str0 );

  return bret;
}

std::string
ReadLine::getPreviousLine( void )
{
   return prevLine ;
}

std::vector<double>
ReadLine::getValues( void )
{
   if( ! isReadFloat )
   {
     val.clear();

     for( size_t i=0 ; i < split.size() ; i++ )
       val.push_back( split.toDouble( i ) );
   }

   return val ;
}

double
ReadLine::getValue( size_t index )
{
   if( isReadFloat )
     return val[index] ;
   else
     return split.toDouble( index ) ;
}

bool
ReadLine::getValue( size_t index, double &v )
{
   if( isReadFloat )
     return val[index] ;
   else
     return split.toDouble( index, v ) ;
}

bool
ReadLine::open( void )
{
  if( inFile.size() )
  {
    FStream = new std::ifstream(inFile.c_str()) ;

    if( FStream->is_open() )
    {
      stream = new std::istream( FStream->rdbuf() );
      isReadFloat=false;
      val.clear();
      return (is_fopen=true);
    }
  }
  else
    stream = new std::istream( std::cin.rdbuf() );

  //method open():  returns true for failure
  return (is_fopen=false) ;
}

bool
ReadLine::open( std::string f )
{
  if(is_fopen)
    return false;

  if( setFilename(f) )
    return open() ;

  return (is_fopen=false) ;
}

int
ReadLine::peek(void)
{
  if( isPutBackLine )
    return line[0];
  else
    return stream->peek();
}

void
ReadLine::putBackLine(void)
{
   swapLine();

   isPutBackLine=true;
   isSwappedEof=isEof;
   isEof=false;
}

bool
ReadLine::readLine(void)
{
  bool is=readLine(true) ;

  if( line.size() )
    split = line;
  else
    split.clear();

  return is;
}

bool
ReadLine::readLine(bool isVoid)
{
  // return: true for inStrem->eof()

  if( isPutBackLine )
  {
    swapLine();  // de-swaps line an prevLine
    isPutBackLine=false;
    isEof=isSwappedEof;
    return false;
  }

  char cbuf ;

  prevLine = line;
  line.erase();
  bool isSkip;
  bool isLeading=false;
  if( vc_skipLeadingChar.size() )
    isLeading=true;

  // variable number of columns
  while( !(isEof = stream->eof()) )
  {
    cbuf = stream->get();

    if( cbuf == '\n' || cbuf == '\r' )
    {
      if( (cbuf = stream->peek()) == '\n' )
      {
        if( (isEof = stream->eof()) )
            return false;

          cbuf = stream->get();
      }

      return false ;  // Zeile erfolgreich gelesen
    }

    if(isLeading)
    {
      bool is=false;
      for( size_t i=0 ; i < vc_skipLeadingChar.size() ; ++i )
      {
        if( cbuf == vc_skipLeadingChar[i] )
        {
          is=true;
          break;
        }
      }

      if(is)
        continue;
      else
        isLeading=false;
    }

    // skip fom #-char to the end of the line
    if( isSkipComment && cbuf == commentChar )
      isSkip=true;

    if(isSkip)
      continue;

    if( isSkipCharacter )
    {
      bool is=false;
      for( size_t j=0 ; j < vSkipChars.size() ; ++j )
      {
        if( cbuf == vSkipChars[j] )
        {
          is=true;
          break;
        }
      }

      if( is )
        continue ;
    }

    if(cbuf == lineContinuation)
    {
      cbuf = stream->peek();
      if( cbuf == '\n' || cbuf == '\r')
      {
        cbuf = stream->get();

        // skip leading blanks and tabs
        while( !(isEof = stream->eof()) )
        {
            cbuf = stream->peek();
            if( cbuf == ' ' || cbuf == '\t')
              cbuf = stream->get();
            else
              break;

        }

        continue;
      }
    }

    line += cbuf ;  // line contains all white chars
  }

  if( (isEof=stream->eof()) )
    return true ;

  return false ;
}

bool
ReadLine::readFloat(void)
{
  bool is=readFloat(true);

  if( line.size() )
    split = line;
  else
    split.clear();

  return is;
}

bool
ReadLine::readFloat(bool isVoid)
{
  // Initialisation
  if( ! isReadFloat )
  {
    // if the begin of a range was searched for, then there was already a readLine
    if( readLine() )
      return true ;

    for( int i=0 ; i < noOfCols ; ++i )
      val.push_back( getValue(i) ) ;

    while( isRange && getValue(rangeCol) < rangeFirst   )
    {
      for( int i=0 ; i < noOfCols ; ++i )
        *stream >> val[i] ;

      if( (isEof=stream->eof()) )
          return true ;

    }

    isReadFloat = true ;  //must be after the for-loop !!

    return false ;
  }

  // return: true for stream->eof()

  for( int i=0 ; i < noOfCols ; ++i )
    *stream >> val[i] ;

  if( (isEof=stream->eof()) )
      return true ;

  // user defined stop of reading
  if( isRange && rangeLast < getValue(rangeCol) )
    return true  ;

  return false ;
}

void
ReadLine::rewind( void )
{
  stream->clear();
  stream->seekg(std::ifstream::beg);
  return;
}

bool
ReadLine::setFilename( std::string f, bool testExistence )
{
  if( f.size() > 0 )
  {
    if( testExistence )
    {
      std::string testFile("/bin/bash -c \'test -f " + f + "\'") ;

      // see 'man system' for the return value, here we expect 0,
      // if the file exists.
      if( system( testFile.c_str()) )
        return false;
    }

    inFile=f;
  }

  return true ;
}

bool
ReadLine::setRange( double wave0, double wave1, size_t col )
{
  rangeFirst = wave0 ;
  rangeLast  = wave1 ;
  rangeCol   = col   ;
  isRange    = true;

  return true ;
}

bool
ReadLine::setRange( std::string & s )
{
   isRange = true;

   rangeFirst = hdhC::string2Double(s,1) ;

   double aa;

   if( hdhC::string2Double(s, 2, &aa) ) // returned bool
     rangeLast = aa ;

   if( hdhC::string2Double(s, 3, &aa) )  // returned bool
     rangeCol = (int)aa - 1 ;

   return true ;
}

void
ReadLine::skipCharacter(std::string s)
{
  for( size_t i=0 ; i < s.size() ; ++i )
    vSkipChars.push_back(s[i]);

  isSkipCharacter=true;

  return;
}

void
ReadLine::skipComment(char c)
{
  commentChar=c;
  isSkipComment=true;
  return;
}

//! Skip leading character(s)
void
ReadLine::skipLeadingChar(char c)
{
  vc_skipLeadingChar.clear();

  // white chars by default
  if( c == '\0' )
  {
    vc_skipLeadingChar.push_back(' ');
    vc_skipLeadingChar.push_back('\t');
  }
  else
    vc_skipLeadingChar.push_back(c);

  return;
}

void
ReadLine::skipLeadingChar(std::vector<char>& vc)
{
  vc_skipLeadingChar = vc;
  return;
}

bool
ReadLine::skipLines( int count )
{
   for( int i=0 ; i < count ; i++)
   {
     if( readLine() )
       return true;  // EOF
   }

   return false ;
}

void
ReadLine::swapLine(void)
{
   line.swap(prevLine);

   split = line ;
   return;
}
