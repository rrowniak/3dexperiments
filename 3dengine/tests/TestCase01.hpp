#include <boost/test/included/unit_test.hpp>

#include "testlib.h"

namespace utf = boost::unit_test;

// BOOST_CHECK( add( 2,2 ) == 4 );        // #1 continues on error
// BOOST_REQUIRE( add( 2,2 ) == 4 );      // #2 throws on error
// if( add( 2,2 ) != 4 )
//   BOOST_ERROR( "Ouch..." );            // #3 continues on error
// if( add( 2,2 ) != 4 )
//   BOOST_FAIL( "Ouch..." );             // #4 throws on error
// if( add( 2,2 ) != 4 ) throw "Ouch..."; // #5 throws on error
// BOOST_CHECK_MESSAGE( add( 2,2 ) == 4,  // #6 continues on error
//                      "add(..) result: " << add( 2,2 ) );
// BOOST_CHECK_EQUAL( add( 2,2 ), 4 );	  // #7 continues on error

BOOST_AUTO_TEST_CASE(basic_add_test, * utf::tolerance(0.01))
{
    BOOST_REQUIRE( add( 2,2 ) == 4 );
}