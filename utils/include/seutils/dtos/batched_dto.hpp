#pragma once

#include <iostream>
#include <deque>
#include <string>
#include <boost/format.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "../compression_helper.hpp"

namespace se {

namespace utils {

template<typename Type>
struct BatchedDTO {
    std::string compression_type;
    mutable std::deque<Type> data;
    ComperssionHelper::buffer_t& buffer;

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar & compression_type;
        std::stringstream inout;
        int count;
        for(count = 0; !data.empty(); ++count) {
            std::ostringstream ostr;
            boost::archive::text_oarchive oa{ ostr, boost::archive::no_header };
            oa << data.front();
            if (inout.tellp() + ostr.tellp() >= buffer.size())
                break;
            inout << ostr.str();
            data.pop_front();
        }
        int size = inout.str().size();
        ar & size;
        ar & count;
        std::string compressed, error;
        if (utils::ComperssionHelper::compress_in_memory(inout, compressed, compression_type, buffer, error))
            throw std::runtime_error(error.c_str());
        ar & compressed;
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int version) {
        int size, count;
        std::string compressed, decompressed;
        ar & compression_type;
        ar & size;
        ar & count;
        ar & compressed;
        std::string error;
        if (size > buffer.size())
            buffer.resize(size);
        if (utils::ComperssionHelper::decompress_in_memory(compressed, decompressed, buffer, error)) 
            throw std::runtime_error(
                (boost::format{ "error in decompression - %1%" } % error).str()
            );
        std::stringstream stream{ decompressed };
        data.clear();
        for(auto i = 0; i < count; ++i) {
            boost::archive::text_iarchive ia{ stream, boost::archive::no_header };
            Type rd;
            ia >> rd;
            data.push_back(std::move(rd));
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

} // namespace utils

} // namespace se