#ifndef _MY_ASSERT_
#define _MY_ASSERT_

#include <vcl.h>
#include <stdio.h>
#include "Unit1.h"

void _ERROR(const char* format, ... );

#ifndef NDEBUG
#define my_assert(exp)    \
    if (!(exp)) { \
		_ERROR("Assert: %s File: %s\n   SrcFile: %s Line: %d", #exp, CurrentFileName.c_str(), __FILE__, __LINE__ ); \
    }
#else
#define my_assert(exp)
#endif

#endif
