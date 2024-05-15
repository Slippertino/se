#pragma once

namespace se {

namespace crawler {

enum class HandlingStatus {
    handled_success,
    handled_failed,
    partly_handled
};

} // namespace crawler

} // namespace se