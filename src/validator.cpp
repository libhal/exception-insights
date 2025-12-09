#include "validator.hpp"

namespace safe {
Validator::Validator(symbol_s p_sym, std::span<std::byte> p_text) :
    m_sym(p_sym), m_text(p_text) {};

void Validator::find_throw_type(std::string_view sym_name){
    std::variant<uint32_t, uint64_t> address;
    for (auto & s : sym){
        if(s.name == sym_name){
            address = s.value;
        }
    }

    
}    

}