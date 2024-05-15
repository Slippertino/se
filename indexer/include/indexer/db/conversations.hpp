#pragma once

#include <chrono>
#include <string>
#include <algorithm>
#include <tuple>
#include <pqxx/composite>
#include <pqxx/strconv>
#include <pqxx/util>
#include <indexer/models/lexem.hpp>
#include <indexer/models/resource.hpp>

namespace pqxx {

template<> std::string const type_name<se::indexer::LexemEntry>{ "LexemEntry" };
template<> struct nullness<se::indexer::LexemEntry> : no_null<se::indexer::LexemEntry> {};
template<> 
struct string_traits<se::indexer::LexemEntry> {
    static se::indexer::LexemEntry from_string(std::string_view text) {
        return se::indexer::LexemEntry{};
    }

    static zview to_buf(char *begin, char *end, se::indexer::LexemEntry const &value) {
        return generic_to_buf(begin, end, value);
    }

    static char *into_buf(char *begin, char *end, se::indexer::LexemEntry const &value) {
        return composite_into_buf(
            begin, end, 
            value.lexem.value,
            value.lexem.lang.name(),
            value.wlf
        );
    }

    static size_t size_buffer(se::indexer::LexemEntry const &value) noexcept {
        return composite_size_buffer( 
            value.lexem.value,
            value.lexem.lang.name(),
            value.wlf
        );
    }
};

} // namespace pqxx
