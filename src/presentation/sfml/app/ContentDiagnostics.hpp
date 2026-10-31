#pragma once

#include "gameplay/common/Diagnostic.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace soma::presentation
{
    inline std::string formatContentDiagnostics(std::string_view heading, const std::vector<gameplay::Diagnostic>& diagnostics)
    {
        std::string result{heading};
        for (const auto& diagnostic : diagnostics)
        {
            result += "\n" + diagnostic.file.string() + ": " + diagnostic.message;
        }
        return result;
    }
} // namespace soma::presentation
