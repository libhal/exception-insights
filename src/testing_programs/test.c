void bar()
{
    int a = 1 + 2;
    (void)a;
}

void foo()
{
    bar();
}

int main()
{
    foo();
    return 0;
}