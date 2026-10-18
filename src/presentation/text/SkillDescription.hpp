#pragma once

#include <string>
#include <string_view>

namespace soma::presentation
{
    [[nodiscard]] std::string skillDescription(std::string_view title, std::string_view description);
}
