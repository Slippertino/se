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
class BatchedDTO {
private:
    friend class boost::serialization::access;

public:
    BatchedDTO(
        CompressionHelper::buffer_t& buffer, 
        size_t max_batch_size = 1, 
        const std::string& compression_type = no_compression_tag
    ) :
        max_batch_size_{ max_batch_size },
        compression_type_{ (!compression_type.empty() ? compression_type : no_compression_tag) },
        buffer_ref_{ buffer }
    { }

    void load_from_string(const std::string& data) {
        deserialize_internal(data, *this);
    }

    bool is_empty() const noexcept {
        return data_.empty();
    }

    bool is_full() const noexcept {
        return data_.size() >= max_batch_size_;
    }

    size_t max_batch_size() const noexcept {
        return max_batch_size_;
    }

    std::string compression_type() const noexcept {
        return compression_type_;
    }

    const std::deque<Type>& data() const {
        return data_;
    }

    void set_max_batch_size(size_t size) {
        max_batch_size_ = size;
    }

    void set_compression_type(const std::string& compression_type) {
        if (compression_type.empty())
            throw std::runtime_error("empty compression type");
        compression_type_ = compression_type;
    }

    std::string serialize() const {
        return serialize_internal(*this);
    }

    bool try_add_data(const Type& data) const {
        if (serialize_internal(data).size() > buffer_ref_.size())
            return false;
        data_.push_back(data);
        return true;
    }

    template<class Archive>
    void save(Archive& ar, const unsigned int) const {
        ar & compression_type_;
        std::stringstream inout;
        int count, size{ 0 };
        for(count = 0; !data_.empty(); ++count) {
            auto cur = serialize_internal(data_.front());
            if (size + cur.size() >= buffer_ref_.size())
                break;
            inout << cur;
            size += cur.size();
            data_.pop_front();
        }
        ar & size;
        ar & count;
        std::string compressed, error;
        if (compression_type_ == no_compression_tag || !size)
            compressed = inout.str();
        else if (utils::CompressionHelper::compress_in_memory(inout, compressed, compression_type_, buffer_ref_, error))
            throw std::runtime_error(error.c_str());
        ar & compressed;
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int) {
        int size, count;
        std::string compressed, decompressed;
        ar & compression_type_;
        ar & size;
        ar & count;
        ar & compressed;
        data_.resize(count);
        if (!count)
            return; 
        if (size > buffer_ref_.size())
            buffer_ref_.resize(size);
        std::string error;
        if (compression_type_ == no_compression_tag)
            decompressed = std::move(compressed);
        else if (utils::CompressionHelper::decompress_in_memory(compressed, decompressed, buffer_ref_, error)) 
            throw std::runtime_error(
                (boost::format{ "error in decompression - %1%" } % error).str()
            );
        std::istringstream stream{ decompressed };
        for(auto i = 0; i < count; ++i)
            deserialize_internal(stream, data_[i]);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    template<typename T>
    static std::string serialize_internal(const T& obj) {
        std::ostringstream buffer;
        boost::archive::text_oarchive oa{ buffer, boost::archive::no_header };
        oa << obj;
        return buffer.str();
    }

    template<typename T>
    static void deserialize_internal(std::istringstream& in, T& target) {
        boost::archive::text_iarchive ia(in, boost::archive::no_header);
        ia >> target;
    }

    template<typename T>
    static void deserialize_internal(const std::string& data, T& target) {
        std::istringstream in{ data };
        deserialize_internal(in, target);
    }

public:
    static constexpr std::string_view no_compression_tag = "nocompression";

private:
    size_t max_batch_size_;
    std::string compression_type_;
    mutable std::deque<Type> data_;
    CompressionHelper::buffer_t& buffer_ref_; 
};

} // namespace utils

} // namespace se