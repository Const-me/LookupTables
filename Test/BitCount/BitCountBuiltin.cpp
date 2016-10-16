#include "stdafx.h"
#include "BitCount.h"

namespace nsBitCnt
{
	// Calculate bits using POPCNT instruction.
	uint64_t testBuiltin( const alignedVector<uint16_t>& vec )
	{
#ifdef _MSC_VER
#define popcnt64 __popcnt64
#define popcnt32 __popcnt
#else
#define popcnt64 __popcntq
#define popcnt32 __popcntd
#endif

#if defined(_M_X64) || defined(__amd64__)
#define _pc popcnt64
		using tElt = uint64_t;
#else
#define _pc popcnt32
		using tElt = uint32_t;
#endif
		const tElt* p = (const tElt*)vec.data();
		const tElt* pEnd = p + ( vec.size() * sizeof( uint16_t ) / sizeof( tElt ) );

		uint64_t bits = _pc( *p );
		p++;
		while( p < pEnd )
		{
			bits += _pc( *p );
			p++;
		}
		return bits;
	}
}