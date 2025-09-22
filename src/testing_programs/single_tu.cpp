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

    return 0;
}