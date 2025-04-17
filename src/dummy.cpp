#include <iostream>
#include <stdexcept>

void foo() 
{
    throw std::runtime_error("test error");
}

int main() 
{
    try {
        foo();
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    return 0;
}
