#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "types.hpp"

namespace crawler {

namespace sitemap {

namespace {

inline const std::unordered_map<std::string, PageChangeFrequencyType> pages_frequency_map = {
    { "always",   PageChangeFrequencyType::always   },
    { "hourly",   PageChangeFrequencyType::hourly   },
    { "daily",    PageChangeFrequencyType::daily    },
    { "weekly",   PageChangeFrequencyType::weekly   },   
    { "monthly",  PageChangeFrequencyType::monthly  },
    { "yearly",   PageChangeFrequencyType::yearly   },
    { "never",    PageChangeFrequencyType::never    },
};

PageChangeFrequencyType get_frequency_type(const std::string &freq_text) {
    return (pages_frequency_map.find(freq_text) != pages_frequency_map.end())
        ? pages_frequency_map.at(freq_text)
        : PageChangeFrequencyType::daily;
}

using boost::property_tree::ptree;

template<typename Target, typename Field>
using return_converter_t = std::remove_reference_t<std::invoke_result_t<Field, Target>>;

template<typename Target, typename Field>
void set(
    Target &target, const Field &field, const std::string &key, 
    const std::pair<const std::string, ptree> &node, 
    const std::function<return_converter_t<Target, Field>(const std::string &)> &converter = [](const std::string &s) { return s; }
) {
    if (node.first == key)
        std::invoke(field, target) = converter(node.second.data());
}

SitemapAny parse_sitemapindex(ptree &obj) {
    SitemapIndex res;
    for(const auto &child : obj.get_child("sitemapindex")) {
        if (child.first != "sitemap")
            continue;
        Sitemap cur;
        for(auto &field : child.second) {
            set(cur, &Sitemap::loc,          "loc",     field);
            set(cur, &Sitemap::lastModified, "lastmod", field);
        }
        res.push_back(std::move(cur));
    }
    return res;
}

SitemapAny parse_urlset(ptree &obj) {
    SitemapUrlset res;
    for(auto &child : obj.get_child("urlset")) {
        if (child.first != "url")
            continue;
        SitemapUrl cur;
        for(const auto &field : child.second) {
            set(cur, &SitemapUrl::loc,          "loc",          field);
            set(cur, &SitemapUrl::lastModified, "lastmod",      field);
            set(cur, &SitemapUrl::changeFreq,   "changefreq",   field, [](const std::string &s){ 
                return get_frequency_type(s); 
            });
            set(cur, &SitemapUrl::priority,     "priority",     field, [](const std::string &s) {
                return std::stod(s);
            });
        }
        res.push_back(std::move(cur));
    }
    return res;
}

inline const std::unordered_map<std::string, std::function<SitemapAny(ptree&)>> sitemap_parsers = {
    { "sitemapindex",   parse_sitemapindex  },
    { "urlset",         parse_urlset        },
};

} // namespace

[[nodiscard]] SitemapAny parse(const std::string &content) {
    std::istringstream istr{content};
    ptree obj;
    boost::property_tree::read_xml(istr, obj); 
    if (obj.empty())
        return SitemapAny{};
    auto id = obj.front().first;
    return sitemap_parsers.contains(id) 
        ? sitemap_parsers.at(id)(obj)
        : SitemapAny{};
}

} // namespace sitemap

} // namespace crawler