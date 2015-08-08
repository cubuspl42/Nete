#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <nete/nete.h>

TEST_CASE( "multivector appending", "[multivector]" )
{
    using namespace nete::tl;

    multivector<types<char, uint16_t>> v;

    REQUIRE(v.size() == 0);
    REQUIRE(v.capacity() == 0);

    v.emplace_back();

    REQUIRE(v.size() == 1);
    REQUIRE(v.capacity() >= 1);

    const char *storage = v.storage();
    char *data_0 = v.data<0>();
    uint16_t *data_1 = v.data<1>();

    REQUIRE(storage == data_0);

    v.get<0>(v.begin()) = 'a';
    v.get<1>(v.begin()) = 0xFFFF;

    REQUIRE(v.get<0>(v.begin()) == 'a');
    REQUIRE(v.get<1>(v.begin()) == 0xFFFF);
    REQUIRE(v.at<0>(0) == 'a');
    REQUIRE(v.at<1>(0) == 0xFFFF);

    char layout_0[] = {'a', '\xff', '\xff'};

    REQUIRE(!memcmp(storage, layout_0, sizeof(layout_0)));

    v.emplace_back();

    storage = v.storage();

    v.get<0>(++v.begin()) = 'b';
    v.get<1>(++v.begin()) = 0xAAAA;

    REQUIRE(v.at<0>(0) == 'a');
    REQUIRE(v.at<1>(0) == 0xFFFF);
    REQUIRE(v.get<0>(++v.begin()) == 'b');
    REQUIRE(v.get<1>(++v.begin()) == 0xAAAA);
    REQUIRE(v.at<0>(1) == 'b');
    REQUIRE(v.at<1>(1) == 0xAAAA);

    char layout_1[] = {'a', 'b', '\xff', '\xff', '\xaa', '\xaa'};

    REQUIRE(!memcmp(storage, layout_1, sizeof(layout_1)));

    v.resize(6);

    v.at<0>(5) = 'c';
    v.at<1>(5) = 0xBBBB;

    REQUIRE(v.at<0>(0) == 'a');
    REQUIRE(v.at<1>(0) == 0xFFFF);
    REQUIRE(v.at<0>(1) == 'b');
    REQUIRE(v.at<1>(1) == 0xAAAA);
    REQUIRE(v.at<0>(5) == 'c');
    REQUIRE(v.at<1>(5) == 0xBBBB);
}

TEST_CASE( "multivector reserving", "[multivector]" )
{
    using namespace nete::tl;

    multivector<types<char, uint16_t, uint32_t>> v;

    REQUIRE(v.capacity() == 0);

    v.emplace_back();

    REQUIRE(v.capacity() > 0);

    v.at<0>(0) = 'a';
    v.at<1>(0) = 0xAAAA;
    v.at<2>(0) = 0xBBBBBBBB;

    REQUIRE(v.at<0>(0) == 'a');
    REQUIRE(v.at<1>(0) == 0xAAAA);
    REQUIRE(v.at<2>(0) == 0xBBBBBBBB);

    const char *storage = v.storage();
    char layout_0[] = {'a', '\xaa', '\xaa', '\xbb', '\xbb', '\xbb', '\xbb'};

    REQUIRE(!memcmp(storage, layout_0, sizeof(layout_0)));

    v.reserve(32);

    REQUIRE(v.capacity() >= 32);

    REQUIRE(v.at<0>(0) == 'a');
    REQUIRE(v.at<1>(0) == 0xAAAA);
    REQUIRE(v.at<2>(0) == 0xBBBBBBBB);

    v.resize(32);

    REQUIRE(v.size() == 32);
    REQUIRE(v.capacity() >= 32);

    v.at<0>(31) = 'b';
    v.at<1>(31) = 0xCCCC;
    v.at<2>(31) = 0xDDDDDDDD;

    v.reserve(64);

    REQUIRE(v.size() == 32);
    REQUIRE(v.capacity() >= 64);

    REQUIRE(v.at<0>(0) == 'a');
    REQUIRE(v.at<1>(0) == 0xAAAA);
    REQUIRE(v.at<2>(0) == 0xBBBBBBBB);

    REQUIRE(v.at<0>(31) == 'b');
    REQUIRE(v.at<1>(31) == 0xCCCC);
    REQUIRE(v.at<2>(31) == 0xDDDDDDDD);
}

TEST_CASE( "multivector swapping", "[multivector]" )
{
    using namespace nete::tl;

    multivector<types<char, uint16_t, uint32_t>> v;

    v.resize(3);

    REQUIRE(v.size() == 3);

    v.at<0>(0) = 'a';
    v.at<1>(0) = 0xAAAA;
    v.at<2>(0) = 0xBBBBBBBB;
    v.at<0>(1) = 'b';
    v.at<1>(1) = 0xBBBB;
    v.at<2>(1) = 0xCCCCCCCC;
    v.at<0>(2) = 'c';
    v.at<1>(2) = 0xDDDD;
    v.at<2>(2) = 0xEEEEEEEE;

    REQUIRE(v.at<0>(0) == 'a');
    REQUIRE(v.at<1>(0) == 0xAAAA);
    REQUIRE(v.at<2>(0) == 0xBBBBBBBB);
    REQUIRE(v.at<0>(1) == 'b');
    REQUIRE(v.at<1>(1) == 0xBBBB);
    REQUIRE(v.at<2>(1) == 0xCCCCCCCC);
    REQUIRE(v.at<0>(2) == 'c');
    REQUIRE(v.at<1>(2) == 0xDDDD);
    REQUIRE(v.at<2>(2) == 0xEEEEEEEE);

    v.swap(v.begin(), --v.end());

    REQUIRE(v.size() == 3);

    REQUIRE(v.at<0>(0) == 'c');
    REQUIRE(v.at<1>(0) == 0xDDDD);
    REQUIRE(v.at<2>(0) == 0xEEEEEEEE);
    REQUIRE(v.at<0>(1) == 'b');
    REQUIRE(v.at<1>(1) == 0xBBBB);
    REQUIRE(v.at<2>(1) == 0xCCCCCCCC);
    REQUIRE(v.at<0>(2) == 'a');
    REQUIRE(v.at<1>(2) == 0xAAAA);
    REQUIRE(v.at<2>(2) == 0xBBBBBBBB);
}

TEST_CASE( "multivector iteration", "[multivector]" )
{
    using namespace nete::tl;

    multivector<types<char, uint16_t, uint32_t>> v;

    v.resize(4);

    REQUIRE(v.size() == 4);

    v.at<0>(0) = 'a';
    v.at<1>(0) = 0xAAAA;
    v.at<2>(0) = 0xBBBBBBBB;
    v.at<0>(1) = 'b';
    v.at<1>(1) = 0xCCCC;
    v.at<2>(1) = 0xDDDDDDDD;
    v.at<0>(2) = 'c';
    v.at<1>(2) = 0xEEEE;
    v.at<2>(2) = 0xFFFFFFFF;
    v.at<0>(3) = 'd';
    v.at<1>(3) = 0xABAB;
    v.at<2>(3) = 0xBCBCBCBC;

    int i = 0;

    for(auto it = v.begin(); it != v.end(); ++it) {
        switch(i) {
            case 0:
                REQUIRE(v.get<0>(it) == 'a');
                REQUIRE(v.get<1>(it) == 0xAAAA);
                REQUIRE(v.get<2>(it) == 0xBBBBBBBB);
                break;
            case 1:
                REQUIRE(v.get<0>(it) == 'b');
                REQUIRE(v.get<1>(it) == 0xCCCC);
                REQUIRE(v.get<2>(it) == 0xDDDDDDDD);
                break;
            case 2:
                REQUIRE(v.get<0>(it) == 'c');
                REQUIRE(v.get<1>(it) == 0xEEEE);
                REQUIRE(v.get<2>(it) == 0xFFFFFFFF);
                break;
            case 3:
                REQUIRE(v.get<0>(it) == 'd');
                REQUIRE(v.get<1>(it) == 0xABAB);
                REQUIRE(v.get<2>(it) == 0xBCBCBCBC);
                break;
            default:
                REQUIRE(0 == 1);
        }
        ++i;
    }
}

TEST_CASE( "multivector reverse iteration", "[multivector]" )
{
    using namespace nete::tl;

    multivector<types<char, uint16_t, uint32_t>> v;

    v.resize(4);

    REQUIRE(v.size() == 4);

    v.at<0>(0) = 'a';
    v.at<1>(0) = 0xAAAA;
    v.at<2>(0) = 0xBBBBBBBB;
    v.at<0>(1) = 'b';
    v.at<1>(1) = 0xCCCC;
    v.at<2>(1) = 0xDDDDDDDD;
    v.at<0>(2) = 'c';
    v.at<1>(2) = 0xEEEE;
    v.at<2>(2) = 0xFFFFFFFF;
    v.at<0>(3) = 'd';
    v.at<1>(3) = 0xABAB;
    v.at<2>(3) = 0xBCBCBCBC;

    int i = 0;

    for(auto it = v.rbegin(); it != v.rend(); ++it) {
        switch(i) {
            case 3:
                REQUIRE(v.get<0>(it) == 'a');
                REQUIRE(v.get<1>(it) == 0xAAAA);
                REQUIRE(v.get<2>(it) == 0xBBBBBBBB);
                break;
            case 2:
                REQUIRE(v.get<0>(it) == 'b');
                REQUIRE(v.get<1>(it) == 0xCCCC);
                REQUIRE(v.get<2>(it) == 0xDDDDDDDD);
                break;
            case 1:
                REQUIRE(v.get<0>(it) == 'c');
                REQUIRE(v.get<1>(it) == 0xEEEE);
                REQUIRE(v.get<2>(it) == 0xFFFFFFFF);
                break;
            case 0:
                REQUIRE(v.get<0>(it) == 'd');
                REQUIRE(v.get<1>(it) == 0xABAB);
                REQUIRE(v.get<2>(it) == 0xBCBCBCBC);
                break;
            default:
                REQUIRE(0 == 1);
        }
        ++i;
    }
}

TEST_CASE( "multivector assigning", "[multivector]" )
{
    using namespace nete::tl;

    multivector<types<char, uint16_t, uint32_t>> v;

    v.resize(4);

    REQUIRE(v.size() == 4);

    v.at<0>(0) = 'a';
    v.at<1>(0) = 0xAAAA;
    v.at<2>(0) = 0xBBBBBBBB;
    v.at<0>(1) = 'b';
    v.at<1>(1) = 0xCCCC;
    v.at<2>(1) = 0xDDDDDDDD;
    v.at<0>(2) = 'c';
    v.at<1>(2) = 0xEEEE;
    v.at<2>(2) = 0xFFFFFFFF;
    v.at<0>(3) = 'd';
    v.at<1>(3) = 0xABAB;
    v.at<2>(3) = 0xBCBCBCBC;

    multivector<types<char, uint16_t, uint32_t>> v1;

    v1 = v;

    REQUIRE(v1.size() == v.size());
    REQUIRE(v1.capacity() == v.capacity());
    REQUIRE(v1.get<0>(0) == 'a');
    REQUIRE(v1.get<1>(0) == 0xAAAA);
    REQUIRE(v1.get<2>(0) == 0xBBBBBBBB);
    REQUIRE(v1.get<0>(1) == 'b');
    REQUIRE(v1.get<1>(1) == 0xCCCC);
    REQUIRE(v1.get<2>(1) == 0xDDDDDDDD);
    REQUIRE(v1.get<0>(2) == 'c');
    REQUIRE(v1.get<1>(2) == 0xEEEE);
    REQUIRE(v1.get<2>(2) == 0xFFFFFFFF);
    REQUIRE(v1.get<0>(3) == 'd');
    REQUIRE(v1.get<1>(3) == 0xABAB);
    REQUIRE(v1.get<2>(3) == 0xBCBCBCBC);
}
