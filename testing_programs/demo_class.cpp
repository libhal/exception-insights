#include "demo_two.h"
struct A
{
    int a = 2;
    int method()
    {
        throw "string";
        return a;
    }
};

void bar()
{
    throw 2;
}

void foo()
{
    auto f = [] {};
    f();
    bar();
}

int main()
{
    A a;
    baz(2);
    try {
        a.method();
        foo();
    } catch (...) {
        return 1;
    }
    return 0;
}