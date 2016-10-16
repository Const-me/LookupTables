#pragma once
#include "align.h"

namespace nsBitCnt
{
	stopwatch::duration test( int nAlgo );

	uint64_t testLookup( const alignedVector<uint16_t>& vec );

	uint64_t testSse2( const alignedVector<uint16_t>& vec );
	uint64_t testSsse3( const alignedVector<uint16_t>& vec );
	uint64_t testXop( const alignedVector<uint16_t>& vec );
	uint64_t testBuiltin( const alignedVector<uint16_t>& vec );
}