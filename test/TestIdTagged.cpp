
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

	IdTaggedClass i3(i2);
	BOOST_CHECK(i2.id() == i3.id());
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
	BOOST_CHECK(c[0].id() != c[1].id());

	BOOST_CHECK(c.remove(object->id()) == true);
	BOOST_CHECK(c.size() == 1);

	// we have one object left
	BOOST_CHECK(c[0].id());
}


BOOST_AUTO_TEST_CASE(iterator_access) {

	BOOST_TEST_MESSAGE("using iterators on id tagged container");

	MyIdTaggedContainer c;
	BOOST_CHECK(c.insert(MyIdTaggedContainer::pointer_type(new IdTaggedClass())));
	BOOST_CHECK(c.insert(MyIdTaggedContainer::pointer_type(new IdTaggedClass())));
	BOOST_CHECK(c.insert(MyIdTaggedContainer::pointer_type(new IdTaggedClass())));

	MyIdTaggedContainer::iterator i;
	MyIdTaggedContainer::const_iterator ci;

	BOOST_CHECK_NO_THROW(i = c.begin());
	BOOST_CHECK_NO_THROW(ci = c.cbegin());
	BOOST_CHECK_NO_THROW(i = c.end());
	BOOST_CHECK_NO_THROW(ci = c.cend());


	MyIdTaggedContainer::iterator start = c.begin();
	MyIdTaggedContainer::iterator end   = c.end();
	BOOST_CHECK(start != end);

//	BOOST_CHECK(c.begin() != c.end());
//	BOOST_CHECK(c.cbegin() != c.cend());

}

