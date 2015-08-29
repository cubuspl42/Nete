#include "catch.hpp"

#include <nete/nete.h>
#include <nete/tl/multivector.h>

#include <string>

TEST_CASE("multivector construction", "[multivector]") {
  using namespace nete::tl;

  SECTION("default constructor") {
    multivector<types<char, uint16_t, std::string, std::unique_ptr<int>>> v;

    REQUIRE(v.size() == 0);
    REQUIRE(v.capacity() == 0);
  }

  SECTION("fill constructor") {
    int size = 8;

    multivector<types<char, uint16_t, std::string, std::unique_ptr<int>>> v(
        size);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == char{});
      REQUIRE(v.at<1>(i) == uint16_t{});
      REQUIRE(v.at<2>(i) == std::string{});
      REQUIRE(v.at<3>(i) == std::unique_ptr<int>{});
    }
  }

  SECTION("fill constructor with copy") {
    int size = 8;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string>> v(size, e0, e1, e2);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }
  }

  SECTION("copy constructor") {
    int size = 8;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string>> v(size, e0, e1, e2);
    multivector<types<char, uint16_t, std::string>> v_prim(v);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }

    REQUIRE(v_prim.size() == size);
    REQUIRE(v_prim.capacity() >= size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v_prim.at<0>(i) == e0);
      REQUIRE(v_prim.at<1>(i) == e1);
      REQUIRE(v_prim.at<2>(i) == e2);
    }
  }

  SECTION("move constructor") {
    int size = 1;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string, std::unique_ptr<int>>> v(
        size);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.at<0>(0) = e0;
    v.at<1>(0) = e1;
    v.at<2>(0) = e2;
    v.at<3>(0) = std::unique_ptr<int>{new int{}};

    const char *v_storage = v.storage();
    const char *string_data = v.at<2>(0).data();
    int *int_pointer = v.at<3>(0).get();

    multivector<types<char, uint16_t, std::string, std::unique_ptr<int>>>
        v_prim(std::move(v));

    REQUIRE(v_prim.size() == size);
    REQUIRE(v_prim.capacity() >= size);
    REQUIRE(v_storage == v_prim.storage());
    REQUIRE(string_data == v_prim.at<2>(0).data());
    REQUIRE(int_pointer == v_prim.at<3>(0).get());
  }
}

TEST_CASE("multivector assignment", "[multivector]") {
  using namespace nete::tl;

  SECTION("copy assignment") {
    int size = 8;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string>> v(size, e0, e1, e2);
    multivector<types<char, uint16_t, std::string>> v_prim;
    v_prim = v;

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }

    REQUIRE(v_prim.size() == size);
    REQUIRE(v_prim.capacity() >= size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v_prim.at<0>(i) == e0);
      REQUIRE(v_prim.at<1>(i) == e1);
      REQUIRE(v_prim.at<2>(i) == e2);
    }
  }

  SECTION("move assignment") {
    int size = 1;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string, std::unique_ptr<int>>> v(
        size);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.at<0>(0) = e0;
    v.at<1>(0) = e1;
    v.at<2>(0) = e2;
    v.at<3>(0) = std::unique_ptr<int>{new int{}};

    const char *v_storage = v.storage();
    const char *string_data = v.at<2>(0).data();
    int *int_pointer = v.at<3>(0).get();

    multivector<types<char, uint16_t, std::string, std::unique_ptr<int>>>
        v_prim;
    v_prim = std::move(v);

    REQUIRE(v_prim.size() == size);
    REQUIRE(v_prim.capacity() >= size);
    REQUIRE(v_prim.at<0>(0) == e0);
    REQUIRE(v_prim.at<1>(0) == e1);
    REQUIRE(v_prim.at<2>(0) == e2);
    REQUIRE(v_storage == v_prim.storage());
    REQUIRE(string_data == v_prim.at<2>(0).data());
    REQUIRE(int_pointer == v_prim.at<3>(0).get());
  }
}

TEST_CASE("multivector resizing", "[multivector]") {
  using namespace nete::tl;

  SECTION("reserve") {
    int size = 1;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::vector<int> e2{1, 2, 3, 4};

    multivector<types<char, uint16_t, std::vector<int>, std::unique_ptr<int>>>
        v(size);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.at<0>(0) = e0;
    v.at<1>(0) = e1;
    v.at<2>(0) = e2;
    v.at<3>(0) = std::unique_ptr<int>{new int{}};

    const char *v_storage = v.storage();
    const int *vector_data = v.at<2>(0).data();
    int *int_pointer = v.at<3>(0).get();

    int new_capacity = 8;
    REQUIRE(new_capacity > v.capacity());

    v.reserve(new_capacity);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= new_capacity);
    REQUIRE(v.at<0>(0) == e0);
    REQUIRE(v.at<1>(0) == e1);
    REQUIRE(v.at<2>(0) == e2);
    REQUIRE(v_storage != v.storage());
    REQUIRE(vector_data == v.at<2>(0).data());
    REQUIRE(int_pointer == v.at<3>(0).get());

    int final_capacity = 4;

    v.reserve(new_capacity);

    REQUIRE(v.capacity() >= new_capacity);
  }

  SECTION("resize") {
    int size = 4;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string>> v(size, e0, e1, e2);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    int new_size = 16;
    v.resize(new_size);

    REQUIRE(v.size() == new_size);
    REQUIRE(v.capacity() >= new_size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }

    for (int i = size; i < new_size; ++i) {
      REQUIRE(v.at<0>(i) == char{});
      REQUIRE(v.at<1>(i) == uint16_t{});
      REQUIRE(v.at<2>(i) == std::string{});
    }

    int newer_size = 16;
    v.resize(newer_size, e0, e1, e2);

    REQUIRE(v.size() == newer_size);
    REQUIRE(v.capacity() >= newer_size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }

    for (int i = size; i < new_size; ++i) {
      REQUIRE(v.at<0>(i) == char{});
      REQUIRE(v.at<1>(i) == uint16_t{});
      REQUIRE(v.at<2>(i) == std::string{});
    }

    for (int i = new_size; i < newer_size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }

    int final_size = 8;
    v.resize(final_size);

    REQUIRE(v.size() == final_size);
    REQUIRE(v.capacity() >= newer_size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }

    for (int i = size; i < final_size; ++i) {
      REQUIRE(v.at<0>(i) == char{});
      REQUIRE(v.at<1>(i) == uint16_t{});
      REQUIRE(v.at<2>(i) == std::string{});
    }
  }

  SECTION("push_back") {
    int size = 4;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string>> v(size, e0, e1, e2);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    char e0_ = 'b';
    uint16_t e1_ = 32;
    std::string e2_ = "world";

    v.push_back(e0_, e1_, e2_);

    REQUIRE(v.size() == size + 1);
    REQUIRE(v.capacity() >= size + 1);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }

    REQUIRE(v.at<0>(size) == e0_);
    REQUIRE(v.at<1>(size) == e1_);
    REQUIRE(v.at<2>(size) == e2_);
  }

  SECTION("emplace_back") {
    int size = 4;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string>> v(size, e0, e1, e2);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.emplace_back();

    REQUIRE(v.size() == size + 1);
    REQUIRE(v.capacity() >= size + 1);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }

    REQUIRE(v.at<0>(size) == char{});
    REQUIRE(v.at<1>(size) == uint16_t{});
    REQUIRE(v.at<2>(size) == std::string{});
  }

  SECTION("pop_back") {
    int size = 4;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string>> v(size, e0, e1, e2);

    const char *v_storage = v.storage();

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.pop_back();

    REQUIRE(v.size() == size - 1);
    REQUIRE(v.capacity() >= size);
    REQUIRE(v_storage == v.storage());

    for (int i = 0; i < size - 1; ++i) {
      REQUIRE(v.at<0>(i) == e0);
      REQUIRE(v.at<1>(i) == e1);
      REQUIRE(v.at<2>(i) == e2);
    }
  }

  SECTION("clear") {
    int size = 4;
    char e0 = 'a';
    uint16_t e1 = 16;
    std::string e2 = "hello";

    multivector<types<char, uint16_t, std::string>> v(size, e0, e1, e2);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.clear();

    REQUIRE(v.size() == 0);
    REQUIRE(v.capacity() >= size);
  }
}

TEST_CASE("multivector iteration", "[multivector]") {
  using namespace nete::tl;

  SECTION("index") {
    int size = 4;

    multivector<types<char, uint16_t, std::string>> v(size);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.at<0>(0) = 'x';
    v.at<1>(0) = 100;
    v.at<2>(0) = "aaa";
    v.at<0>(1) = 'y';
    v.at<1>(1) = 200;
    v.at<2>(1) = "bbb";
    v.at<0>(2) = 'z';
    v.at<1>(2) = 300;
    v.at<2>(2) = "ccc";
    v.at<0>(3) = 'w';
    v.at<1>(3) = 400;
    v.at<2>(3) = "ddd";

    for (int i = 0; i < size; ++i) {
      switch (i) {
      case 0:
        REQUIRE(v.at<0>(i) == 'x');
        REQUIRE(v.at<1>(i) == 100);
        REQUIRE(v.at<2>(i) == "aaa");
        break;
      case 1:
        REQUIRE(v.at<0>(i) == 'y');
        REQUIRE(v.at<1>(i) == 200);
        REQUIRE(v.at<2>(i) == "bbb");
        break;
      case 2:
        REQUIRE(v.at<0>(i) == 'z');
        REQUIRE(v.at<1>(i) == 300);
        REQUIRE(v.at<2>(i) == "ccc");
        break;
      case 3:
        REQUIRE(v.at<0>(i) == 'w');
        REQUIRE(v.at<1>(i) == 400);
        REQUIRE(v.at<2>(i) == "ddd");
        break;
      }
    }
  }

  SECTION("iterator") {
    int size = 4;

    multivector<types<char, uint16_t, std::string>> v(size);
    using multivector_type = decltype(v);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.at<0>(0) = 'x';
    v.at<1>(0) = 100;
    v.at<2>(0) = "aaa";
    v.at<0>(1) = 'y';
    v.at<1>(1) = 200;
    v.at<2>(1) = "bbb";
    v.at<0>(2) = 'z';
    v.at<1>(2) = 300;
    v.at<2>(2) = "ccc";
    v.at<0>(3) = 'w';
    v.at<1>(3) = 400;
    v.at<2>(3) = "ddd";

    for (multivector_type::iterator it = v.begin(); it != v.end(); ++it) {
      switch (it - v.begin()) {
      case 0:
        REQUIRE(v.get<0>(it) == 'x');
        REQUIRE(v.get<1>(it) == 100);
        REQUIRE(v.get<2>(it) == "aaa");
        break;
      case 1:
        REQUIRE(v.get<0>(it) == 'y');
        REQUIRE(v.get<1>(it) == 200);
        REQUIRE(v.get<2>(it) == "bbb");
        break;
      case 2:
        REQUIRE(v.get<0>(it) == 'z');
        REQUIRE(v.get<1>(it) == 300);
        REQUIRE(v.get<2>(it) == "ccc");
        break;
      case 3:
        REQUIRE(v.get<0>(it) == 'w');
        REQUIRE(v.get<1>(it) == 400);
        REQUIRE(v.get<2>(it) == "ddd");
        break;
      }
    }
  }

  SECTION("reverse_iterator") {
    int size = 4;

    multivector<types<char, uint16_t, std::string>> v(size);
    using multivector_type = decltype(v);

    REQUIRE(v.size() == size);
    REQUIRE(v.capacity() >= size);

    v.at<0>(0) = 'x';
    v.at<1>(0) = 100;
    v.at<2>(0) = "aaa";
    v.at<0>(1) = 'y';
    v.at<1>(1) = 200;
    v.at<2>(1) = "bbb";
    v.at<0>(2) = 'z';
    v.at<1>(2) = 300;
    v.at<2>(2) = "ccc";
    v.at<0>(3) = 'w';
    v.at<1>(3) = 400;
    v.at<2>(3) = "ddd";

    for (multivector_type::reverse_iterator it = v.rbegin(); it != v.rend();
         ++it) {
      switch (it - v.rbegin()) {
      case 3:
        REQUIRE(v.get<0>(it) == 'x');
        REQUIRE(v.get<1>(it) == 100);
        REQUIRE(v.get<2>(it) == "aaa");
        break;
      case 2:
        REQUIRE(v.get<0>(it) == 'y');
        REQUIRE(v.get<1>(it) == 200);
        REQUIRE(v.get<2>(it) == "bbb");
        break;
      case 1:
        REQUIRE(v.get<0>(it) == 'z');
        REQUIRE(v.get<1>(it) == 300);
        REQUIRE(v.get<2>(it) == "ccc");
        break;
      case 0:
        REQUIRE(v.get<0>(it) == 'w');
        REQUIRE(v.get<1>(it) == 400);
        REQUIRE(v.get<2>(it) == "ddd");
        break;
      }
    }
  }
}

TEST_CASE("multivector swapping", "[multivector]") {
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

TEST_CASE("multivector consistency", "[multivector]") {
  using namespace nete::tl;

  int size = 3;

  multivector<types<char, uint16_t>> v(size);

  v.at<0>(0) = 'x';
  v.at<1>(0) = 100;
  v.at<0>(1) = 'y';
  v.at<1>(1) = 200;
  v.at<0>(2) = 'z';
  v.at<1>(2) = 300;

  REQUIRE(v.get<0>(v.begin() + 0) == 'x');
  REQUIRE(v.get<1>(v.begin() + 0) == 100);
  REQUIRE(v.get<0>(v.begin() + 1) == 'y');
  REQUIRE(v.get<1>(v.begin() + 1) == 200);
  REQUIRE(v.get<0>(v.begin() + 2) == 'z');
  REQUIRE(v.get<1>(v.begin() + 2) == 300);

  REQUIRE(v.data<0>()[0] == 'x');
  REQUIRE(v.data<1>()[0] == 100);
  REQUIRE(v.data<0>()[1] == 'y');
  REQUIRE(v.data<1>()[1] == 200);
  REQUIRE(v.data<0>()[2] == 'z');
  REQUIRE(v.data<1>()[2] == 300);
}

TEST_CASE("multivector alignment", "[multivector]") {
  using namespace nete::tl;

  CHECK(sizeof(int) == 4);
  CHECK(sizeof(double) == 8);

  if (sizeof(int) == 4 && sizeof(double) == 8) {
    multivector<types<char, int, double>> v(2, 'a', 123, 456.0);
    v.reserve(5);

    const char *storage = v.storage();

    void *d0 = v.data<0>();
    void *d1 = v.data<1>();
    void *d2 = v.data<2>();

    void *d0_ = d0;
    void *d1_ = d1;
    void *d2_ = d2;

    REQUIRE(reinterpret_cast<char *>(d0_) - storage == 0);
    REQUIRE(reinterpret_cast<char *>(d1_) - storage == 8);
    REQUIRE(reinterpret_cast<char *>(d2_) - storage == 32);
  }
}

class fast_multivector_allocator : public std::allocator<char> {
  using base = std::allocator<char>;

public:
  using pointer = typename base::pointer;

  pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0) {
    pointer buffer = base::allocate(n, hint);
    std::fill(buffer, buffer + n, 0xFF);
    return buffer;
  }
};

struct fast_multivector_traits {
  using allocator_type = fast_multivector_allocator;
  using size_type = size_t;
  static constexpr bool disable_initialization = true;
};

template <typename Types>
using fast_multivector = nete::tl::multivector<Types, fast_multivector_traits>;

TEST_CASE("multivector initialization", "[multivector]") {
  SECTION("initialization enabled") {
    using namespace nete::tl;

    int size = 8;

    multivector<types<char, uint16_t>> v(size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == char{});
      REQUIRE(v.at<1>(i) == uint16_t{});
    }

    int new_size = 16;

    v.resize(new_size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) == char{});
      REQUIRE(v.at<1>(i) == uint16_t{});
    }

    int final_size = 32;
    v.resize(final_size, 'a', 16);

    for (int i = 0; i < new_size; ++i) {
      REQUIRE(v.at<0>(i) == char{});
      REQUIRE(v.at<1>(i) == uint16_t{});
    }
    for (int i = new_size; i < final_size; ++i) {
      REQUIRE(v.at<0>(i) == 'a');
      REQUIRE(v.at<1>(i) == 16);
    }
  }

  SECTION("initialization disabled") {
    using nete::tl::types;

    int size = 8;

    fast_multivector<types<char, uint16_t>> v(size);

    for (int i = 0; i < size; ++i) {
      REQUIRE(v.at<0>(i) != char{});
      REQUIRE(v.at<1>(i) != uint16_t{});
    }

    int new_size = 16;
    v.resize(new_size);

    for (int i = 0; i < new_size; ++i) {
      REQUIRE(v.at<0>(i) != char{});
      REQUIRE(v.at<1>(i) != uint16_t{});
    }

    int final_size = 32;
    v.resize(final_size, 'a', 16);

    for (int i = 0; i < new_size; ++i) {
      REQUIRE(v.at<0>(i) != char{});
      REQUIRE(v.at<1>(i) != uint16_t{});
    }
    for (int i = new_size; i < final_size; ++i) {
      REQUIRE(v.at<0>(i) == 'a');
      REQUIRE(v.at<1>(i) == 16);
    }
  }
}
