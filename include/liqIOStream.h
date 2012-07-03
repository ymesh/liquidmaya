#ifndef liqIOStream_H
#define liqIOStream_H

// This file is intended to be the only std io header file included by any Liquid 
// files. This is to get around problems between Maya versions, platforms and
// compilers, some of which try and help by auto-including or auto-namespacing,
// and others do not

// NOTE: as of Maya 7.0 it seems you need to put this include before any Maya 
//       includes, or things can get confused (you'll get errors like "introduced 
//       ambiguous type `iostream'")


#if MAYA_API_VERSION < 500
  #include <iostream>
    using std::cout;
    using std::cerr;
    using std::endl;
    using std::flush;
#else
  #include <maya/MIOStream.h>
#endif


#endif // liqIOStream_H
