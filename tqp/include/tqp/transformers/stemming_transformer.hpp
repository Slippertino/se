#pragma once

#include <stdexcept>
#include <boost/format.hpp>
#include <tqp/token.hpp>
#include <tqp/tools/stemmer.hpp>
#include <tqp/tools/threaded_resource.hpp>

namespace tqp {

using StemmersMap = std::unordered_map<Language, Stemmer>;

struct StemmingTransformer final : public ThreadedResource<StemmingTransformer, StemmersMap> {

    StemmingTransformer() = default;

    void operator()(TokenSequence& seq, Language lang);

    template<std::same_as<StemmersMap> R>
    StemmersMap create_thread_resource() {
        StemmersMap map;
        for(const auto& lang : Stemmer::get_available_languages())
            map.insert({ lang, Stemmer(lang) });
        return map;
    }
};

} // namespace tqp