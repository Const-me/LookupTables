#include "stdafx.h"
#include "../Test/SinCos/SinCosLinInt.hpp"

class Stat
{
	double m_sum;
	double m_max;
	int m_count;

public:
	Stat() : m_sum( 0 ), m_max( 0 ), m_count( 0 ) {}

	void add( double v )
	{
		v = abs( v );
		m_count++;
		m_sum += v;
		m_max = std::max( m_max, v );
	}

	void print( const char* title )
	{
		const int digs = DBL_DECIMAL_DIG;
		printf( "%s\t%.*e\t%.*e\n",
			title,
			digs, m_sum / m_count,
			digs, m_max );
	}
};

int main()
{
	using namespace DirectX;
	const int nTests = 1000;

	Stat sinHi, sinLo, cosHi, cosLo, sinLinInt, cosLinInt;

	const LinInt linInt;

	for( int i = 0; i < nTests; i++ )
	{
		float angle = float( M_PI * i / double( nTests ) );

		float sinPrecise = sinf( angle );
		float cosPrecise = cosf( angle );

		float s, c;
		XMScalarSinCos( &s, &c, angle );
		sinHi.add( s - sinPrecise );
		cosHi.add( c - cosPrecise );

		XMScalarSinCosEst( &s, &c, angle );
		sinLo.add( s - sinPrecise );
		cosLo.add( c - cosPrecise );

		linInt.sinCos( angle, s, c );
		sinLinInt.add( s - sinPrecise );
		cosLinInt.add( c - cosPrecise );
	}

	sinHi.print( "High-degree sine" );
	cosHi.print( "High-degree cosine" );
	sinLo.print( "Low-degree sine" );
	cosLo.print( "Low-degree cosine" );
	sinLinInt.print( "LinInt sine" );
	cosLinInt.print( "LinInt cosine" );

	return 0;
}