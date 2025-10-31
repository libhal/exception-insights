#include "demo_two.h"

void bar()
{
    throw 1;
}

void foo()
{
    bar();
}

int main()
{
    try {
        foo();
    } catch (...) {
    }

    return baz(2);
}