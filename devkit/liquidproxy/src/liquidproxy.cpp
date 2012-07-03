#include <stdio.h>
#include <direct.h>
#include <process.h>
#include <windows.h>

#include <iostream>

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <strstream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>



using namespace boost;
using namespace std;

int main(int argc, char **argv)
{
  int returnCode = -1;
  
  if ( argc > 1 )
  {
    stringstream ss;
    
    for( unsigned int i = 1 ; i < (unsigned int)(argc) ; ++i ) 
      ss << argv[i] << " ";

    ss << ends;
    
//    printf ( "call cmd: %s\n", ss.str().c_str() );
    
    /*
    char **new_argv = argv + 1;
    int returnCode =  _spawnv( _P_WAIT, argv[1], new_argv );
    */
    returnCode =  system( ss.str().c_str() );
    
//   DWORD dw = GetLastError();
//   printf( "Return value = %d GetLastError = %d\n", returnCode, dw  );
  }

  return ( returnCode );
}
