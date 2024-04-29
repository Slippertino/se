#pragma once

#include <iostream>
#include <deque>
#include <string>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <crawler/utils/compression_helper.hpp>

namespace crawler {

namespace utils {

template<typename Type>
struct BatchedDTO {
    std::string compression_type;
    mutable std::deque<Type> data;
    utils::ComperssionHelper::buffer_t& buffer;

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar & compression_type;
        std::stringstream inout;
        int count;
        for(count = 0; !data.empty(); ++count) {
            std::ostringstream ostr;
            boost::archive::text_oarchive oa{ ostr };
            oa << data.front();
            if (inout.tellp() + ostr.tellp() >= buffer.size())
                break;
            inout << ostr.str();
            data.pop_front();
        }
        ar & count;
        std::string compressed, error;
        if (utils::ComperssionHelper::compress_in_memory(inout, compressed, compression_type, buffer, error))
            throw std::runtime_error(error.c_str());
        ar & compressed;
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int version) {
        int count;
        std::string compressed, decompressed;
        ar & compression_type;
        ar & count;
        ar & compressed;
        std::string error;
        if (utils::ComperssionHelper::decompress_in_memory(compressed, decompressed, buffer, error)) 
            throw std::runtime_error(error.c_str());
        std::stringstream stream{ decompressed };
        boost::archive::text_iarchive ia{ stream };
        data.clear();
        for(auto i = 0; i < count; ++i) {
            Type rd;
            ia >> rd;
            data.push_back(std::move(rd));
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

} // namespace utils 

} // namespace crawler