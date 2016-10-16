#include "stdafx.h"
#include "SinCos.h"
#include "BitCount/BitCount.h"

int result( stopwatch::duration dr )
{
	typedef std::chrono::duration<double, std::milli> ms;
	ms d = dr;
	printf( "%f\n", d.count() );
	return 0;
}

int sinCos( int argc, char** argv )
{
	int a = -1;
	if( argc > 0 )
		a = atoi( *argv );
	printf( "%i\t", a );

	using namespace nsSinCos;
	switch( a )
	{
	case 1:
		return result( testSinCos<eAlgo::StdLib>() );
	case 2:
		return result( testSinCos<eAlgo::Lookup>() );
	case 3:
		return result( testSinCos<eAlgo::PolyGTE>() );
	case 4:
		return result( testSinCos<eAlgo::PolyDX>() );
	case 5:
		return result( testSinCos<eAlgo::PolyDxLow>() );
	}

	printf( "Unknown algorithm.\nPossible values: 1 = C standard library, 2 = lookup table, 3 = GTEngine high-degree poly, 4 = DirectX high-degree poly, 5 = DirectX low-degree poly" );
	return -1;
}

int bitCnt( int argc, char** argv )
{
	if( argc <= 0 )
	{
		nsBitCnt::test( -1 );
		return 1;
	}

	int a = atoi( *argv );
	return result( nsBitCnt::test( a ) );
}

int main( int argc, char** argv )
{
	if( argc <= 1 || argc > 3 )
	{
		printf( "Usage: LookupTables <sincos|bitcount> <N>\n" );
		return 1;
	}

	if( 0 == _strcmpi( argv[ 1 ], "sincos" ) )
	{
		return sinCos( argc - 2, argv + 2 );
	}

	if( 0 == _strcmpi( argv[ 1 ], "bitcount" ) )
	{
		return bitCnt( argc - 2, argv + 2 );
	}


	printf( "E_NOTIMPL\n" );
	return 1;
}