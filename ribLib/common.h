#include <string>
using namespace std;

#define OS_MAX_PATH_LENGTH 1024
#define C_PI                                3.141592653589793238462643383279502884197169399375105820974944592308

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef WIN32
#define popen _popen
#define pclose _pclose
#include <windows.h>
#include <assert.h>
#include <malloc.h>
#endif

#include "variable.h"
