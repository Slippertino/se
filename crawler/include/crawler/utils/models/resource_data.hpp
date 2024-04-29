#pragma once

#include <string>
#include <boost/url/url.hpp>
#include <boost/serialization/split_member.hpp>

namespace crawler {

namespace utils {
    
struct CrawledResourceData final {
    boost::urls::url url;
    std::string content;

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        auto surl = std::string{ url.buffer() };
        ar & surl;
        ar & content;
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int version) {
        std::string surl;
        ar & surl;
        url = boost::url{ surl };
        ar & content;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

} // namespace utils

} // namespace crawler 