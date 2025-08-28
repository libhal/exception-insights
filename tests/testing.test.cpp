#include <boost/ut.hpp>

boost::ut::suite<"testing"> testing_existance = []() {
    using namespace boost::ut;
    "input_pin interface test"_test = []() {
        auto r = 1 + 1;
        expect(that % 2 == r);
    };
};