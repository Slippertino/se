#include <tqp/token.hpp>

std::ostream& operator<<(std::ostream& os, const tqp::Token& token) {
    os 
        << "Token [" 
        << "pos=" << token.pos << ", "
        << "lang=" << token.lang.name() << ", "
        << "token=" << token.token << ", "
        << "source=" << token.source << "]";
    return os;
}