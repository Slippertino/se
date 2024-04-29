#pragma once

#include <optional>
#include <crawler/parsers/robots/robots.hpp>
#include <crawler/utils/static_shared_resource.hpp>

namespace crawler {

enum class RobotStateType {
    None,
    Loading,
    Loaded
};

struct RobotState {
    static constexpr size_t max_load_attempts_count = 5;

    std::optional<parsers::Robots> robots;
    RobotStateType type;
    size_t load_attempts_last;

    RobotState() :
        robots{ std::nullopt },
        type{ RobotStateType::None },
        load_attempts_last{ max_load_attempts_count }
    { }
};

class RobotsCache 
    : public utils::StaticSharedResource<RobotsCache, RobotState> { };

} // namespace crawler