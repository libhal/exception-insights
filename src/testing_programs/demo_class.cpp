struct A
{
    int a = 2;
    int method()
    {
        return a;
    }
};

void bar()
{
    throw 2;
}

void foo()
{
    bar();
}

int main()
{
    A a;
    a.method();
    try {
        foo();
    } catch (...) {
    }
    return 0;
}