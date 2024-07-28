/*****************************************************************************
**
** common.h: an include file containing general type definitions, macros,
** and inline functions used in the other modules.
**
** Copyright (C) 1995 by Dani Lischinski 
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <assert.h>

#ifndef TRUE
#define TRUE 		1
#endif

#ifndef FALSE
#define FALSE 		0
#endif

#ifndef ON
#define ON		1
#define OFF		0
#endif

#define NIL(type) 	((type*)0)

#ifndef MAX
#define MAX(a, b) 	((a) >= (b) ? (a) : (b))
#define MIN(a, b) 	((a) <= (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)		((a) >= 0 ? (a) : -(a))
#endif

#define SGN(a) 		((a) > 0 ? 1 : ((a) < 0 ? -1 : 0))

#define forever		for(;;)

typedef int    Boolean;
typedef int    Switch;
typedef float  Sreal;
typedef double Lreal;
typedef char * charPtr;


inline int Max(int a, int b)
{
	return (a >= b) ? a : b;
}

inline int Min(int a, int b)
{
	return (a < b) ? a : b;
}

inline int Max(int a, int b, int c)
{
	return (a >= b) ? (a >= c ? a : c)
	                : (b >= c ? b : c);
}

inline int Min(int a, int b, int c)
{
	return (a < b) ? (a < c ? a : c)
	               : (b < c ? b : c);
}

inline float Max(float a, float b)
{
	return (a >= b) ? a : b;
}

inline float Min(float a, float b)
{
	return (a < b) ? a : b;
}

inline float Max(float a, float b, float c)
{
	return (a >= b) ? (a >= c ? a : c)
	                : (b >= c ? b : c);
}

inline float Min(float a, float b, float c)
{
	return (a < b) ? (a < c ? a : c)
	               : (b < c ? b : c);
}

inline double Max(double a, double b)
{
	return (a >= b) ? a : b;
}

inline double Min(double a, double b)
{
	return (a < b) ? a : b;
}

inline double Max(double a, double b, double c)
{
	return (a >= b) ? (a >= c ? a : c)
	                : (b >= c ? b : c);
}

inline double Min(double a, double b, double c)
{
	return (a < b) ? (a < c ? a : c)
	               : (b < c ? b : c);
}


inline int iMin(double a, double b, double c)
{
	return (a < b) ? (a < c ? 0 : 2)
	               : (b < c ? 1 : 2);
}

#include <stdlib.h>

#ifdef NDEBUG
#	define cdtAssert(e)	(void(0))
#	define cdtWarning(msg)	(void(0))
#	define cdtError(msg)	(void(0))
#else
#	define cdtAssert(e)	assert(e)
#	define cdtWarning(msg) \
		(cerr << "Warning: " << msg \
		      << ", file " << __FILE__ \
		      << ", line " << __LINE__ << "\n")
#	define cdtError(msg) \
		((cerr << "Error: " << msg \
		       << ", file " << __FILE__ \
		       << ", line " << __LINE__ << "\n"), exit(1))
#endif
#define cdtAbort(msg) \
		((cerr << "Fatal Error: " << msg \
		       << ", file " << __FILE__ \
		       << ", line " << __LINE__ << "\n"), abort())
	
#endif

