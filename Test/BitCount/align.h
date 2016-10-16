#pragma once
#include <vector>

// http://stackoverflow.com/a/12942652/126995

enum class Alignment: size_t
{
	Normal = sizeof( void* ),
	SSE = 16,
	AVX = 32,
};

namespace detail
{
	void* allocate_aligned_memory( size_t align, size_t size ) noexcept;
	void deallocate_aligned_memory( void* ptr ) noexcept;
}

template <typename T, Alignment Align = Alignment::SSE>
class AlignedAllocator;

template <Alignment Align>
class AlignedAllocator<void, Align>
{
public:
	typedef void*             pointer;
	typedef const void*       const_pointer;
	typedef void              value_type;

	template <class U> struct rebind { typedef AlignedAllocator<U, Align> other; };
};

template <typename T, Alignment Align>
class AlignedAllocator
{
public:
	typedef T         value_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;

	typedef std::true_type propagate_on_container_move_assignment;

	template <class U>
	struct rebind { typedef AlignedAllocator<U, Align> other; };

public:
	AlignedAllocator() noexcept
	{}

	template <class U>
	AlignedAllocator( const AlignedAllocator<U, Align>& ) noexcept
	{}

	size_type max_size() const noexcept
	{
		return ( size_type( ~0 ) - size_type( Align ) ) / sizeof( T );
	}

	pointer address( reference x ) const noexcept
	{
		return std::addressof( x );
	}

	const_pointer address( const_reference x ) const noexcept
	{
		return std::addressof( x );
	}

	pointer allocate( size_type n, typename AlignedAllocator<void, Align>::const_pointer = 0 )
	{
		const size_type alignment = static_cast<size_type>( Align );
		void* ptr = detail::allocate_aligned_memory( alignment, n * sizeof( T ) );
		if( ptr == nullptr ) {
			throw std::bad_alloc();
		}

		return reinterpret_cast<pointer>( ptr );
	}

	void deallocate( pointer p, size_type ) noexcept
	{
		return detail::deallocate_aligned_memory( p );
	}

	template <class U, class ...Args>
	void construct( U* p, Args&&... args )
	{
		::new( reinterpret_cast<void*>( p ) ) U( std::forward<Args>( args )... );
	}

	void destroy( pointer p )
	{
		p->~T();
	}
};

template <typename T, Alignment Align>
class AlignedAllocator<const T, Align>
{
public:
	typedef T         value_type;
	typedef const T*  pointer;
	typedef const T*  const_pointer;
	typedef const T&  reference;
	typedef const T&  const_reference;
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;

	typedef std::true_type propagate_on_container_move_assignment;

	template <class U>
	struct rebind { typedef AlignedAllocator<U, Align> other; };

public:
	AlignedAllocator() noexcept
	{}

	template <class U>
	AlignedAllocator( const AlignedAllocator<U, Align>& ) noexcept
	{}

	size_type max_size() const noexcept
	{
		return ( size_type( ~0 ) - size_type( Align ) ) / sizeof( T );
	}

	const_pointer address( const_reference x ) const noexcept
	{
		return std::addressof( x );
	}

	pointer allocate( size_type n, typename AlignedAllocator<void, Align>::const_pointer = 0 )
	{
		const size_type alignment = static_cast<size_type>( Align );
		void* ptr = detail::allocate_aligned_memory( alignment, n * sizeof( T ) );
		if( ptr == nullptr ) {
			throw std::bad_alloc();
		}

		return reinterpret_cast<pointer>( ptr );
	}

	void deallocate( pointer p, size_type ) noexcept
	{
		return detail::deallocate_aligned_memory( p );
	}

	template <class U, class ...Args>
	void construct( U* p, Args&&... args )
	{
		::new( reinterpret_cast<void*>( p ) ) U( std::forward<Args>( args )... );
	}

	void destroy( pointer p )
	{
		p->~T();
	}
};

template <typename T, Alignment TAlign, typename U, Alignment UAlign>
inline bool operator== ( const AlignedAllocator<T, TAlign>&, const AlignedAllocator<U, UAlign>& ) noexcept
{
	return TAlign == UAlign;
}

template <typename T, Alignment TAlign, typename U, Alignment UAlign>
inline bool operator!= ( const AlignedAllocator<T, TAlign>&, const AlignedAllocator<U, UAlign>& ) noexcept
{
	return TAlign != UAlign;
}

template<typename T>
using alignedVector = std::vector<T, AlignedAllocator<T, Alignment::SSE>>;

namespace detail
{
	static const Alignment default_alignment = Alignment::SSE;

	// template<typename T> T* allocate()
	template<typename T, class... _Types> T* allocate( _Types&&... _Args )
	{
		std::unique_ptr<T, void( *)( void* )> hold( static_cast<T*>( allocate_aligned_memory( default_alignment, sizeof( T ) ) ), deallocate_aligned_memory );
		::new ( hold.get() ) T( _STD forward<_Types>( _Args )... );
		return static_cast<T*>( hold.release() );
	}

	template<typename T>
	void deallocate( void* p )
	{
		static_cast<T*>( p )->~T();
		deallocate_aligned_memory( p );
	}
}