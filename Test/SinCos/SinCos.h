#pragma once

namespace nsSinCos
{
	enum struct eAlgo: uint8_t
	{
		StdLib,

		Lookup,

		PolyGTE,

#ifdef _WIN32
		PolyDX,
		PolyDxLow,
#endif
	};

	template<eAlgo algo>
	stopwatch::duration testSinCos();
}