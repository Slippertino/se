#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <archive.h>
#include <archive_entry.h>

namespace se {

namespace utils {

class CompressionHelper {
public:
    using format_t = std::string;
    using buffer_t = std::vector<char>;

public:
    static int compress_in_memory(
        std::istream& source,
        std::ostream& dest,
        const format_t& format,
        buffer_t& buffer,
        std::string& error_desc
    );

    static int compress_in_memory(
        std::istream& source,
        std::string& dest,
        const format_t& format,
        buffer_t& buffer,
        std::string& error_desc
    );

    static int compress_in_memory(
        const std::string& source,
        std::string& dest, 
        const format_t& format,
        buffer_t& buffer,
        std::string& error_desc
    );

    static int decompress_in_memory(
        std::istream& source,
        std::ostream& dest,
        buffer_t& buffer,
        std::string& error_desc
    );

    static int decompress_in_memory(
        const std::string& source,
        std::string& dest,
        buffer_t& buffer,
        std::string& error_desc
    );

    template<typename Callback>
    static int decompress_complex(
        std::istream& source, 
        buffer_t& buffer,
        std::string& error_desc,
        Callback&& cb
    ) {
        auto ar = archive_read_new();
        archive_read_support_filter_all(ar);
        archive_read_support_format_all(ar);
        archive_read_support_format_empty(ar);
        archive_read_support_format_raw(ar);
        auto in = read_input(source);
        auto on_read = std::forward<Callback>(cb);
        if (archive_read_open_memory(ar, in.data(), in.size())) 
            return fail(ar, error_desc);
        struct archive_entry *ae;
        int size;
        for (;;) {
            if (archive_read_next_header(ar, &ae))
                break;
            size = archive_read_data(ar, buffer.data(), buffer.size());
            if (size < 0)
                return fail(ar, error_desc);
            if (!size)
                break;
            on_read(
                std::string(archive_entry_pathname(ae)), 
                std::string(buffer.begin(), std::next(buffer.begin(), size))
            );
        }
        archive_read_free(ar);
        return ARCHIVE_OK;
    }

private:
    static buffer_t read_input(std::istream& source);
    static int fail(struct archive* ar, std::string& error_desc);
};

} // namespace utils

} // namespace se