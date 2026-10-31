#pragma once

#include "content/rules/RuleContent.hpp"
#include "gameplay/common/Diagnostic.hpp"

namespace soma::content
{
    struct RuleContentLoadResult
    {
        RuleContent content;
        std::vector<gameplay::Diagnostic> diagnostics;
        [[nodiscard]] bool valid() const noexcept;
    };

    /// Loads rule fragments, their compatibility metadata, acquisition rules, and panel layout.
    class RuleContentLoader
    {
      public:
        [[nodiscard]] RuleContentLoadResult load(const std::filesystem::path& contentRoot) const;
    };
} // namespace soma::content
