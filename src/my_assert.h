#ifndef _MY_ASSERT_
#define _MY_ASSERT_

#include <stdio.h>

void _ERROR(const char* format, ... );

#if (defined DEBUG || defined _DEBUG)
#define my_assert(exp)    \
    if (!(exp)) { \
		_ERROR("Assert: %s File: %s Line: %s", exp, __FILE__, __LINE__ ); \
    }
#else
#define my_assert(exp)
#endif

#endif
