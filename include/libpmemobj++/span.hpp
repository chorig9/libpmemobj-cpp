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

#ifndef SPAN_HPP
#define SPAN_HPP

namespace pmem
{

namespace obj
{

/**
 * pmem::obj::span - provides interface to access sequence of objects
 */
template <typename T, typename Iterator>
class span {
public:
	typedef std::size_t size_type;
	typedef T &reference;

	constexpr span(Iterator begin, Iterator end)
	    : it_begin(begin), it_end(end)
	{
	}

	span<T, Iterator> &
	operator=(const span<T, Iterator> &other) noexcept = default;

	constexpr Iterator
	begin() const noexcept
	{
		return it_begin;
	}

	constexpr const Iterator
	cbegin() const noexcept
	{
		return it_begin;
	}

	constexpr Iterator
	end() const noexcept
	{
		return it_end;
	}

	constexpr const Iterator
	cend() const noexcept
	{
		return it_end;
	}

	constexpr Iterator
	rend() const noexcept
	{
		return it_begin;
	}

	constexpr const Iterator
	crend() const noexcept
	{
		return it_begin;
	}

	constexpr Iterator
	rbegin() const noexcept
	{
		return it_end;
	}

	constexpr const Iterator
	crbegin() const noexcept
	{
		return it_end;
	}

	constexpr reference operator[](size_type idx) const
	{
		return *(it_begin + idx);
	}

	constexpr size_type
	size() const
	{
		return it_end - it_begin;
	}

private:
	Iterator it_begin, it_end;
};

} /* namespace obj */

} /* namespace pmem */

#endif /* SPAN_HPP */
