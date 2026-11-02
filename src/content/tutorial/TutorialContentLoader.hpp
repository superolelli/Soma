#pragma once

#include "content/tutorial/TutorialContent.hpp"
#include "gameplay/common/Diagnostic.hpp"

#include <filesystem>
#include <vector>

namespace soma::content
{
    struct TutorialContentLoadResult
    {
        TutorialContent content;
        std::vector<gameplay::Diagnostic> diagnostics;

        [[nodiscard]] bool valid() const;
    };

    /// Loads the editable tutorial sequence, modal messages, highlights, and on-demand trigger mapping.
    class TutorialContentLoader
    {
      public:
        [[nodiscard]] TutorialContentLoadResult load(const std::filesystem::path& contentRoot) const;
    };
} // namespace soma::content
