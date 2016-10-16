#pragma once

#define _PI        (float)( M_PI )
#define _2PI       (float)( M_PI * 2 )
#define _PIDIV2    (float)( M_PI / 2 )
#define _1DIV2PI   (float)( 1.0 / ( M_PI * 2 ) )

class LinInt
{
	static const size_t size = 256;
	// Two tables for sin and cos, from -PI/2 to +PI/2, interleaved for cache friendliness
	std::array<float, ( size + 1 ) * 2> lookupTable;
	const float indexMul;

	// value should be from -PI/2 to +PI/2
	inline void lookup( float value, float& sin, float& cos ) const
	{
		// Calculate index + coefficients for linear interpolation
		value *= indexMul;
		int i1 = int( floor( value ) );
		value -= i1;
		i1 += ( size / 2 );
		const float b = 1.0f - value;

		// Interpolate both sin + cos using same coefficients.
		const float* entries = lookupTable.data() + ( i1 << 1 );
		sin = entries[ 0 ] * b + entries[ 2 ] * value;
		cos = entries[ 1 ] * b + entries[ 3 ] * value;
	}
public:
	LinInt() : indexMul( float( size / M_PI ) )
	{
		for( int i = 0; i <= size; i++ )
		{
			double val = M_PI * ( ( i - ( size / 2 ) ) / double( size ) );
			lookupTable[ i * 2 ] = float( sin( val ) );
			lookupTable[ i * 2 + 1 ] = float( cos( val ) );
		}
	}

	inline void sinCos( float Value, float& sin, float& cos ) const
	{
		// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		float quotient = _1DIV2PI * Value;
		if( Value >= 0.0f )
		{
			quotient = (float)( (int)( quotient + 0.5f ) );
		}
		else
		{
			quotient = (float)( (int)( quotient - 0.5f ) );
		}
		float y = Value - _2PI * quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
		float sign;
		if( y > _PIDIV2 )
		{
			y = _PI - y;
			sign = -1.0f;
		}
		else if( y < -_PIDIV2 )
		{
			y = -_PI - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}
		// Interpolate both
		float cosNoSign;
		lookup( y, sin, cosNoSign );
		// Restore cos sign
		cos = cosNoSign * sign;
	}
};