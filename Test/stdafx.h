#pragma once
#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <emmintrin.h>
#include <intrin.h>

#include <vector>
using std::vector;

#include <array>
using std::array;

#include <chrono>
typedef std::chrono::high_resolution_clock stopwatch;

#ifdef _WIN32
#include <DirectXMath.h>
#define  WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

inline uint32_t getTickCount()
{
#ifdef _WIN32
	return GetTickCount();
#else
	struct timespec ts;
	unsigned theTick = 0U;
	clock_gettime( CLOCK_REALTIME, &ts );
	theTick = ts.tv_nsec / 1000000;
	theTick += ts.tv_sec * 1000;
	return theTick;
#endif
}

inline void resetRand()
{
	srand( getTickCount() );
}