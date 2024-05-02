#include <seutils/compression_helper.hpp>

namespace se {

namespace utils {

int ComperssionHelper::compress_in_memory(
    std::istream& source,
    std::ostream& dest,
    const format_t& format,
    buffer_t& buffer,
    std::string& error_desc
) {
    auto ar = archive_write_new();
    archive_write_set_format_raw(ar);
    if (archive_write_add_filter_by_name(ar, format.c_str())) 
        return fail(ar, error_desc);    
    auto in = read_input(source);
    size_t size;
    if (archive_write_open_memory(ar, buffer.data(), buffer.size(), &size)) 
        return fail(ar, error_desc);   
    auto ae = archive_entry_new();
    archive_entry_set_filetype(ae, AE_IFREG);
    archive_write_header(ar, ae);
    auto res_size = archive_write_data(ar, in.data(), in.size());
    if (res_size < 0)
        return fail(ar, error_desc);
    if (!res_size) {
        error_desc = "not managed to compress data";
        return 1;
    }
    archive_entry_free(ae);
    archive_write_close(ar);
    archive_write_free(ar);
    std::copy(buffer.begin(), std::next(buffer.begin(), size), std::ostream_iterator<char>(dest));
    return ARCHIVE_OK;
}

int ComperssionHelper::compress_in_memory(
    std::istream& source,
    std::string& dest,
    const format_t& format,
    buffer_t& buffer,
    std::string& error_desc
) {
    std::ostringstream out;
    auto res = compress_in_memory(source, out, format, buffer, error_desc);
    dest = out.str();
    return res;
}

int ComperssionHelper::compress_in_memory(
    const std::string& source,
    std::string& dest, 
    const format_t& format,
    buffer_t& buffer,
    std::string& error_desc
) {
    std::istringstream in{source};
    std::ostringstream out;
    auto res = compress_in_memory(in, out, format, buffer, error_desc);
    dest = out.str();
    return res;
}

int ComperssionHelper::decompress_in_memory(
    std::istream& source,
    std::ostream& dest,
    buffer_t& buffer,
    std::string& error_desc
) {
    auto ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_raw(ar);
    auto in = read_input(source);
    if (archive_read_open_memory(ar, in.data(), in.size()))
        return fail(ar, error_desc);       
    struct archive_entry* ae;
    int size;
    if (archive_read_next_header(ar, &ae))
        return fail(ar, error_desc);
    size = archive_read_data(ar, buffer.data(), buffer.size());
    if (size < 0) 
        return fail(ar, error_desc);
    if (!size) {
        error_desc = "not managed to decompress data";
        return 1;    
    }
    archive_read_free(ar);
    std::copy(buffer.begin(), std::next(buffer.begin(), size), std::ostream_iterator<char>(dest));
    return ARCHIVE_OK;
}

int ComperssionHelper::decompress_in_memory(
    const std::string& source,
    std::string& dest,
    buffer_t& buffer,
    std::string& error_desc
) {
    std::istringstream in{source};
    std::ostringstream out;
    auto res = decompress_in_memory(in, out, buffer, error_desc);
    dest = out.str();
    return res;
}

ComperssionHelper::buffer_t ComperssionHelper::read_input(std::istream& source) {
    source.seekg(0, std::ios::end);
    auto sz = source.tellg();
    buffer_t buf(sz);
    source.seekg(0, std::ios::beg);
    source.read(buf.data(), sz);
    return buf;
}

int ComperssionHelper::fail(struct archive* ar, std::string& error_desc) {
    error_desc = archive_error_string(ar);
    return archive_errno(ar);
}

} // namespace utils

} // namespace se