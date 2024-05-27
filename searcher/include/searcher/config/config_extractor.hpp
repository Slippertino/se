#pragma once

#include <string>
#include <userver/components/component_config.hpp>
#include <userver/formats/yaml/value.hpp>
#include <seutils/private_robber.hpp>
#include "config.hpp"

namespace se {

namespace utils {

struct TypeHolder {
    typedef const YAML::Node&(userver::formats::yaml::Value::*type)() const; 
};

template class rob<TypeHolder, &userver::formats::yaml::Value::GetNative>;

} // namespace utils

} // namespace se

namespace se {

namespace searcher {

inline Config extract_from_userver_config(const userver::components::ComponentConfig& config) {
    return Config((config.Yaml().*se::utils::result<se::utils::TypeHolder>::ptr)());
}

} // namespace searcher

} // namespace se