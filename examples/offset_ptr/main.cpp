#include "self_relative_ptr.hpp"

namespace nvobj = pmem::obj;
namespace nvobjexp = nvobj::experimental;

/*
 * test_null_ptr -- verifies if the pointer correctly behaves like a
 * nullptr-value
 */
template <template <typename U> class pointer>
void
test_null_ptr(pointer<int> &f)
{
	UT_ASSERT((bool)f == false);
	UT_ASSERT(!f);
	UT_ASSERTeq(f.get(), nullptr);
	UT_ASSERT(f == nullptr);
}

/*
 * test_ptr_operators_null -- verifies various operations on nullptr pointers
 */
template <template <typename U> class pointer>
void
test_ptr_operators_null()
{
	pointer<int> int_default_null;
	test_null_ptr(int_default_null);

	pointer<int> int_explicit_ptr_null = nullptr;
	test_null_ptr(int_explicit_ptr_null);

	pointer<int> int_explicit_oid_null = OID_NULL;
	test_null_ptr(int_explicit_oid_null);

	pointer<int> int_base = nullptr;
	pointer<int> int_same = int_base;
	int_same = int_base;
	test_null_ptr(int_same);

	swap(int_base, int_same);
}

constexpr int TEST_INT = 10;
constexpr int TEST_ARR_SIZE = 10;
constexpr char TEST_CHAR = 'a';

struct foo {
	int bar;
	char arr[TEST_ARR_SIZE];
};

template <template <typename U> class pointer>
struct nested {
	pointer<foo> inner;
};

template <template <typename U> class pointer, class pointer_base>
struct templated_root {
	pointer<foo> pfoo;
	pointer<int[(long unsigned) TEST_ARR_SIZE]> parr;
	pointer_base arr[3];

	/* This variable is unused, but it's here to check if the persistent_ptr
	 * does not violate its own restrictions.
	 */
	pointer<nested<pointer>> outer;
};

template <typename T>
using self_relative_ptr = nvobj::experimental::self_relative_ptr<T>;
using self_relative_ptr_base = nvobj::experimental::self_relative_ptr_base;

bool
base_is_null(const self_relative_ptr_base &ptr)
{
	return ptr.is_null();
}

/*
 * test_offset -- test offset calculation within a hierarchy
 */
void
test_offset()
{
	struct A {
		uint64_t a;
	} a;

	struct B {
		uint64_t b;
	} b;

	struct C : public A, public B {
		uint64_t c;
	} c;


    auto distance =
        self_relative_ptr_base::distance_between;

    self_relative_ptr<C> cptr = &c;
    self_relative_ptr<B> bptr = cptr;
    UT_ASSERT(distance(cptr, bptr) > 0);
    UT_ASSERT(static_cast<std::size_t>(
                distance(cptr, bptr)) == sizeof(A));

    self_relative_ptr<B> bptr2;
    bptr2 = cptr;
    UT_ASSERT(distance(cptr, bptr2) > 0);
    UT_ASSERT(static_cast<std::size_t>(
                distance(cptr, bptr2)) == sizeof(A));

    self_relative_ptr<B> bptr3 =
        static_cast<self_relative_ptr<B>>(cptr);
    UT_ASSERT(distance(cptr, bptr3) > 0);
    UT_ASSERT(static_cast<std::size_t>(
                distance(cptr, bptr3)) == sizeof(A));
}

void
test_base_ptr_casting()
{
    foo f;
    int test_int = TEST_INT;

    r->arr[0] = self_relative_ptr<foo>{&f};
    r->arr[1] = self_relative_ptr<int>{&test_int};
    r->arr[2] = nullptr;

    UT_ASSERTne(r->arr[0].to_void_pointer(), nullptr);
    UT_ASSERTeq(*static_cast<int *>(
                r->arr[1].to_void_pointer()),
            TEST_INT);
    UT_ASSERTeq(r->arr[2].to_void_pointer(), nullptr);

    self_relative_ptr<foo> tmp0 =
        static_cast<foo *>(r->arr[0].to_void_pointer());
    self_relative_ptr<int> tmp1 =
        static_cast<int *>(r->arr[1].to_void_pointer());
    self_relative_ptr<foo> tmp2 =
        static_cast<foo *>(r->arr[2].to_void_pointer());
}

void
test_base_ptr_assignment()
{
	int tmp;

	self_relative_ptr_base ptr1 = &tmp;
	self_relative_ptr_base ptr2 = nullptr;

	ptr1 = ptr2;

	UT_ASSERT(ptr1.to_void_pointer() == nullptr);
	UT_ASSERT(ptr2.to_void_pointer() == nullptr);
}

constexpr int ARR_SIZE = 10000;

struct root {
	atomic_ptr<int[ARR_SIZE]> parr;
	atomic_ptr<int> ptr;
};

/*
 * prepare_array -- preallocate and fill a persistent array
 */
/*
 * prepare_array -- preallocate and fill a persistent array
 */
template <typename T, template <typename U> class pointer>
pointer<T>
get_test_array()
{
	static T arr[TEST_ARR_SIZE];
	pointer<T> parr_vsize = arr;

	for (int i = 0; i < TEST_ARR_SIZE; ++i) {
        parray[i] = i;
    }

    return parr_vsize;
}

/*
 * test_arith -- test arithmetic operations on persistent pointers
 */
template <template <typename U> class pointer>
void
test_arith()
{
	auto parr_vsize = get_test_array<int, pointer>(pop);

	/* test prefix postfix operators */
	for (int i = 0; i < TEST_ARR_SIZE; ++i) {
		UT_ASSERTeq(*parr_vsize, i);
		parr_vsize++;
	}

	for (int i = TEST_ARR_SIZE; i > 0; --i) {
		parr_vsize--;
		UT_ASSERTeq(*parr_vsize, i - 1);
	}

	for (int i = 0; i < TEST_ARR_SIZE; ++i) {
		UT_ASSERTeq(*parr_vsize, i);
		++parr_vsize;
	}

	for (int i = TEST_ARR_SIZE; i > 0; --i) {
		--parr_vsize;
		UT_ASSERTeq(*parr_vsize, i - 1);
	}

	/* test addition assignment and subtraction */
	parr_vsize += 2;
	UT_ASSERTeq(*parr_vsize, 2);

	parr_vsize -= 2;
	UT_ASSERTeq(*parr_vsize, 0);

	/* test strange invocations, parameter ignored */
	parr_vsize.operator++(5);
	UT_ASSERTeq(*parr_vsize, 1);

	parr_vsize.operator--(2);
	UT_ASSERTeq(*parr_vsize, 0);

	/* test subtraction and addition */
	for (int i = 0; i < TEST_ARR_SIZE; ++i)
		UT_ASSERTeq(*(parr_vsize + i), i);

	/* using STL one-pas-end style */
	auto parr_end = parr_vsize + TEST_ARR_SIZE;

	for (int i = TEST_ARR_SIZE; i > 0; --i)
		UT_ASSERTeq(*(parr_end - i), TEST_ARR_SIZE - i);

	UT_OUT("%ld", parr_end - parr_vsize);
	UT_ASSERTeq(parr_end - parr_vsize, TEST_ARR_SIZE);

	/* check ostream operator */
	std::stringstream stream;
	stream << parr_vsize;
	UT_OUT("%s", stream.str().c_str());
}

/*
 * test_relational -- test relational operators on persistent pointers
 */
template <template <typename U> class pointer>
void
test_relational()
{
	auto first_elem = get_test_array<int, pointer>();
	pointer<int[10][12]> parray;
	auto last_elem = first_elem + TEST_ARR_SIZE - 1;

	UT_ASSERT(first_elem != last_elem);
	UT_ASSERT(first_elem <= last_elem);
	UT_ASSERT(first_elem < last_elem);
	UT_ASSERT(last_elem > first_elem);
	UT_ASSERT(last_elem >= first_elem);
	UT_ASSERT(first_elem == first_elem);
	UT_ASSERT(first_elem >= first_elem);
	UT_ASSERT(first_elem <= first_elem);

	/* nullptr comparisons */
	UT_ASSERT(first_elem != nullptr);
	UT_ASSERT(nullptr != first_elem);
	UT_ASSERT(!(first_elem == nullptr));
	UT_ASSERT(!(nullptr == first_elem));

	UT_ASSERT(nullptr < first_elem);
	UT_ASSERT(!(first_elem < nullptr));
	UT_ASSERT(nullptr <= first_elem);
	UT_ASSERT(!(first_elem <= nullptr));

	UT_ASSERT(first_elem > nullptr);
	UT_ASSERT(!(nullptr > first_elem));
	UT_ASSERT(first_elem >= nullptr);
	UT_ASSERT(!(nullptr >= first_elem));

	/* pointer to array */
	UT_ASSERT(parray == nullptr);
	UT_ASSERT(nullptr == parray);
	UT_ASSERT(!(parray != nullptr));
	UT_ASSERT(!(nullptr != parray));

	UT_ASSERT(!(nullptr < parray));
	UT_ASSERT(!(parray < nullptr));
	UT_ASSERT(nullptr <= parray);
	UT_ASSERT(parray <= nullptr);

	UT_ASSERT(!(parray > nullptr));
	UT_ASSERT(!(nullptr > parray));
	UT_ASSERT(parray >= nullptr);
	UT_ASSERT(nullptr >= parray);

	auto different_array = get_test_array<double, pointer>();

	/* only verify if this compiles */
	UT_ASSERT((first_elem < different_array) || true);
}

// constexpr size_t CONCURRENCY = 20;
// constexpr size_t MEAN_CONCURRENCY = CONCURRENCY * 2;
// constexpr size_t HIGH_CONCURRENCY = CONCURRENCY * 5;

// using pmem::obj::experimental::self_relative_ptr;

// template <typename T, bool need_volatile>
// using atomic_type = typename std::conditional<
// 	need_volatile,
// 	typename std::add_volatile<std::atomic<self_relative_ptr<T>>>::type,
// 	std::atomic<self_relative_ptr<T>>>::type;

// template <bool volatile_atomic>
// void
// test_fetch()
// {
// 	constexpr size_t count_iterations = 300;
// 	constexpr size_t arr_size = CONCURRENCY * count_iterations;
// 	std::vector<int> vptr(arr_size, 0);

// 	atomic_type<int, volatile_atomic> ptr{vptr.data()};

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		for (size_t i = 0; i < count_iterations; ++i) {
// 			auto element = ptr.fetch_add(1);
// 			*element += 1;
// 		}
// 	});

// 	UT_ASSERT(vptr.data() + arr_size == ptr.load().get());
// 	for (auto element : vptr) {
// 		UT_ASSERTeq(element, 1);
// 	}

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		for (size_t i = 0; i < count_iterations; ++i) {
// 			auto element = ptr.fetch_sub(1) - 1;
// 			*element += 1;
// 		}
// 	});

// 	UT_ASSERT(vptr.data() == ptr.load().get());
// 	for (auto element : vptr) {
// 		UT_ASSERTeq(element, 2);
// 	}

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		for (size_t i = 0; i < count_iterations; ++i) {
// 			auto element = ptr++;
// 			*element += 1;
// 		}
// 	});

// 	UT_ASSERT(vptr.data() + arr_size == ptr.load().get());
// 	for (auto element : vptr) {
// 		UT_ASSERTeq(element, 3);
// 	}

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		for (size_t i = 0; i < count_iterations; ++i) {
// 			auto element = --ptr;
// 			*element += 1;
// 		}
// 	});

// 	UT_ASSERT(vptr.data() == ptr.load().get());
// 	for (auto element : vptr) {
// 		UT_ASSERTeq(element, 4);
// 	}

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		for (size_t i = 0; i < count_iterations; ++i) {
// 			auto element = ++ptr - 1;
// 			*element += 1;
// 		}
// 	});

// 	UT_ASSERT(vptr.data() + arr_size == ptr.load().get());
// 	for (auto element : vptr) {
// 		UT_ASSERTeq(element, 5);
// 	}

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		for (size_t i = 0; i < count_iterations; ++i) {
// 			auto element = ptr-- - 1;
// 			*element += 1;
// 		}
// 	});

// 	UT_ASSERT(vptr.data() == ptr.load().get());
// 	for (auto element : vptr) {
// 		UT_ASSERTeq(element, 6);
// 	}

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		for (size_t i = 0; i < count_iterations; ++i) {
// 			auto element = (ptr += 1) - 1;
// 			*element += 1;
// 		}
// 	});

// 	UT_ASSERT(vptr.data() + arr_size == ptr.load().get());
// 	for (auto element : vptr) {
// 		UT_ASSERTeq(element, 7);
// 	}

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		for (size_t i = 0; i < count_iterations; ++i) {
// 			auto element = (ptr -= 1);
// 			*element += 1;
// 		}
// 	});

// 	UT_ASSERT(vptr.data() == ptr.load().get());
// 	for (auto element : vptr) {
// 		UT_ASSERTeq(element, 8);
// 	}
// }

// template <bool volatile_atomic>
// void
// test_exchange()
// {
// 	self_relative_ptr<int> first = reinterpret_cast<int *>(uintptr_t{0});
// 	self_relative_ptr<int> second = reinterpret_cast<int *>(~uintptr_t{0});

// 	atomic_type<int, volatile_atomic> ptr;

// 	UT_ASSERT(ptr.load(std::memory_order_acquire).is_null());

// 	ptr.store(first, std::memory_order_release);

// 	UT_ASSERT(ptr.load() == first);

// 	auto before_ptr = ptr.exchange(second, std::memory_order_acq_rel);

// 	UT_ASSERT(ptr.load(std::memory_order_acquire) == second);

// 	parallel_exec(MEAN_CONCURRENCY, [&](size_t i) {
// 		for (size_t j = 0; j < 1000000; j++) {
// 			auto before = ptr.exchange(i % 2 == 0 ? first : second,
// 						   std::memory_order_acq_rel);
// 			UT_ASSERT(before == first || before == second);
// 		}
// 	});

// 	auto last_ptr = ptr.load();
// 	UT_ASSERT(last_ptr == first || last_ptr == second);
// }

// template <bool volatile_atomic>
// void
// test_compare_exchange()
// {
// 	int *first = reinterpret_cast<int *>(uintptr_t{0});
// 	int *second = reinterpret_cast<int *>(~uintptr_t{0});
// 	atomic_type<int, volatile_atomic> atomic_ptr{first};
// 	std::atomic<size_t> exchanged(0);

// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		// tst_val != atomic_ptr  ==>  tst_val is modified
// 		// tst_val == atomic_ptr  ==>  atomic_ptr is modified

// 		self_relative_ptr<int> tst_val{first}, new_val{second};
// 		if (atomic_ptr.compare_exchange_strong(tst_val, new_val)) {
// 			++exchanged;
// 		} else {
// 			UT_ASSERT(tst_val == new_val);
// 		}
// 	});

// 	UT_ASSERTeq(exchanged.load(), 1);
// 	UT_ASSERT(atomic_ptr.load().get() == second);

// 	atomic_ptr = first;
// 	parallel_exec(CONCURRENCY, [&](size_t) {
// 		// tst_val != atomic_ptr  ==>  tst_val is modified
// 		// tst_val == atomic_ptr  ==>  atomic_ptr is modified

// 		self_relative_ptr<int> tst_val{first}, new_val{second};
// 		if (atomic_ptr.compare_exchange_strong(
// 			    tst_val, new_val, std::memory_order_acquire,
// 			    std::memory_order_relaxed)) {
// 			++exchanged;
// 		} else {
// 			UT_ASSERT(tst_val == new_val);
// 		}
// 	});

// 	UT_ASSERTeq(exchanged.load(), 2);
// 	UT_ASSERT(atomic_ptr.load().get() == second);
// }

// /**
//  * Small lock free stack for tests
//  */
// template <bool volatile_atomic, std::memory_order... weak_args>
// class test_stack {
// public:
// 	struct node;

// 	using value_type = size_t;
// 	using node_ptr_type = self_relative_ptr<node>;

// 	struct node {
// 		size_t value;
// 		node_ptr_type next;
// 	};

// 	void
// 	push(const value_type &data)
// 	{
// 		node_ptr_type new_node = new node{data, nullptr};

// 		auto next_node = head.load(std::memory_order_acquire);

// 		while (!head.compare_exchange_weak(next_node, new_node,
// 						   weak_args...))
// 			; // empty
// 		new_node->next = next_node;
// 	}

// 	std::vector<value_type>
// 	get_all()
// 	{
// 		auto current_node = head.load();
// 		std::vector<value_type> values;
// 		while (current_node != nullptr) {
// 			values.push_back(current_node->value);
// 			current_node = current_node->next;
// 		}
// 		return values;
// 	}

// 	~test_stack()
// 	{
// 		auto current_node = head.load();
// 		while (current_node != nullptr) {
// 			auto prev_node = current_node.get();
// 			current_node = current_node->next;
// 			delete prev_node;
// 		}
// 	}

// private:
// 	atomic_type<node, volatile_atomic> head;
// };

// template <bool volatile_atomic, std::memory_order... weak_args>
// void
// test_stack_based_on_atomic()
// {
// 	test_stack<volatile_atomic, weak_args...> stack;
// 	constexpr size_t count_iterations = 1000;
// 	parallel_exec(HIGH_CONCURRENCY, [&](size_t i) {
// 		for (size_t j = 0; j < count_iterations; j++) {
// 			stack.push(j + (i * count_iterations));
// 		}
// 	});
// 	auto all = stack.get_all();
// 	std::sort(all.begin(), all.end());
// 	for (size_t i = 0; i < HIGH_CONCURRENCY * count_iterations; i++) {
// 		UT_ASSERTeq(all[i], i);
// 	}
// }

// template <bool volatile_atomic>
// void
// test_the_stack()
// {
// 	test_stack_based_on_atomic<volatile_atomic, std::memory_order_acquire,
// 				   std::memory_order_relaxed>();
// 	test_stack_based_on_atomic<volatile_atomic,
// 				   std::memory_order_seq_cst>();
// }

// template <bool volatile_atomic>
// void
// test_is_lock_free()
// {
// 	atomic_type<int, volatile_atomic> a;
// 	((void)(a.is_lock_free()));
// 	((void)(std::atomic_is_lock_free(&a)));
// }

// template <bool volatile_atomic>
// void
// test(int argc, char *argv[])
// {
// 	test_fetch<volatile_atomic>();
// 	test_exchange<volatile_atomic>();
// 	test_compare_exchange<volatile_atomic>();
// 	test_the_stack<volatile_atomic>();
// 	test_is_lock_free<volatile_atomic>();
// }

int main() {
    
}
