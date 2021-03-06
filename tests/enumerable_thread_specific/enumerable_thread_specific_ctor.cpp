/*
 * Copyright 2019, Intel Corporation
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

#include "unittest.hpp"

#include <libpmemobj++/experimental/enumerable_thread_specific.hpp>
#include <libpmemobj++/make_persistent.hpp>

#include <set>

namespace nvobj = pmem::obj;

using test_t = size_t;
using container_type = nvobj::experimental::enumerable_thread_specific<test_t>;

// Adding more concurrency will increase DRD test time
const size_t concurrency = 16;

struct root {
	nvobj::persistent_ptr<container_type> pptr1;
	nvobj::persistent_ptr<container_type> pptr2;
	nvobj::persistent_ptr<container_type> pptr3;
};

template <typename Function>
void
parallel_exec(size_t concurrency, Function f)
{
	std::vector<std::thread> threads;
	threads.reserve(concurrency);

	for (size_t i = 0; i < concurrency; ++i) {
		threads.emplace_back(f, i);
	}

	for (auto &t : threads) {
		t.join();
	}
}

void
copy_ctor_test(nvobj::pool<struct root> &pop,
	       nvobj::persistent_ptr<container_type> &copy_from,
	       nvobj::persistent_ptr<container_type> &copy_to)
{
	UT_ASSERT(copy_to == nullptr);
	// Checking copy ctor
	nvobj::transaction::run(pop, [&] {
		copy_to = nvobj::make_persistent<container_type>(*copy_from);
	});
	UT_ASSERT(copy_from->size() == concurrency);
	UT_ASSERT(copy_to->size() == concurrency);

	std::set<size_t> checker;
	for (auto &e : *copy_to) {
		UT_ASSERT(checker.emplace(e).second);
	}
	for (auto &e : *copy_from) {
		UT_ASSERT(!checker.emplace(e).second);
	}
	UT_ASSERT(checker.size() == concurrency);
}

void
move_ctor_test(nvobj::pool<struct root> &pop,
	       nvobj::persistent_ptr<container_type> &move_from,
	       nvobj::persistent_ptr<container_type> &move_to)
{
	UT_ASSERT(move_to == nullptr);
	// Checking move ctor
	nvobj::transaction::run(pop, [&] {
		move_to = nvobj::make_persistent<container_type>(
			std::move(*move_from));
	});
	UT_ASSERT(move_from->empty());
	UT_ASSERT(move_to->size() == concurrency);

	std::set<size_t> checker;
	for (auto &e : *move_to) {
		UT_ASSERT(checker.emplace(e).second);
	}
	UT_ASSERT(checker.size() == concurrency);
}

int
main(int argc, char *argv[])
{
	START();

	if (argc < 2) {
		std::cerr << "usage: " << argv[0] << " file-name" << std::endl;
		return 1;
	}

	auto path = argv[1];
	auto pop = nvobj::pool<root>::create(
		path, "TLSTest: enumerable_thread_specific_ctor",
		PMEMOBJ_MIN_POOL, S_IWUSR | S_IRUSR);

	auto r = pop.root();

	try {
		nvobj::transaction::run(pop, [&] {
			r->pptr1 = nvobj::make_persistent<container_type>();
		});

		parallel_exec(concurrency, [&](size_t thread_index) {
			r->pptr1->local() = thread_index;
		});

		copy_ctor_test(pop, r->pptr1, r->pptr2);
		move_ctor_test(pop, r->pptr1, r->pptr3);

		nvobj::transaction::run(pop, [&] {
			nvobj::delete_persistent<container_type>(r->pptr1);
			nvobj::delete_persistent<container_type>(r->pptr2);
			nvobj::delete_persistent<container_type>(r->pptr3);
		});
	} catch (std::exception &e) {
		UT_FATALexc(e);
	}

	pop.close();

	return 0;
}
