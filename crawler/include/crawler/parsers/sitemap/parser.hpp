#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "types.hpp"

namespace se {

namespace crawler {

namespace parsers {

class SitemapParser final {
    class const_iterator;
    friend class const_iterator;

    using inner_iterator_t = boost::property_tree::ptree::const_iterator;

public:
    SitemapParser() = delete;
    SitemapParser(const std::string &content) {
        std::istringstream istr{content};
        boost::property_tree::read_xml(istr, obj_);
        for(const auto& v : obj_) {
            type_ = v.first;
            if (type_ != "<xmlcomment>")
                break;
        }
        handler_ = parsers_.at(type_);
        target_ = obj_.get_child(type_);
    }

    inline auto begin() const noexcept;
    inline auto end() const noexcept;

private:
    static PageChangeFrequencyType get_frequency_type(const std::string &freq_text) {
        return (pages_frequency_map.find(freq_text) != pages_frequency_map.end())
            ? pages_frequency_map.at(freq_text)
            : PageChangeFrequencyType::daily;
    }

    using default_converter_t = decltype([](const std::string &s) { return s; });

    template<typename Target, typename Field, typename Converter = default_converter_t>
    static inline void set(
        Target &target, const Field &field, const std::string &key, 
        const std::pair<const std::string, boost::property_tree::ptree> &node, 
        Converter&& converter = {}
    ) {
        if (node.first == key)
            std::invoke(field, target) = converter(node.second.data());
    }

    static SitemapAny parse_sitemapindex(inner_iterator_t it) {
        SitemapAny res;
        res.type = SitemapType::INDEX;

        auto &child = *it;
        for(auto &field : child.second) {
            set(res, &SitemapAny::loc, "loc", field);
            set(res, &SitemapAny::last_modified, "lastmod", field);
        }

        return res;
    }

    static SitemapAny parse_urlset(inner_iterator_t it) {
        SitemapAny res;
        res.type = SitemapType::URLSET;

        auto &child = *it;
        for(const auto &field : child.second) {
            set(res, &SitemapAny::loc, "loc", field);
            set(res, &SitemapAny::last_modified, "lastmod", field);
            set(res, &SitemapAny::change_freq, "changefreq", field, [](const std::string &s){ 
                return get_frequency_type(s); 
            });
            set(res, &SitemapAny::priority, "priority", field, [](const std::string &s) {
                return std::stod(s);
            });
        }

        return res;
    }

private:
    using handler_t = std::function<SitemapAny(inner_iterator_t)>;

    static inline const std::unordered_map<std::string, handler_t> parsers_ = {
        { "sitemapindex",   parse_sitemapindex  },
        { "urlset",         parse_urlset        },
    };

    static inline const std::unordered_map<std::string, std::string> element_marks_ = {
        { "sitemapindex", "sitemap" },
        { "urlset",       "url"     }
    };

    static inline const std::unordered_map<std::string, PageChangeFrequencyType> pages_frequency_map = {
        { "always",   PageChangeFrequencyType::always   },
        { "hourly",   PageChangeFrequencyType::hourly   },
        { "daily",    PageChangeFrequencyType::daily    },
        { "weekly",   PageChangeFrequencyType::weekly   },   
        { "monthly",  PageChangeFrequencyType::monthly  },
        { "yearly",   PageChangeFrequencyType::yearly   },
        { "never",    PageChangeFrequencyType::never    },
    };

private:
    std::string type_;
    handler_t handler_;
    boost::property_tree::ptree obj_;
    boost::property_tree::ptree &target_ = obj_;
};

class SitemapParser::const_iterator final {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = SitemapAny;
    using difference_type = std::ptrdiff_t;
    using reference = SitemapAny&;

public:
    const_iterator(
        const SitemapParser& parser, 
        inner_iterator_t begin, 
        inner_iterator_t end, 
        const std::string& mark
    ) : 
        parser_{parser}, 
        current_{ begin },
        end_ { end },
        mark_{ mark }
    { 
        move_until_mark();
    }

    value_type operator*() const noexcept {
        return parser_.handler_(current_);
    }

    const_iterator(const const_iterator& rhs) = default;
    const_iterator& operator=(const const_iterator&) = default;

    const_iterator& operator++() noexcept {
        ++current_;
        move_until_mark();
        return *this;
    }

    const_iterator operator++(int) noexcept {
        const_iterator copy{*this};
        ++current_;
        move_until_mark();
        return *this;
    }

    bool operator==(const const_iterator& rhs) const noexcept {
        return current_ == rhs.current_;
    }

private:
    inline void move_until_mark() {
        while(current_ != end_ && (*current_).first != mark_) 
            ++current_;
    }

private:
    const SitemapParser& parser_;
    inner_iterator_t current_;
    inner_iterator_t end_;
    std::string mark_;
};

inline auto SitemapParser::begin() const noexcept {
    return const_iterator{ *this, target_.begin(), target_.end(), element_marks_.at(type_) };
}

inline auto SitemapParser::end() const noexcept {
    return const_iterator{ *this, target_.end(), target_.end(), element_marks_.at(type_) };
}

} // namespace parsers

} // namespace crawler

} // namespace se