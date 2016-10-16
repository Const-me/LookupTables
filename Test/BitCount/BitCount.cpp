#include "stdafx.h"
#include "BitCount.h"
#include "align.h"

// static const size_t szArr = 10000;
static const size_t szArr = 100000000;	// 100M elements, ~190 MB RAM

static_assert( 0 == ( szArr * sizeof( uint16_t ) % 16 ), "Invalid array length: must be a multiple of 16 bytes, because SSE" );

namespace nsBitCnt
{
	using fnTest = uint64_t( * )( const alignedVector<uint16_t>& );

	static inline stopwatch::duration measure( const alignedVector<uint16_t>& vec, fnTest test )
	{
		const auto start = stopwatch::now();
		const uint64_t bits = test( vec );
		const auto stop = stopwatch::now();
		printf( "%llu\t", bits );
		return stop - start;
	}

	static bool hasSsse3()
	{
		int regs[ 4 ];
		__cpuid( regs, 1 );
		const int ecx = regs[ 2 ];
		return 0 != ( ecx & ( 1 << 9 ) );
	}

	static bool hasXop()
	{
		int regs[ 4 ];
		__cpuid( regs, 0 );

		static const int idAmd[ 3 ] =
		{
			0x68747541, 0x444d4163, 0x69746e65
		};

		if( 0 != memcmp( regs + 1, idAmd, 12 ) )
			return false;

		__cpuid( regs, 0x80000001 );
		const int ecx = regs[ 2 ];
		return 0 != ( ecx & ( 1 << 11 ) );
	}

	static bool hasBuiltin()
	{
		int regs[ 4 ];
		__cpuid( regs, 1 );
		const int ecx = regs[ 2 ];
		return 0 != ( ecx & ( 1 << 23 ) );
	}

	static inline fnTest testFunc( int nAlgo )
	{
		switch( nAlgo )
		{
		case 1:
			return &testLookup;
		case 2:
			return &testSse2;
		case 3:
			if( !hasSsse3() )
			{
				printf( "This CPU doesn't support SSSE3 instruction set." );
				return nullptr;
			}
			return &testSsse3;
		case 4:
			if( !hasXop() )
			{
				printf( "This CPU doesn't support XOP instruction set." );
				return nullptr;
			}
			return &testXop;
		case 5:
			if( !hasBuiltin() )
			{
				printf( "This CPU doesn't support POPCNT instruction." );
				return nullptr;
			}
			return &testBuiltin;
		default:
			break;
		}
		printf( "Unknown algorithm.\nPossible values: 1 = lookup table, 2 = SSE2, 3 = SSSE3, 4 = XOP, 5 = POPCNT\n" );
		return nullptr;
	}

	stopwatch::duration test( int nAlgo )
	{
		printf( "%i\t", nAlgo );

		const fnTest fn = testFunc( nAlgo );
		if( nullptr == fn )
			return stopwatch::duration();

		alignedVector<uint16_t> vec;
		vec.resize( szArr );

		resetRand();
		for( uint16_t& i : vec )
			i = rand();

		return measure( vec, fn );
	}
}