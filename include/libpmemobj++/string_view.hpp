// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/**
 * @file
 * Our partial std::string_view implementation.
 */

#ifndef LIBPMEMOBJ_CPP_STRING_VIEW
#define LIBPMEMOBJ_CPP_STRING_VIEW

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#if __cpp_lib_string_view
#include <string_view>
#endif

namespace pmem
{

namespace obj
{

#if __cpp_lib_string_view

template <typename CharT, typename Traits = std::char_traits<CharT>>
using basic_string_view = std::basic_string_view<CharT, Traits>;
using string_view = std::string_view;
using wstring_view = std::basic_string_view<wchar_t>;
using u16string_view = std::basic_string_view<char16_t>;
using u32string_view = std::basic_string_view<char32_t>;

#else

/**
 * Our partial std::string_view implementation.
 *
 * If C++17's std::string_view implementation is not available, this one
 * is used to avoid unnecessary string copying.
 */
template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_string_view {
public:
	/* Member types */
	using traits_type = Traits;
	using value_type = CharT;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type &;
	using const_reference = const value_type &;
	using pointer = value_type *;
	using const_pointer = const value_type *;
	using const_iterator = const_pointer;
	using iterator = const_iterator;
	using reverse_iterator = std::reverse_iterator<const_iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr const size_type npos =
		(std::numeric_limits<size_type>::max)();

	constexpr basic_string_view() noexcept;
	constexpr basic_string_view(const CharT *data, size_type size);
	constexpr basic_string_view(const std::basic_string<CharT, Traits> &s);
	constexpr basic_string_view(const CharT *data);

	constexpr basic_string_view(const basic_string_view &rhs) noexcept =
		default;
	basic_string_view &
	operator=(const basic_string_view &rhs) noexcept = default;

	constexpr const_iterator begin() const noexcept;
	constexpr const_iterator cbegin() const noexcept;
	constexpr const_iterator end() const noexcept;
	constexpr const_iterator cend() const noexcept;
	constexpr const_reverse_iterator rbegin() const noexcept;
	constexpr const_reverse_iterator crbegin() const noexcept;
	constexpr const_reverse_iterator rend() const noexcept;
	constexpr const_reverse_iterator crend() const noexcept;

	constexpr const CharT *data() const noexcept;
	constexpr size_type size() const noexcept;
	constexpr size_type length() const noexcept;
	constexpr bool empty() const noexcept;
	constexpr size_type max_size() const noexcept;

	const CharT &at(size_type pos) const;
	constexpr const CharT &operator[](size_type pos) const noexcept;
	constexpr const_reference front() const noexcept;
	constexpr const_reference back() const noexcept;

	void remove_prefix(size_type n);
	void remove_suffix(size_type n);
	void swap(basic_string_view &v) noexcept;

	constexpr basic_string_view substr(size_type pos = 0,
					   size_type count = npos) const;
	size_type copy(CharT *dest, size_type count, size_type pos = 0) const;
	inline int compare(size_type pos1, size_type n1,
			   basic_string_view sv) const;
	inline int compare(size_type pos1, size_type n1, basic_string_view sv,
			   size_type pos2, size_type n2) const;
	inline int compare(const CharT *s) const noexcept;
	inline int compare(size_type pos1, size_type n1, const CharT *s) const;
	inline int compare(size_type pos1, size_type n1, const CharT *s,
			   size_type n2) const;
	int compare(const basic_string_view &other) const noexcept;

private:
	const value_type *data_;
	size_type size_;
};

using string_view = basic_string_view<char>;
using wstring_view = basic_string_view<wchar_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

/**
 * Default constructor with empty data.
 */
template <typename CharT, typename Traits>
constexpr inline basic_string_view<CharT, Traits>::basic_string_view() noexcept
    : data_(nullptr), size_(0)
{
}

/**
 * Constructor initialized by *data* and its *size*.
 *
 * @param[in] data pointer to the C-like string to initialize with,
 *	it can contain null characters.
 * @param[in] size length of the given data.
 */
template <typename CharT, typename Traits>
constexpr inline basic_string_view<CharT, Traits>::basic_string_view(
	const CharT *data, size_type size)
    : data_(data), size_(size)
{
}

/**
 * Constructor initialized by the basic string *s*.
 *
 * @param[in] s reference to the string to initialize with.
 */
template <typename CharT, typename Traits>
constexpr inline basic_string_view<CharT, Traits>::basic_string_view(
	const std::basic_string<CharT, Traits> &s)
    : data_(s.c_str()), size_(s.size())
{
}

/**
 * Constructor initialized by *data*. Size of the data will be set
 * using Traits::length().
 *
 * @param[in] data pointer to C-like string (char *) to initialize with,
 *	it has to end with the terminating null character.
 */
template <typename CharT, typename Traits>
constexpr inline basic_string_view<CharT, Traits>::basic_string_view(
	const CharT *data)
    : data_(data), size_(Traits::length(data))
{
}

/**
 * Returns an iterator to the first character of the view.
 *
 * @return const_iterator to the first character
 */
template <typename CharT, typename Traits>
constexpr typename basic_string_view<CharT, Traits>::const_iterator
basic_string_view<CharT, Traits>::begin() const noexcept
{
	return cbegin();
}

/**
 * Returns an iterator to the first character of the view.
 *
 * @return const_iterator to the first character
 */
template <typename CharT, typename Traits>
constexpr typename basic_string_view<CharT, Traits>::const_iterator
basic_string_view<CharT, Traits>::cbegin() const noexcept
{
	return data_;
}

/**
 * Returns an iterator to the character following the last character of the
 * view. This character acts as a placeholder, attempting to access it results
 * in undefined behavior.
 *
 * @return const_iterator to the character following the last character.
 */
template <typename CharT, typename Traits>
constexpr typename basic_string_view<CharT, Traits>::const_iterator
basic_string_view<CharT, Traits>::end() const noexcept
{
	return cend();
}

/**
 * Returns an iterator to the character following the last character of the
 * view. This character acts as a placeholder, attempting to access it results
 * in undefined behavior.
 *
 * @return const_iterator to the character following the last character.
 */
template <typename CharT, typename Traits>
constexpr typename basic_string_view<CharT, Traits>::const_iterator
basic_string_view<CharT, Traits>::cend() const noexcept
{
	return data_ + size_;
}

template <typename CharT, typename Traits>
constexpr typename basic_string_view<CharT, Traits>::const_reverse_iterator
basic_string_view<CharT, Traits>::rbegin() const noexcept
{
	return reverse_iterator(cend());
}

template <typename CharT, typename Traits>
constexpr typename basic_string_view<CharT, Traits>::const_reverse_iterator
basic_string_view<CharT, Traits>::crbegin() const noexcept
{
	return reverse_iterator(cend());
}

template <typename CharT, typename Traits>
constexpr typename basic_string_view<CharT, Traits>::const_reverse_iterator
basic_string_view<CharT, Traits>::rend() const noexcept
{
	return reverse_iterator(cbegin());
}

template <typename CharT, typename Traits>
constexpr typename basic_string_view<CharT, Traits>::const_reverse_iterator
basic_string_view<CharT, Traits>::crend() const noexcept
{
	return reverse_iterator(cbegin());
}

/**
 * Returns pointer to data stored in this pmem::obj::string_view. It may
 * not contain the terminating null character.
 *
 * @return pointer to C-like string (char *), it may not end with null
 *	character.
 */
template <typename CharT, typename Traits>
constexpr inline const CharT *
basic_string_view<CharT, Traits>::data() const noexcept
{
	return data_;
}

/**
 * Returns that view is empty or not.
 *
 * @return true when size() == 0.
 */
template <typename CharT, typename Traits>
constexpr inline bool
basic_string_view<CharT, Traits>::empty() const noexcept
{
	return size() == 0;
}

/**
 * Returns the largest possible number of char-like objects that can be
 * referred to by a basic_string_view.
 *
 * @return maximum number of characters.
 */
template <typename CharT, typename Traits>
constexpr inline typename basic_string_view<CharT, Traits>::size_type
basic_string_view<CharT, Traits>::max_size() const noexcept
{
	return (std::numeric_limits<size_type>::max)();
}

/**
 * Returns count of characters stored in this pmem::obj::string_view
 * data.
 *
 * @return the number of CharT elements in the view.
 */
template <typename CharT, typename Traits>
constexpr inline typename basic_string_view<CharT, Traits>::size_type
basic_string_view<CharT, Traits>::size() const noexcept
{
	return size_;
}

/**
 * Returns count of characters stored in this pmem::obj::string_view data.
 *
 * @return the number of CharT elements in the view.
 */
template <typename CharT, typename Traits>
constexpr inline typename basic_string_view<CharT, Traits>::size_type
basic_string_view<CharT, Traits>::length() const noexcept
{
	return size_;
}

/**
 * Returns reference to a character at position @param[in] pos .
 *
 * @return reference to the char.
 */
template <typename CharT, typename Traits>
constexpr inline const CharT &
	basic_string_view<CharT, Traits>::operator[](size_t pos) const noexcept
{
	return data()[pos];
}

/**
 * Returns reference to the character at position @param[in] pos and
 * performs bound checking.
 *
 * @return reference to the char.
 *
 * @throw std::out_of_range when out of bounds occurs.
 */
template <typename CharT, typename Traits>
inline const CharT &
basic_string_view<CharT, Traits>::at(size_t pos) const
{
	if (pos >= size())
		throw std::out_of_range("Accessing a position out of bounds!");
	return data()[pos];
}

/**
 * Returns reference to the last character in the view.
 * The behavior is undefined if empty() == true.
 *
 * @return reference to the last character.
 */
template <typename CharT, typename Traits>
constexpr inline const CharT &
basic_string_view<CharT, Traits>::back() const noexcept
{
	return operator[](size() - 1);
}

/**
 * Returns reference to the first character in the view.
 * The behavior is undefined if empty() == true.
 *
 * @return reference to the first character.
 */
template <typename CharT, typename Traits>
constexpr inline const CharT &
basic_string_view<CharT, Traits>::front() const noexcept
{
	return operator[](0);
}

/**
 * Moves the start of the view forward by n characters.
 * The behavior is undefined if n > size().
 *
 * @param[in] n number of characters to remove from the start of the view
 */
template <typename CharT, typename Traits>
void
basic_string_view<CharT, Traits>::remove_prefix(size_type n)
{
	data_ += n;
	size_ -= n;
}

/**
 * Moves the end of the view back by n characters.
 * The behavior is undefined if n > size().
 *
 * @param[in] n number of characters to remove from the end of the view
 */
template <typename CharT, typename Traits>
void
basic_string_view<CharT, Traits>::remove_suffix(size_type n)
{
	size_ -= n;
}

/**
 * Exchanges the view with that of v.
 *
 * @param[in] v view to swap with
 */
template <typename CharT, typename Traits>
void
basic_string_view<CharT, Traits>::swap(
	basic_string_view<CharT, Traits> &v) noexcept
{
	std::swap(data_, v.data_);
	std::swap(size_, v.size_);
}

/**
 * Returns a view of the substring [pos, pos + rcount), where rcount is the
 * smaller of count and size() - pos.
 *
 * @param[in] pos position of the first character
 * @param[in] count requested length
 *
 * @return view of the substring [pos, pos + rcount).
 *
 * @throw std::out_of_range if pos > size()
 */
template <typename CharT, typename Traits>
constexpr basic_string_view<CharT, Traits>
basic_string_view<CharT, Traits>::substr(
	basic_string_view<CharT, Traits>::size_type pos,
	basic_string_view<CharT, Traits>::size_type count) const
{
	return pos > size()
		? throw std::out_of_range("string_view::substr")
		: basic_string_view(data() + pos,
				    (std::min)(count, size() - pos));
}

/**
 * Copies the substring [pos, pos + rcount) to the character array pointed to by
 * dest, where rcount is the smaller of count and size() - pos.
 *
 * @param[in] dest pointer to the destination character string
 * @param[in] pos position of the first character
 * @param[in] count requested substring length
 *
 * @return number of characters copied.
 *
 * @throw std::out_of_range if pos > size()
 */
template <typename CharT, typename Traits>
typename basic_string_view<CharT, Traits>::size_type
basic_string_view<CharT, Traits>::copy(
	CharT *dest, basic_string_view<CharT, Traits>::size_type count,
	basic_string_view<CharT, Traits>::size_type pos) const
{
	if (pos > size())
		throw std::out_of_range("string_view::copy");
	size_type rlen = (std::min)(count, size() - pos);
	Traits::copy(dest, data() + pos, rlen);
	return rlen;
}

/**
 * Compares two character sequences.
 *
 * @param[in] pos1 position of the first character in this view to compare
 * @param[in] n1 number of characters of this view to compare
 * @param[in] sv view to compare
 *
 * @return negative value if this view is less than the other character
 * sequence, zero if the both character sequences are equal, positive value if
 * this view is greater than the other character sequence.
 */
template <typename CharT, typename Traits>
inline int
basic_string_view<CharT, Traits>::compare(
	size_type pos1, size_type n1, basic_string_view<CharT, Traits> sv) const
{
	return substr(pos1, n1).compare(sv);
}

/**
 * Compares two character sequences.
 *
 * @param[in] pos1 position of the first character in this view to compare
 * @param[in] n1 number of characters of this view to compare
 * @param[in] pos2 position of the first character of the given view to compare
 * @param[in] n2 number of characters of the given view to compare
 * @param[in] sv view to compare
 *
 * @return negative value if this view is less than the other character
 * sequence, zero if the both character sequences are equal, positive value if
 * this view is greater than the other character sequence.
 */
template <typename CharT, typename Traits>
inline int
basic_string_view<CharT, Traits>::compare(size_type pos1, size_type n1,
					  basic_string_view sv, size_type pos2,
					  size_type n2) const
{
	return substr(pos1, n1).compare(sv.substr(pos2, n2));
}

/**
 * Compares two character sequences.
 *
 * @param[in] s pointer to the character string to compare to
 *
 * @return negative value if this view is less than the other character
 * sequence, zero if the both character sequences are equal, positive value if
 * this view is greater than the other character sequence.
 */
template <typename CharT, typename Traits>
inline int
basic_string_view<CharT, Traits>::compare(const CharT *s) const noexcept
{
	return compare(basic_string_view(s));
}

/**
 * Compares two character sequences.
 *
 * @param[in] pos1 position of the first character in this view to compare
 * @param[in] n1 number of characters of this view to compare
 * @param[in] s pointer to the character string to compare to
 *
 * @return negative value if this view is less than the other character
 * sequence, zero if the both character sequences are equal, positive value if
 * this view is greater than the other character sequence.
 */
template <typename CharT, typename Traits>
inline int
basic_string_view<CharT, Traits>::compare(size_type pos1, size_type n1,
					  const CharT *s) const
{
	return substr(pos1, n1).compare(basic_string_view(s));
}

/**
 * Compares two character sequences.
 *
 * @param[in] pos1 position of the first character in this view to compare
 * @param[in] n1 number of characters of this view to compare
 * @param[in] s pointer to the character string to compare to
 * @param[in] n2 number of characters of the given string to compare
 *
 * @return negative value if this view is less than the other character
 * sequence, zero if the both character sequences are equal, positive value if
 * this view is greater than the other character sequence.
 */
template <typename CharT, typename Traits>
inline int
basic_string_view<CharT, Traits>::compare(size_type pos1, size_type n1,
					  const CharT *s, size_type n2) const
{
	return substr(pos1, n1).compare(basic_string_view(s, n2));
}

/**
 * Compares this string_view with other. Works in the same way as
 * std::basic_string::compare.
 *
 * @return 0 if both character sequences compare equal,
 *	positive value if this is lexicographically greater than other,
 *	negative value if this is lexicographically less than other.
 */
template <typename CharT, typename Traits>
inline int
basic_string_view<CharT, Traits>::compare(const basic_string_view &other) const
	noexcept
{
	int ret = Traits::compare(data(), other.data(),
				  (std::min)(size(), other.size()));
	if (ret != 0)
		return ret;
	if (size() < other.size())
		return -1;
	if (size() > other.size())
		return 1;
	return 0;
}

/**
 * Non-member equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator==(basic_string_view<CharT, Traits> lhs,
	   basic_string_view<CharT, Traits> rhs)
{
	return (lhs.size() != rhs.size() ? false : lhs.compare(rhs) == 0);
}

/**
 * Non-member equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator==(
	basic_string_view<CharT, Traits> lhs,
	typename std::common_type<basic_string_view<CharT, Traits>>::type rhs)
{
	return (lhs.size() != rhs.size() ? false : lhs.compare(rhs) == 0);
}

/**
 * Non-member equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator==(
	typename std::common_type<basic_string_view<CharT, Traits>>::type lhs,
	basic_string_view<CharT, Traits> rhs)
{
	return (lhs.size() != rhs.size() ? false : lhs.compare(rhs) == 0);
}

/**
 * Non-member not equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator!=(basic_string_view<CharT, Traits> lhs,
	   basic_string_view<CharT, Traits> rhs)
{
	return (lhs.size() != rhs.size() ? true : lhs.compare(rhs) != 0);
}

/**
 * Non-member not equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator!=(
	typename std::common_type<basic_string_view<CharT, Traits>>::type lhs,
	basic_string_view<CharT, Traits> rhs)
{
	return (lhs.size() != rhs.size() ? true : lhs.compare(rhs) != 0);
}

/**
 * Non-member not equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator!=(
	basic_string_view<CharT, Traits> lhs,
	typename std::common_type<basic_string_view<CharT, Traits>>::type rhs)
{
	return (lhs.size() != rhs.size() ? true : lhs.compare(rhs) != 0);
}

/**
 * Non-member less than operator.
 */
template <class CharT, class Traits>
constexpr bool
operator<(basic_string_view<CharT, Traits> lhs,
	  basic_string_view<CharT, Traits> rhs)
{
	return lhs.compare(rhs) < 0;
}

/**
 * Non-member less than operator.
 */
template <class CharT, class Traits>
constexpr bool
operator<(typename std::common_type<basic_string_view<CharT, Traits>>::type lhs,
	  basic_string_view<CharT, Traits> rhs)
{
	return lhs.compare(rhs) < 0;
}

/**
 * Non-member less than operator.
 */
template <class CharT, class Traits>
constexpr bool
operator<(basic_string_view<CharT, Traits> lhs,
	  typename std::common_type<basic_string_view<CharT, Traits>>::type rhs)
{
	return lhs.compare(rhs) < 0;
}

/**
 * Non-member less or equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator<=(basic_string_view<CharT, Traits> lhs,
	   basic_string_view<CharT, Traits> rhs)
{
	return lhs.compare(rhs) <= 0;
}

/**
 * Non-member less or equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator<=(
	basic_string_view<CharT, Traits> lhs,
	typename std::common_type<basic_string_view<CharT, Traits>>::type rhs)
{
	return lhs.compare(rhs) <= 0;
}

/**
 * Non-member less or equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator<=(
	typename std::common_type<basic_string_view<CharT, Traits>>::type lhs,
	basic_string_view<CharT, Traits> rhs)
{
	return lhs.compare(rhs) <= 0;
}

/**
 * Non-member greater than operator.
 */
template <class CharT, class Traits>
constexpr bool
operator>(basic_string_view<CharT, Traits> lhs,
	  basic_string_view<CharT, Traits> rhs)
{
	return lhs.compare(rhs) > 0;
}

/**
 * Non-member greater than operator.
 */
template <class CharT, class Traits>
constexpr bool
operator>(typename std::common_type<basic_string_view<CharT, Traits>>::type lhs,
	  basic_string_view<CharT, Traits> rhs)
{
	return lhs.compare(rhs) > 0;
}

/**
 * Non-member greater than operator.
 */
template <class CharT, class Traits>
constexpr bool
operator>(basic_string_view<CharT, Traits> lhs,
	  typename std::common_type<basic_string_view<CharT, Traits>>::type rhs)
{
	return lhs.compare(rhs) > 0;
}

/**
 * Non-member greater or equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator>=(basic_string_view<CharT, Traits> lhs,
	   basic_string_view<CharT, Traits> rhs)
{
	return lhs.compare(rhs) >= 0;
}

/**
 * Non-member greater or equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator>=(
	typename std::common_type<basic_string_view<CharT, Traits>>::type lhs,
	basic_string_view<CharT, Traits> rhs)
{
	return lhs.compare(rhs) >= 0;
}

/**
 * Non-member greater or equal operator.
 */
template <class CharT, class Traits>
constexpr bool
operator>=(
	basic_string_view<CharT, Traits> lhs,
	typename std::common_type<basic_string_view<CharT, Traits>>::type rhs)
{
	return lhs.compare(rhs) >= 0;
}
#endif

} /* namespace obj */
} /* namespace pmem */

#endif /* LIBPMEMOBJ_CPP_STRING_VIEW */
