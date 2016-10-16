#include "stdafx.h"
#include "BitCount.h"

namespace nsBitCnt
{
	static const __m128i popcount_mask1 = _mm_set1_epi8( 0x77 );
	static const __m128i popcount_mask2 = _mm_set1_epi8( 0x0F );
	static inline __m128i popcnt8_sse2( __m128i x )
	{
		__m128i n;
		// Count bits in each 4-bit field.
		n = _mm_srli_epi64( x, 1 );
		n = _mm_and_si128( popcount_mask1, n );
		x = _mm_sub_epi8( x, n );
		n = _mm_srli_epi64( n, 1 );
		n = _mm_and_si128( popcount_mask1, n );
		x = _mm_sub_epi8( x, n );
		n = _mm_srli_epi64( n, 1 );
		n = _mm_and_si128( popcount_mask1, n );
		x = _mm_sub_epi8( x, n );
		x = _mm_add_epi8( x, _mm_srli_epi16( x, 4 ) );
		x = _mm_and_si128( popcount_mask2, x );
		return x;
	}

	static inline __m128i popcnt8_ssse3( __m128i n )
	{
		static const __m128i popcount_mask = _mm_set1_epi8( 0x0F );
		static const __m128i popcount_table = _mm_setr_epi8( 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 );

		const __m128i pcnt0 = _mm_shuffle_epi8( popcount_table, _mm_and_si128( n, popcount_mask ) );
		const __m128i pcnt1 = _mm_shuffle_epi8( popcount_table, _mm_and_si128( _mm_srli_epi16( n, 4 ), popcount_mask ) );
		return _mm_add_epi8( pcnt0, pcnt1 );
	}

	static inline __m128i popcount8_xop( __m128i n )
	{
		static const __m128i popcount_mask = _mm_set1_epi8( 0x0F );
		static const __m128i popcount_table = _mm_setr_epi8( 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 );
		static const __m128i popcount_shift = _mm_set1_epi8( -4 );

		const __m128i pcnt0 = _mm_perm_epi8( popcount_table, popcount_table, _mm_and_si128( n, popcount_mask ) );
		const __m128i pcnt1 = _mm_perm_epi8( popcount_table, popcount_table, _mm_shl_epi8( n, popcount_shift ) );
		return _mm_add_epi8( pcnt0, pcnt1 );
	}

	template<class pcnt8>
	static inline uint64_t countBitsImpl( const alignedVector<uint16_t>& vec, pcnt8 pc8 )
	{
		const __m128i* p = ( const __m128i* )vec.data();
		const __m128i* pEnd = p + ( vec.size() * sizeof( uint16_t ) / sizeof( __m128i ) );

		const __m128i zero = _mm_setzero_si128();
		__m128i res = zero;
		for( ; p < pEnd; p++ )
		{
			const __m128i cnt8 = pc8( *p );
			const __m128i cnt64 = _mm_sad_epu8( cnt8, zero );
			res = _mm_add_epi64( res, cnt64 );
		}
		return res.m128i_u64[ 0 ] + res.m128i_u64[ 1 ];
	}

	uint64_t testSse2( const alignedVector<uint16_t>& vec )
	{
		return countBitsImpl( vec, &popcnt8_sse2 );
	}
	uint64_t testSsse3( const alignedVector<uint16_t>& vec )
	{
		return countBitsImpl( vec, &popcnt8_ssse3 );
	}
	uint64_t testXop( const alignedVector<uint16_t>& vec )
	{
		return countBitsImpl( vec, &popcount8_xop );
	}
}