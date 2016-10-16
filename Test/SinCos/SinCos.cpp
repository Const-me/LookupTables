#include "stdafx.h"
#include "SinCos.h"
#include <Mathematics/GteSinEstimate.h>
#include <Mathematics/GteCosEstimate.h>
#include "SinCosLinInt.hpp"

using namespace nsSinCos;

struct StdLib
{
	inline void sinCos( float val, float& s, float& c ) const
	{
		s = sinf( val );
		c = cosf( val );
	}
};


#ifdef _WIN32
struct DX
{
	inline void sinCos( float val, float& s, float& c ) const
	{
		DirectX::XMScalarSinCos( &s, &c, val );
	}
};
struct DxLow
{
	inline void sinCos( float val, float& s, float& c ) const
	{
		DirectX::XMScalarSinCosEst( &s, &c, val );
	}
};
#endif

struct Gte
{
	inline void sinCos( float val, float& s, float& c ) const
	{
		s = gte::SinEstimate<float>::DegreeRR<11>( val );
		c = gte::CosEstimate<float>::DegreeRR<10>( val );
	}
};


template<class Algo>
stopwatch::duration testImpl( const vector<float>& src, const Algo& algo )
{
	float rs = 0, rc = 0;
	float s, c;
	auto start = stopwatch::now();
	for( float i : src )
	{
		algo.sinCos( i, s, c );
		rs += s;
		rc += c;
	}
	auto stop = stopwatch::now();
	auto duration = stop - start;

	const double avgSin = double( rs ) / double( src.size() );
	const double avgCos = double( rc ) / double( src.size() );
	printf( "%f / %f\t", avgSin, avgCos );
	return duration;
}

template<class algo>
static stopwatch::duration testImpl()
{
	resetRand();

	// static const size_t testSize = 10 * 1000;
	static const size_t testSize = 10 * 1000 * 1000;

	vector<float> src;
	src.resize( testSize );
	for( float& f : src )
		f = float( 2.0 * rand() * M_PI / RAND_MAX );

	// const StdLib algo;
	// const DX algo;
	const algo algo;
	return testImpl( src, algo );
}

namespace nsSinCos
{
	template<>
	stopwatch::duration testSinCos<eAlgo::StdLib>()
	{
		return testImpl<StdLib>();
	}

	template<>
	stopwatch::duration testSinCos<eAlgo::Lookup>()
	{
		return testImpl<LinInt>();
	}

	template<>
	stopwatch::duration testSinCos<eAlgo::PolyGTE>()
	{
		return testImpl<Gte>();
	}

#ifdef _WIN32
	template<>
	stopwatch::duration testSinCos<eAlgo::PolyDX>()
	{
		return testImpl<DX>();
	}

	template<>
	stopwatch::duration testSinCos<eAlgo::PolyDxLow>()
	{
		return testImpl<DxLow>();
	}	
#endif
}