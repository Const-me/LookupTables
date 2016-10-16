#include "stdafx.h"
#include "align.h"

void* detail::allocate_aligned_memory( size_t align, size_t size ) noexcept
{
	assert( align >= sizeof( void* ) );
	if( size == 0 )
		return nullptr;
	return _aligned_malloc( size, align );
}

void detail::deallocate_aligned_memory( void *ptr ) noexcept
{
	return _aligned_free( ptr );
}