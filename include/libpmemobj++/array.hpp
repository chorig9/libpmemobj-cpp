/*
 * Copyright 2018, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * Array container based on std::array
 */

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <algorithm>

#include "libpmemobj++/detail/common.hpp"
#include "libpmemobj++/persistent_ptr.hpp"
#include "libpmemobj++/pext.hpp"
#include "libpmemobj++/span.hpp"
#include "libpmemobj.h"

namespace pmem
{

namespace obj
{

/**
 * pmem::obj::array - persistent container based on std::array
 *
 * Implements all methods implemented by std::array and additional
 * 'range' method for accessing specific part of the array.
 *
 * All methods which allow write access to specific element add this element
 * to a transaction.
 * All methods which return non-const iterator or non-const pointer to raw data
 * add entire array to a transaction.
 */
template <typename T, std::size_t N>
struct array {
	using value_type = T;
	using pointer = value_type*;
	using const_pointer = const pointer;
	using reference = value_type&;
	using const_referenece = const value_type&;
	using iterator = value_type*;
	using const_iterator = const iterator;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	/* Support for zero-sized arrays */
	value_type elems[N ? N : 1];

	array() = default;

	// Declaring this constructor means that array is no longer aggeragete type
	//array(array &&other)
	//{
	//	conditional_add_to_tx(&other);
	//	std::move(other.begin(), other.end(), begin());
	//}

	array &
	operator=(const array &other)
	{
		std::copy(other.cbegin(), other.cend(), begin());
		return *this;
	}

	array &
	operator=(const std::array<T, N> &other)
	{
		std::copy(other.cbegin(), other.cend(), begin());
		return *this;
	}

	array &
	operator=(array &&other)
	{
		std::move(other.begin(), other.end(), begin());
		return *this;
	}

	array &
	operator=(std::array<T, N> &&other)
	{
		std::move(other.begin(), other.end(), begin());
		return *this;
	}

	reference
	at(size_type n)
	{
		if (n >= N)
			throw std::out_of_range("array::at");

		detail::conditional_add_to_tx(elems + n);

		return elems[n];
	}

	constexpr const_reference
	at(size_type n) const
	{
		if (n >= N)
			throw std::out_of_range("array::at");

		return elems[n];
	}

	reference operator[](size_type n)
	{
		return at(n);
	}

	constexpr const_reference operator[](size_type n) const
	{
		return at(n);
	}

	T*
	data()
	{
		detail::conditional_add_to_tx(this);
		return elems;
	}

	constexpr const T*
	data() const noexcept
	{
		return elems;
	}

	iterator
	begin()
	{
		detail::conditional_add_to_tx(this);
		return elems;
	}

	iterator
	end()
	{
		detail::conditional_add_to_tx(this);
		return elems + size();
	}

	constexpr const_iterator
	begin() const noexcept
	{
		return elems;
	}

	constexpr const_iterator
	cbegin() const noexcept
	{
		return elems;
	}

	constexpr const_iterator
	end() const noexcept
	{
		return elems + size();
	}

	constexpr const_iterator
	cend() const noexcept
	{
		return elems + size();
	}

	reverse_iterator
	rbegin()
	{
		detail::conditional_add_to_tx(this);
		return elems + size();
	}

	reverse_iterator
	rend()
	{
		detail::conditional_add_to_tx(this);
		return elems;
	}

	constexpr const_reverse_iterator
	rbegin() const noexcept
	{
		return elems + size();
	}

	constexpr const_reverse_iterator
	crbegin() const noexcept
	{
		return elems + size();
	}

	constexpr const_reverse_iterator
	rend() const noexcept
	{
		return elems;
	}

	constexpr const_reverse_iterator
	crend() const noexcept
	{
		return elems;
	}

	reference
	front()
	{
		detail::conditional_add_to_tx(&elems[0]);
		return elems[0];
	}

	reference
	back()
	{
		detail::conditional_add_to_tx(&elems[size() - 1]);
		return elems[size() - 1];
	}

	constexpr const_reference
	front() const
	{
		return elems[0];
	}

	constexpr const_reference
	back() const
	{
		return elems[size() - 1];
	}

	/**
	 * Adds requested range to a transaction
	 * and returns span over this range
	 *
	 * @param start start index of requested range
	 * @param n number of elements in range
	 * @return span on requested range
	 */
	span<value_type, iterator>
	range(size_type start, size_type n)
	{
		if (start + n >= N)
			throw std::out_of_range("array::range");

		detail::conditional_add_range_to_tx(elems + start, n);

		return {elems + start, elems + start + n};
	}

	constexpr const span<value_type, iterator>
	range(size_type start, size_type n) const
	{
		if (start + n >= N)
			throw std::out_of_range("array::range");

		return {elems + start, elems + start + n};
	}

	constexpr const span<value_type, iterator>
	crange(size_type start, size_type n) const
	{
		if (start + n >= N)
			throw std::out_of_range("array::range");

		return {elems + start, elems + start + n};
	}

	constexpr size_type
	size() const
	{
		return N;
	}

	constexpr size_type
	max_size() const
	{
		return N;
	}

	constexpr bool
	empty() const
	{
		return size() != 0;
	}

	void
	fill(const_reference value)
	{
		// call to begin() will add array to a transaction
		std::fill(begin(), size(), value);
	}

	void
	swap(array &other)
	{
		// calls to begin() will add both arrays to a transaction
		std::swap_ranges(begin(), end(), other.begin());
	}
};

template <typename T, std::size_t N>
inline bool
operator==(const array<T, N> &lhs, const array<T, N> &rhs)
{
	return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template <typename T, std::size_t N>
inline bool
operator!=(const array<T, N> &lhs, const array<T, N> &rhs)
{
	return !(lhs == rhs);
}

template <typename T, std::size_t N>
inline bool
operator<(const array<T, N> &lhs, const array<T, N> &rhs)
{
	return std::lexicographical_compare(lhs.cbegin(), lhs.cend(),
					    rhs.cbegin(), rhs.cend());
}

template <typename T, std::size_t N>
inline bool
operator>(const array<T, N> &lhs, const array<T, N> &rhs)
{
	return rhs < lhs;
}

template <typename T, std::size_t N>
inline bool
operator>=(const array<T, N> &lhs, const array<T, N> &rhs)
{
	return !(lhs < rhs);
}

template <typename T, std::size_t N>
inline bool
operator<=(const array<T, N> &lhs, const array<T, N> &rhs)
{
	return !(lhs > rhs);
}

} /* namespace obj */

} /* namespace pmem */

namespace std
{

template <typename T, size_t N>
inline void
swap(pmem::obj::array<T, N> &lhs, pmem::obj::array<T, N> &rhs)
{
	lhs.swap(rhs);
}

template <size_t I, typename T, size_t N>
T &
get(pmem::obj::array<T, N> &a)
{
	static_assert(I < N,
		      "Index out of bounds in std::get<> (pmem::obj::array)");
	return a.at(I);
}

template <size_t I, typename T, size_t N>
T &&
get(pmem::obj::array<T, N> &&a)
{
	static_assert(I < N,
		      "Index out of bounds in std::get<> (pmem::obj::array)");
	return move(a.at(I));
}

template <size_t I, typename T, size_t N>
constexpr const T &
get(const pmem::obj::array<T, N> &a) noexcept
{
	static_assert(I < N,
		      "Index out of bounds in std::get<> (pmem::obj::array)");
	return a.at(I);
}

template <size_t I, typename T, size_t N>
constexpr const T &&
get(const pmem::obj::array<T, N> &&a) noexcept
{
	static_assert(I < N,
		      "Index out of bounds in std::get<> (pmem::obj::array)");
	return move(a.at(I));
}

// Tuple interface to class template array.
template <typename T, size_t N>
class tuple_size<pmem::obj::array<T, N>>
	: public std::integral_constant<size_t, N> {
};

template <size_t I, class T, size_t N>
struct tuple_element<I, pmem::obj::array<T, N>> {
	using type = T;
};

} /* namespace std */

#endif /* ARRAY_HPP */
