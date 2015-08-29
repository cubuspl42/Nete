#include "catch.hpp"

#include <nete/Signal.h>

#include <sstream>

std::stringstream ss;

void global_function1(int a, int b) {
    ss << "global_function1 " << a << " " << b << " ";
}

void global_function2(int a, int b) {
    ss << "global_function2 " << a << " " << b << " ";
}

struct TestClass {
    int _n;
    
    TestClass(int n) : _n(n) {}
    
    void foo(int a, int b) {
        ss << "TestClass::foo " << _n << " " << a << " " << b << " ";
    }
    
    void bar(double a, int b, char c) {
        ss << "TestClass::bar " << _n << " " << a << " " << b << " " << c << " ";
    }
};

struct TestObserver : nete::Observer {
    int _n;
    
    TestObserver(int n) : _n(n) {}
    
    void foo(int a, int b) {
        ss << "TestObserver::foo " << _n << " " << a << " " << b << " ";
    }
};

TEST_CASE("Signal connect", "[signal]")
{
    SECTION("global")
    {
        ss.clear();
        ss.str("");
        
        nete::Signal<void(int, int)> s;
        
        s.connect<global_function1>();
        s.connect<global_function2>();
        
        s(123, 456);
        s(234, 567);
        
        std::string expected =
        "global_function1 123 456 "
        "global_function2 123 456 "
        "global_function1 234 567 "
        "global_function2 234 567 ";
        
        REQUIRE(ss.str() == expected);
    }
    
    SECTION("method")
    {
        ss.clear();
        ss.str("");
        
        nete::Signal<void(int, int)> s;
        
        TestClass t1 { 1 };
        TestClass t2 { 2 };
        
        s.connect<TestClass, &TestClass::foo>(t1);
        s.connect<TestClass, &TestClass::foo>(t2);

        s(123, 456);
        s(234, 567);
        
        std::string expected =
        "TestClass::foo 1 123 456 "
        "TestClass::foo 2 123 456 "
        "TestClass::foo 1 234 567 "
        "TestClass::foo 2 234 567 ";
        
        REQUIRE(ss.str() == expected);
    }
}

TEST_CASE("Signal disconnect", "[signal]")
{
    SECTION("global")
    {
        ss.clear();
        ss.str("");
        
        nete::Signal<void(int, int)> s;
        
        s.connect<global_function1>();
        s.connect<global_function2>();
        
        s(123, 456);
        s(234, 567);
        
        s.disconnect<global_function1>();
        
        s(123, 456);
        s(234, 567);
        
        s.disconnect<global_function2>();
        
        s(000, 000);

        std::string expected =
        "global_function1 123 456 "
        "global_function2 123 456 "
        "global_function1 234 567 "
        "global_function2 234 567 "
        "global_function2 123 456 "
        "global_function2 234 567 ";
        
        REQUIRE(ss.str() == expected);
    }
    
    SECTION("method")
    {
        ss.clear();
        ss.str("");
        
        nete::Signal<void(int, int)> s;
        
        TestClass t1 { 1 };
        TestClass t2 { 2 };
        
        s.connect<TestClass, &TestClass::foo>(t1);
        s.connect<TestClass, &TestClass::foo>(t2);
        
        s(123, 456);
        s(234, 567);
        
        s.disconnect<TestClass, &TestClass::foo>(t1);
        
        s(123, 456);
        s(234, 567);
        
        s.disconnect<TestClass, &TestClass::foo>(t2);
        
        s(000, 000);
        
        std::string expected =
        "TestClass::foo 1 123 456 "
        "TestClass::foo 2 123 456 "
        "TestClass::foo 1 234 567 "
        "TestClass::foo 2 234 567 "
        "TestClass::foo 2 123 456 "
        "TestClass::foo 2 234 567 ";
        
        REQUIRE(ss.str() == expected);
    }
    
}

TEST_CASE("Observer", "[signal]")
{
    ss.clear();
    ss.str("");
    
    nete::Signal<void(int, int)> s;
    
    std::unique_ptr<TestObserver> t1 { new TestObserver { 1 } };
    TestObserver t2 { 2 };
    
    s.connect<TestObserver, &TestObserver::foo>(*t1);
    s.connect<TestObserver, &TestObserver::foo>(t2);
    
    {
        TestObserver t3 { 3 };
        s.connect<TestObserver, &TestObserver::foo>(t3);
        
        s(123, 456);
    }
    
    s(234, 567);
    
    t1.reset();
    
    TestObserver t4 { 4 };
    
    s.connect<TestObserver, &TestObserver::foo>(t4);
    
    s(345, 678);
    
    s.disconnect<TestObserver, &TestObserver::foo>(t4);
    
    s(456, 789);
    
    std::string expected =
    "TestObserver::foo 1 123 456 "
    "TestObserver::foo 2 123 456 "
    "TestObserver::foo 3 123 456 "
    "TestObserver::foo 1 234 567 "
    "TestObserver::foo 2 234 567 "
    "TestObserver::foo 2 345 678 "
    "TestObserver::foo 4 345 678 "
    "TestObserver::foo 2 456 789 ";
    
    REQUIRE(ss.str() == expected);
}

struct TestEmitter {
    template<typename T> using signal = nete::ObjectSignal<TestEmitter, T>;
    
    void emit_foo(int a, int b) {
        foo(a, b);
    }
    
    void emit_bar(double a, int b, char c) {
        bar(a, b, c);
    }
    
    signal<void(int, int)> foo;
    signal<void(double, int, char)> bar;
};

TEST_CASE("ObjectSignal", "[signal]")
{
    ss.clear();
    ss.str("");
    
    TestEmitter e1;
    TestClass t1 { 1 };
    
    e1.foo.connect<TestClass, &TestClass::foo>(t1);
    e1.bar.connect<TestClass, &TestClass::bar>(t1);
    
    e1.emit_foo(123, 456);
    e1.emit_bar(1.23, 456, 'a');
    
    std::string expected =
    "TestClass::foo 1 123 456 "
    "TestClass::bar 1 1.23 456 a ";
    
    REQUIRE(ss.str() == expected);
}
