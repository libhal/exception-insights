void foo() {
    throw 67;
}

void baa() {
    foo();
}

int main(){
    try{
        baa();
    } catch (...) {

    };
}