#include "gcc_parse.hpp"
#include <print>
#include <ranges>
#include <stdexcept>

namespace rng = std::ranges;
namespace views = rng::views;

using string = std::string;
using std::operator""sv;

int main()
{
    try {
        parse("/Users/phazonic/Documents/Programming/exception-insights/"
              "demo_class.wpa.081i.whole-program");
    } catch (std::runtime_error& e) {
        std::println("Error: {}", e.what());
        return 1;
    }

    return 0;
}