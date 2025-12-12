#include <stdexcept>

void foo(int i) {
    if(i == 5){
        throw std::invalid_argument("arg error");
    } else if(i == 1){
        throw 67;
    } else if(i == 0){
        throw std::runtime_error("error");
    } else if(i == 2){
        throw "error";
    } else if (i == 3){
        throw 6.7;
    }
}

void baa() {
    int i = 1;
    throw "not from foo";
    foo(i);
}

int main(){
    try{
        baa();
    } catch (...) {

    };
}