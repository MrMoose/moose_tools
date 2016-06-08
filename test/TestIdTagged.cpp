
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE IdTaggedTests
#include <boost/test/unit_test.hpp>

#include "../IdTaggedContainer.hpp"
#include "../IdTagged.hpp"
#include "../Error.hpp"

#if BOOST_MSVC
#pragma warning (disable : 4553) // faulty '==': operator has no effect; did you intend '='?  in checks
#endif

using namespace moose::tools;

class IdTaggedClass : public IdTagged< IdTaggedClass > {

	public:
		IdTaggedClass() = default;
		IdTaggedClass(const IdTaggedClass &n_other) = default;
		~IdTaggedClass() = default;
};

BOOST_AUTO_TEST_CASE(id_tagged_class) {

	BOOST_TEST_MESSAGE("using id tagged class container");

	IdTaggedClass i1;
	IdTaggedClass i2;

	BOOST_CHECK(i1.id() != i2.id());   // give me a call when this yields an error ;-)

// I have deleted the copy c'tor
//	IdTaggedClass i3(i2);
//	BOOST_CHECK(i2.id() == i3.id());
}

class MyIdTaggedContainer : public IdTaggedContainer< IdTaggedClass > {

	public:
		MyIdTaggedContainer() = default;
		MyIdTaggedContainer(const MyIdTaggedContainer &n_other) = delete;  // not allowed by base
		~MyIdTaggedContainer() = default;
};

BOOST_AUTO_TEST_CASE( basic_access ) {

	BOOST_TEST_MESSAGE("using id tagged container");
	
	MyIdTaggedContainer c;
	MyIdTaggedContainer::pointer_type object;
	BOOST_CHECK(c.size() == 0);
	
	BOOST_CHECK_THROW(c.insert(object), internal_error); // throw on null

	object.reset(new IdTaggedClass());
	
	BOOST_CHECK(c.insert(object) == true);
	BOOST_CHECK(c.insert(object) == false);  // already present
	BOOST_CHECK(c.size() == 1);

	object.reset(new IdTaggedClass());
	BOOST_CHECK(c.insert(object) == true);
	BOOST_CHECK(c.insert(object) == false);  // already present
	BOOST_CHECK(c.size() == 2);

	// we have two seperate object with differing id
	BOOST_CHECK(c[0]->id() != c[1]->id());

	BOOST_CHECK(c.remove(object->id()) == true);
	BOOST_CHECK(c.size() == 1);

	// we have one object left
	BOOST_CHECK(c[0]->id());
}


BOOST_AUTO_TEST_CASE(iterator_access) {

	BOOST_TEST_MESSAGE("using iterators on id tagged container");

	MyIdTaggedContainer::pointer_type o1(new IdTaggedClass());
	MyIdTaggedContainer::pointer_type o2(new IdTaggedClass());
	MyIdTaggedContainer::pointer_type o3(new IdTaggedClass());

	BOOST_REQUIRE(o1->id() != o2->id());
	BOOST_REQUIRE(o1->id() != o3->id());
	BOOST_REQUIRE(o2->id() != o3->id());

	MyIdTaggedContainer c;
	BOOST_CHECK(c.insert(o1));
	BOOST_CHECK(c.insert(o2));
	BOOST_CHECK(c.insert(o3));

	MyIdTaggedContainer::iterator i;
	MyIdTaggedContainer::const_iterator ci;

	BOOST_CHECK_NO_THROW(i = c.begin());
	BOOST_CHECK_NO_THROW(ci = c.cbegin());
	BOOST_CHECK_NO_THROW(i = c.end());
	BOOST_CHECK_NO_THROW(ci = c.cend());


	MyIdTaggedContainer::iterator start = c.begin();
	MyIdTaggedContainer::iterator end   = c.end();
	(void)end;

	MyIdTaggedContainer::pointer_type test1 = *start;

	// just some rough sanity checks
	BOOST_CHECK(   ((*start)->id() == o1->id())
				|| ((*start)->id() == o2->id())
				|| ((*start)->id() == o3->id()));
	
	o1 = *start++;
	o2 = *start++;
	o3 = *start;
	
	BOOST_CHECK(o1->id() != o2->id());
	BOOST_CHECK(o1->id() != o3->id());
	BOOST_CHECK(o2->id() != o3->id());
}


BOOST_AUTO_TEST_CASE(range_based_for) {

	BOOST_TEST_MESSAGE("testing range based for loop on id tagged container");
	
	int cnt = 0;
	IdTaggedClass::id_type last_id = 0;  // yeah, I bitch out too

	MyIdTaggedContainer::pointer_type o1(new IdTaggedClass());
	MyIdTaggedContainer::pointer_type o2(new IdTaggedClass());
	MyIdTaggedContainer::pointer_type o3(new IdTaggedClass());

	BOOST_REQUIRE(o1->id() != o2->id());
	BOOST_REQUIRE(o1->id() != o3->id());
	BOOST_REQUIRE(o2->id() != o3->id());

	MyIdTaggedContainer c;

	// It was buggy iterating an empty container
	for (const MyIdTaggedContainer::pointer_type &p : c) {
		(void)p;
		cnt++;  // count how many loops 
	}
	BOOST_CHECK(cnt == 0);

	BOOST_CHECK(c.insert(o1));
	BOOST_CHECK(c.insert(o2));
	BOOST_CHECK(c.insert(o3));
	
	for (const MyIdTaggedContainer::pointer_type &p : c) {
		BOOST_CHECK(p->id() != last_id);
		cnt++;  // count how many loops 
	}
	BOOST_CHECK(cnt == 3);

	// and do the same for const & container
	const MyIdTaggedContainer &cc(c);
	cnt = 0;
	for (const MyIdTaggedContainer::pointer_type &p : cc) {
		BOOST_CHECK(p->id() != last_id);
		cnt++;  // count how many loops 
	}
	BOOST_CHECK(cnt == 3);
}

